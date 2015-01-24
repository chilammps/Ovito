///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <plugins/particles/Particles.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include "ParticleExpressionEvaluator.h"

#include <QtConcurrent>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/// List of characters allowed in variable names.
QByteArray ParticleExpressionEvaluator::_validVariableNameChars("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.");

/******************************************************************************
* Initializes the list of input variables from the given input state.
******************************************************************************/
void ParticleExpressionEvaluator::createInputVariables(const PipelineFlowState& inputState, int animationFrame)
{
	_inputVariables.clear();

	int propertyIndex = 1;
	size_t particleCount = 0;
	for(DataObject* o : inputState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
		if(!property) continue;

		ExpressionVariable v;

		// Properties with custom data type are not supported by this modifier.
		if(property->dataType() == qMetaTypeId<int>())
			v.type = PARTICLE_INT_PROPERTY;
		else if(property->dataType() == qMetaTypeId<FloatType>())
			v.type = PARTICLE_FLOAT_PROPERTY;
		else
			continue;
		particleCount = property->size();

		// Derive a valid variable name from the property name by removing all invalid characters.
		QString propertyName = property->name();
		// If the name is empty, generate one.
		if(propertyName.isEmpty())
			propertyName = QString("Property%1").arg(propertyIndex);
		// If the name starts with a number, prepend an underscore.
		else if(propertyName[0].isDigit())
			propertyName.prepend(QChar('_'));

		for(size_t k = 0; k < property->componentCount(); k++) {

			QString fullPropertyName = propertyName;
			if(property->componentNames().size() == property->componentCount())
				fullPropertyName += "." + property->componentNames()[k];

			// Filter out invalid characters.
			v.name.clear();
			for(QChar c : fullPropertyName) {
				char cc = c.toLatin1();
				if(_validVariableNameChars.contains(cc))
					v.name.push_back(cc);
			}
			if(v.name.empty()) continue;

			// Initialize data pointer into particle property storage.
			if(property->dataType() == qMetaTypeId<int>())
				v.dataPointer = reinterpret_cast<const char*>(property->constDataInt() + k);
			else
				v.dataPointer = reinterpret_cast<const char*>(property->constDataFloat() + k);
			v.stride = property->stride();

			addVariable(v);
		}

		propertyIndex++;
	}

	SimulationCellObject* simCell = inputState.findObject<SimulationCellObject>();

	// Create variable for reduced particle coordinates.
	ParticlePropertyObject* posProperty = ParticlePropertyObject::findInState(inputState, ParticleProperty::PositionProperty);
	if(posProperty && simCell) {
		SimulationCell cellData = simCell->data();
		ExpressionVariable v;
		v.type = DERIVED_PARTICLE_PROPERTY;
		v.name = "ReducedPosition.X";
		v.function = [posProperty,cellData](size_t particleIndex) -> double {
			return cellData.inverseMatrix().prodrow(posProperty->getPoint3(particleIndex), 0);
		};
		addVariable(v);
		v.name = "ReducedPosition.Y";
		v.function = [posProperty,cellData](size_t particleIndex) -> double {
			return cellData.inverseMatrix().prodrow(posProperty->getPoint3(particleIndex), 1);
		};
		addVariable(v);
		v.name = "ReducedPosition.Z";
		v.function = [posProperty,cellData](size_t particleIndex) -> double {
			return cellData.inverseMatrix().prodrow(posProperty->getPoint3(particleIndex), 2);
		};
		addVariable(v);
	}

	// Create particle index variable.
	ExpressionVariable pindexVar;
	pindexVar.name = "ParticleIndex";
	pindexVar.type = PARTICLE_INDEX;
	addVariable(pindexVar);

	// Create constant variables.
	ExpressionVariable constVar;

	// Number of particles
	constVar.name = "N";
	constVar.type = GLOBAL_PARAMETER;
	constVar.value = particleCount;
	constVar.description = tr("number of particles");
	addVariable(constVar);

	// Animation frame
	constVar.name = "Frame";
	constVar.type = GLOBAL_PARAMETER;
	constVar.value = animationFrame;
	constVar.description = tr("animation frame number");
	addVariable(constVar);

	// Timestep.
	if(inputState.attributes().contains(QStringLiteral("Timestep"))) {
		constVar.name = "Timestep";
		constVar.type = GLOBAL_PARAMETER;
		constVar.value = inputState.attributes().value(QStringLiteral("Timestep")).toDouble();
		constVar.description = tr("simulation timestep");
		addVariable(constVar);
	}

	if(simCell) {
		// Cell volume
		constVar.name = "CellVolume";
		constVar.type = GLOBAL_PARAMETER;
		constVar.value = simCell->volume();
		constVar.description = tr("simulation cell volume");
		addVariable(constVar);

		// Cell size
		constVar.type = GLOBAL_PARAMETER;
		constVar.value = std::abs(simCell->edgeVector1().x());
		constVar.name = "CellSize.X";
		constVar.description = tr("size along X");
		addVariable(constVar);
		constVar.value = std::abs(simCell->edgeVector2().y());
		constVar.name = "CellSize.Y";
		constVar.description = tr("size along Y");
		addVariable(constVar);
		constVar.value = std::abs(simCell->edgeVector3().z());
		constVar.name = "CellSize.Z";
		constVar.description = tr("size along Z");
		addVariable(constVar);
	}

	// Pi
	constVar.name = "pi";
	constVar.type = CONSTANT;
	constVar.value = M_PI;
	constVar.description = QStringLiteral("%1...").arg(M_PI);
	addVariable(constVar);
}

