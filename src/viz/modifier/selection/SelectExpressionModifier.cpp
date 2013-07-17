///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/gui/properties/StringParameterUI.h>
#include <viz/util/muparser/muParser.h>
#include "SelectExpressionModifier.h"

#include <QtConcurrent>

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, SelectExpressionModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, SelectExpressionModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(SelectExpressionModifier, SelectExpressionModifierEditor)
DEFINE_PROPERTY_FIELD(SelectExpressionModifier, _expression, "Expression")
SET_PROPERTY_FIELD_LABEL(SelectExpressionModifier, _expression, "Boolean expression")

/******************************************************************************
* Determines the available variable names.
******************************************************************************/
QStringList SelectExpressionModifier::getVariableNames(const PipelineFlowState& inputState)
{
	// Regular expression used to filter out invalid characters in a expression variable name.
	QRegExp regExp("[^A-Za-z\\d_]");

	QStringList variableNames;
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(!property) continue;

		// Properties with custom data type are not supported by this modifier.
		if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;

		// Alter the property name to make it a valid variable name for the parser.
		QString variableName = property->name();
		variableName.remove(regExp);
		if(property->componentNames().empty()) {
			OVITO_ASSERT(property->componentCount() == 1);
			variableNames << variableName;
		}
		else {
			Q_FOREACH(QString componentName, property->componentNames()) {
				componentName.remove(regExp);
				variableNames << (variableName + "." + componentName);
			}
		}
	}

	// The particle index is always available in the expression as an input variable.
	variableNames << "ParticleIndex";

	return variableNames;
}

/**
 * This helper class is needed to enable multi-threaded evaluation of math expressions
 * for all particles. Each instance of this class is assigned a chunk of particles that it processes.
 */
class SelExpressionEvaluationKernel
{
private:

	/// An input variable.
	struct ExpressionVariable {
		double value;
		const char* dataPointer;
		size_t stride;
		bool isFloat;
	};

public:

	/// Initializes the expressions parser.
	bool initialize(const QString& expression, const QStringList& variableNames, const PipelineFlowState& input, int timestep, int inputParticleCount) {
		variables.resize(variableNames.size());
		bool usesTimeInExpression = false;
		nSelected = 0;

		// Compile the expression string.
		try {
			// Configure parser to accept '.' in variable names.
			parser.DefineNameChars("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.");

			// Let the muParser process the math expression.
			parser.SetExpr(expression.toStdString());

			// Register variables
			for(int v = 0; v < variableNames.size(); v++)
				parser.DefineVar(variableNames[v].toStdString(), &variables[v].value);

			// If the current animation time is used in the math expression then we have to
			// reduce the validity interval to the current time only.
			mu::varmap_type usedVariables = parser.GetUsedVar();
			if(usedVariables.find("t") != usedVariables.end())
				usesTimeInExpression = true;

			// Add constants.
			parser.DefineConst("pi", 3.1415926535897932);
			parser.DefineConst("N", inputParticleCount);
			parser.DefineConst("t", timestep);
		}
		catch(mu::Parser::exception_type& ex) {
			throw Exception(QString::fromStdString(ex.GetMsg()));
		}

		// Setup input data pointers.
		size_t vindex = 0;
		for(const auto& o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
			if(!property) continue;
			if(property->dataType() == qMetaTypeId<FloatType>()) {
				for(size_t k = 0; k < property->componentCount(); k++) {
					OVITO_ASSERT((int)vindex < variableNames.size());
					variables[vindex].dataPointer = reinterpret_cast<const char*>(property->constDataFloat() + k);
					variables[vindex].stride = property->perParticleSize();
					variables[vindex].isFloat = true;
					vindex++;
				}
			}
			else if(property->dataType() == qMetaTypeId<int>()) {
				for(size_t k = 0; k < property->componentCount(); k++) {
					OVITO_ASSERT((int)vindex < variableNames.size());
					variables[vindex].dataPointer = reinterpret_cast<const char*>(property->constDataInt() + k);
					variables[vindex].stride = property->perParticleSize();
					variables[vindex].isFloat = false;
					vindex++;
				}
			}
			else OVITO_ASSERT(false);
		}

		// Add the special Index variable.
		variables[vindex].dataPointer = nullptr;
		variables[vindex].stride = 0;
		variables[vindex].isFloat = false;
		vindex++;

		OVITO_ASSERT(vindex == variableNames.size());

		return usesTimeInExpression;
	}

	void run(size_t startIndex, size_t endIndex, ParticlePropertyObject* outputProperty) {
		try {
			// Position pointers.
			for(auto& v : variables)
				v.dataPointer += v.stride * startIndex;

			nSelected = 0;
			for(size_t i = startIndex; i < endIndex; i++) {

				// Update variable values for the current particle.
				for(auto& v : variables) {
					if(v.isFloat)
						v.value = *reinterpret_cast<const FloatType*>(v.dataPointer);
					else if(v.dataPointer)
						v.value = *reinterpret_cast<const int*>(v.dataPointer);
					else
						v.value = i;
					v.dataPointer += v.stride;
				}

				// Evaluate expression for the current particle.
				double value = parser.Eval();

				// Store computed value in output property.
				if(value) {
					outputProperty->setInt(i, 1);
					nSelected++;
				}
				else outputProperty->setInt(i, 0);
			}
		}
		catch(const mu::Parser::exception_type& ex) {
			errorMsg = QString::fromStdString(ex.GetMsg());
		}
	}