/******************************************************************************
* Registers an input variable if the name does not exist yet.
******************************************************************************/
void ParticleExpressionEvaluator::addVariable(const ExpressionVariable& v)
{
	// Check if name is unique.
	if(std::none_of(_inputVariables.begin(), _inputVariables.end(), [&v](const ExpressionVariable& v2) -> bool { return v2.name == v.name; }))
		_inputVariables.push_back(v);
}

/******************************************************************************
* Returns the list of available input variables.
******************************************************************************/
QStringList ParticleExpressionEvaluator::inputVariableNames() const
{
	QStringList vlist;
	for(const ExpressionVariable& v : _inputVariables)
		vlist << QString::fromLatin1(v.name.c_str());
	return vlist;
}

/******************************************************************************
* Specifies the expressions to be evaluated for each particle and create the
* list of input variables.
******************************************************************************/
void ParticleExpressionEvaluator::initialize(const QStringList& expressions, const PipelineFlowState& inputState, int animationFrame)
{
	// Create list of input variables.
	createInputVariables(inputState, animationFrame);

	// Copy expression strings into internal array.
	_expressions.resize(expressions.size());
	std::transform(expressions.begin(), expressions.end(), _expressions.begin(), [](const QString& s) -> std::string { return s.toStdString(); });

	// Determine number of input particles.
	_particleCount = 0;
	if(ParticlePropertyObject* posProperty = ParticlePropertyObject::findInState(inputState, ParticleProperty::PositionProperty))
		_particleCount = posProperty->size();
}

/******************************************************************************
* Initializes the parser object and evaluates the expressions for every particle
******************************************************************************/
void ParticleExpressionEvaluator::evaluate(const std::function<void(size_t,size_t,double)>& callback, const std::function<bool(size_t)>& filter)
{
	// Make sure initialize() has been called.
	OVITO_ASSERT(!_inputVariables.empty());
	_usedVars.clear();

	// Determine the number of parallel threads to use.
	size_t nthreads = std::max(QThread::idealThreadCount(), 1);
	if(_particleCount == 0)
		return;
	else if(_particleCount < 100)
		nthreads = 1;
	else if(nthreads > _particleCount)
		nthreads = _particleCount;

	if(nthreads == 1) {
		WorkerThread worker;
		worker.initialize(_expressions, _inputVariables, _usedVars);
		worker.run(0, _particleCount, callback, filter);
		if(worker._errorMsg.isEmpty() == false)
			throw Exception(worker._errorMsg);
	}
	else if(nthreads > 1) {
		std::vector<WorkerThread> workers(nthreads);
		for(auto& worker : workers)
			worker.initialize(_expressions, _inputVariables, _usedVars);

		// Spawn worker threads.
		QFutureSynchronizer<void> synchronizer;
		size_t chunkSize = _particleCount / nthreads;
		OVITO_ASSERT(chunkSize > 0);
		for(size_t i = 0; i < workers.size(); i++) {
			// Setup data range.
			size_t startIndex = chunkSize * i;
			size_t endIndex = startIndex + chunkSize;
			if(i == workers.size() - 1) endIndex = _particleCount;
			OVITO_ASSERT(endIndex > startIndex);
			OVITO_ASSERT(endIndex <= _particleCount);
			synchronizer.addFuture(QtConcurrent::run(&workers[i], &WorkerThread::run, startIndex, endIndex, callback, filter));
		}
		synchronizer.waitForFinished();

		// Check for errors.
		for(auto& worker : workers) {
			if(worker._errorMsg.isEmpty() == false)
				throw Exception(worker._errorMsg);
		}
	}
}

/******************************************************************************
* Initializes the parser objects of this thread.
******************************************************************************/
void ParticleExpressionEvaluator::WorkerThread::initialize(const std::vector<std::string>& expressions, const QVector<ExpressionVariable>& inputVariables, std::set<std::string>& usedVars)
{
	_parsers.resize(expressions.size());
	_inputVariables = inputVariables;

	auto parser = _parsers.begin();
	auto expr = expressions.cbegin();
	for(size_t i = 0; i < expressions.size(); i++, ++parser, ++expr) {

		if(expr->empty()) {
			if(expressions.size() > 1)
				throw Exception(tr("Expression %1 is empty.").arg(i+1));
			else
				throw Exception(tr("Expression is empty."));
		}

		try {
			// Configure parser to accept alpha-numeric characters and '.' in variable names.
			parser->DefineNameChars(_validVariableNameChars.constData());

			// Let the muParser process the math expression.
			parser->SetExpr(*expr);

			// Register input variables.
			for(auto& v : _inputVariables)
				parser->DefineVar(v.name, &v.value);

			// If the current animation time is used in the math expression then we have to
			// reduce the validity interval to the current time only.
			for(const auto& vname : parser->GetUsedVar())
				usedVars.insert(vname.first);
		}
		catch(mu::Parser::exception_type& ex) {
			throw Exception(QString::fromStdString(ex.GetMsg()));
		}
	}
}

/******************************************************************************
* The worker routine.
******************************************************************************/
void ParticleExpressionEvaluator::WorkerThread::run(size_t startIndex, size_t endIndex, std::function<void(size_t,size_t,double)> callback, std::function<bool(size_t)> filter)
{
	try {
		// Position variable pointers to first input particle.
		for(auto& v : _inputVariables)
			v.dataPointer += v.stride * startIndex;

		int integerDataType = qMetaTypeId<int>();
		for(size_t i = startIndex; i < endIndex; i++) {

			// Update variable values for the current particle.
			for(auto& v : _inputVariables) {
				if(v.type == PARTICLE_FLOAT_PROPERTY) {
					v.value = *reinterpret_cast<const FloatType*>(v.dataPointer);
					v.dataPointer += v.stride;
				}
				if(v.type == PARTICLE_INT_PROPERTY) {
					v.value = *reinterpret_cast<const int*>(v.dataPointer);
					v.dataPointer += v.stride;
				}
				else if(v.type == PARTICLE_INDEX) {
					v.value = i;
				}
				else if(v.type == DERIVED_PARTICLE_PROPERTY) {
					v.value = v.function(i);
				}
			}

			if(filter && !filter(i))
				continue;

			for(size_t j = 0; j < _parsers.size(); j++) {
				// Evaluate expression for the current particle.
				callback(i, j, _parsers[j].Eval());
			}
		}
	}
	catch(const mu::Parser::exception_type& ex) {
		_errorMsg = QString::fromStdString(ex.GetMsg());
	}
}

/******************************************************************************
* Returns a human-readable text listing the input variables.
******************************************************************************/
QString ParticleExpressionEvaluator::inputVariableTable() const
{
	QString str(tr("<p>The following inputs can be referenced in the expression:</p><p><b>Particle properties:</b><ul>"));
	for(const ExpressionVariable& v : _inputVariables) {
		if(v.type == PARTICLE_FLOAT_PROPERTY || v.type == PARTICLE_INT_PROPERTY || v.type == PARTICLE_INDEX || v.type == DERIVED_PARTICLE_PROPERTY) {
			if(v.description.isEmpty())
				str.append(QStringLiteral("<li>%1</li>").arg(QString::fromStdString(v.name)));
			else
				str.append(QStringLiteral("<li>%1 (<i>%2</i>)</li>").arg(QString::fromStdString(v.name)).arg(v.description));
		}
	}
	str.append(QStringLiteral("</ul></p><p><b>Global parameters:</b><ul>"));
	for(const ExpressionVariable& v : _inputVariables) {
		if(v.type == GLOBAL_PARAMETER) {
			if(v.description.isEmpty())
				str.append(QStringLiteral("<li>%1</li>").arg(QString::fromStdString(v.name)));
			else
				str.append(QStringLiteral("<li>%1 (<i>%2</i>)</li>").arg(QString::fromStdString(v.name)).arg(v.description));
		}
	}
	str.append(QStringLiteral("</ul></p><p><b>Constants:</b><ul>"));
	for(const ExpressionVariable& v : _inputVariables) {
		if(v.type == CONSTANT) {
			if(v.description.isEmpty())
				str.append(QStringLiteral("<li>%1</li>").arg(QString::fromStdString(v.name)));
			else
				str.append(QStringLiteral("<li>%1 (<i>%2</i>)</li>").arg(QString::fromStdString(v.name)).arg(v.description));
		}
	}
	str.append(QStringLiteral("</ul></p><p></p>"));
	return str;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