	QString errorMsg;
	size_t nSelected;

private:

	mu::Parser parser;
	std::vector<ExpressionVariable> variables;
};


/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus SelectExpressionModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get list of available input variables.
	_variableNames = getVariableNames(input());

	// The current animation frame number.
	int currentFrame = AnimManager::instance().timeToFrame(time);

	// If the user has not yet entered an expression let him know which
	// data channels can be used in the expression.
	if(expression().isEmpty())
		return ObjectStatus(ObjectStatus::Warning, tr("Please enter a boolean expression."));

	// Check if expression contain an assignment ('=' operator).
	// This is dangerous because the user probably means the comparison operator '=='.
	if(expression().contains(QRegExp("[^=!><]=(?!=)")))
		throw Exception("The expression contains the assignment operator '='. Please use the comparison operator '==' instead.");

	// The number of selected particles.
	size_t nSelected = 0;

	// Create and initialize the worker threads.
	int nthreads = std::max(QThread::idealThreadCount(), 1);
	if((size_t)nthreads > inputParticleCount())
		nthreads = (int)inputParticleCount();

	QVector<SelExpressionEvaluationKernel> workers(nthreads);
	for(QVector<SelExpressionEvaluationKernel>::iterator worker = workers.begin(); worker != workers.end(); ++worker) {
		if(worker->initialize(expression(), _variableNames, input(), currentFrame, (int)inputParticleCount()))
			validityInterval.intersect(TimeInterval(time));
	}

	// Get the deep copy of the selection property.
	ParticlePropertyObject* selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);

	if(inputParticleCount() != 0) {

		// Shared memory management is not thread-safe. Make sure the deep copy of the data has been
		// made before the worker threads are started.
		selProperty->data();

		// Spawn worker threads.
		QFutureSynchronizer<void> synchronizer;
		size_t chunkSize = std::max(inputParticleCount() / workers.size(), (size_t)1);
		for(int i = 0; i < workers.size(); i++) {

			// Setup data range.
			size_t startIndex = chunkSize * (size_t)i;
			size_t endIndex = std::min(startIndex + chunkSize, inputParticleCount());
			if(i == workers.size() - 1) endIndex = inputParticleCount();
			if(endIndex <= startIndex) continue;

			synchronizer.addFuture(QtConcurrent::run(&workers[i], &SelExpressionEvaluationKernel::run, startIndex, endIndex, selProperty));
		}
		synchronizer.waitForFinished();

		// Check for errors.
		for(auto& worker : workers) {
			if(worker.errorMsg.isEmpty() == false)
				throw Exception(worker.errorMsg);

			nSelected += worker.nSelected;
		}

		selProperty->changed();
	}

	QString statusMessage = tr("%1 out of %2 particles selected (%3%)").arg(nSelected).arg(inputParticleCount()).arg(nSelected * 100 / std::max(inputParticleCount(), (size_t)1));
	return ObjectStatus(ObjectStatus::Success, QString(), statusMessage);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void SelectExpressionModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Build list of available input variables.
	PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
	_variableNames = getVariableNames(input);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SelectExpressionModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	QWidget* rollout = createRollout(tr("Expression select"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);

	layout->addWidget(new QLabel(tr("Boolean expression:")));
	StringParameterUI* expressionUI = new StringParameterUI(this, PROPERTY_FIELD(SelectExpressionModifier::_expression));
	expressionLineEdit = new AutocompleteLineEdit();
	expressionUI->setTextBox(expressionLineEdit);
	layout->addWidget(expressionUI->textBox());

	// Status label.
	layout->addSpacing(12);
	layout->addWidget(statusLabel());

	QWidget* variablesRollout = createRollout(tr("Variables"), rolloutParams.after(rollout));
    QVBoxLayout* variablesLayout = new QVBoxLayout(variablesRollout);
    variablesLayout->setContentsMargins(4,4,4,4);
	variableNamesList = new QLabel();
	variableNamesList->setWordWrap(true);
	variableNamesList->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	variablesLayout->addWidget(variableNamesList);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool SelectExpressionModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		updateEditorFields();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Updates the enabled/disabled status of the editor's controls.
******************************************************************************/
void SelectExpressionModifierEditor::updateEditorFields()
{
	SelectExpressionModifier* mod = static_object_cast<SelectExpressionModifier>(editObject());
	if(!mod) return;

	QString labelText(tr("The following variables can be used in the boolean expression:<ul>"));
	Q_FOREACH(QString s, mod->lastVariableNames()) {
		labelText.append(QString("<li>%1</li>").arg(s));
	}
	labelText.append(QString("<li>N (number of particles)</li>"));
	labelText.append(QString("<li>t (current animation frame)</li>"));
	labelText.append("</ul><p></p>");
	variableNamesList->setText(labelText);

	expressionLineEdit->setWordList(mod->lastVariableNames());
}


};	// End of namespace
