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
#include <core/gui/undo/UndoManager.h>
#include <core/gui/widgets/AutocompleteLineEdit.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <viz/util/muparser/muParser.h>
#include "CreateExpressionPropertyModifier.h"

#include <QtConcurrent>

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, CreateExpressionPropertyModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, CreateExpressionPropertyModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(CreateExpressionPropertyModifier, CreateExpressionPropertyModifierEditor)
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _expressions, "Expressions")
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _propertyType, "PropertyType")
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _propertyName, "PropertyName")
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _propertyDataType, "PropertyDataType")
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _onlySelectedParticles, "OnlySelectedParticles")
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _expressions, "Expressions")
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _propertyType, "Property type")
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _propertyName, "Property name")
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _propertyDataType, "Data type")
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _onlySelectedParticles, "Compute only for selected particles")

/******************************************************************************
* Sets the type of the property being created by this modifier.
******************************************************************************/
void CreateExpressionPropertyModifier::setPropertyType(ParticleProperty::Type newType)
{
	if(newType == this->propertyType()) return;
	this->_propertyType = newType;

	if(newType != ParticleProperty::UserProperty) {
		setPropertyName(ParticleProperty::standardPropertyName(newType));
		setPropertyDataType(ParticleProperty::standardPropertyDataType(newType));
		setPropertyComponentCount(ParticleProperty::standardPropertyComponentCount(newType));
	}
}

/******************************************************************************
* Sets the number of vector components of the property to create.
******************************************************************************/
void CreateExpressionPropertyModifier::setPropertyComponentCount(int newComponentCount)
{
	if(newComponentCount == this->propertyComponentCount()) return;

	if(newComponentCount < propertyComponentCount()) {
		setExpressions(expressions().mid(0, newComponentCount));
	}
	else {
		QStringList newList = expressions();
		while(newList.size() < newComponentCount)
			newList.append("0");
		setExpressions(newList);
	}
}

/******************************************************************************
* Determines the available variable names.
******************************************************************************/
QStringList CreateExpressionPropertyModifier::getVariableNames(const PipelineFlowState& inputState)
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
class CreateExpressionEvaluationKernel
{
private:

	struct ExpressionVariable {
		double value;
		const char* dataPointer;
		size_t stride;
		bool isFloat;
	};

public:

	/// Initializes the expressions parsers.
	bool initialize(const QStringList& expressions, const QStringList& variableNames, const PipelineFlowState& input, int timestep, int inputParticleCount) {
		parsers.resize(expressions.size());
		variables.resize(variableNames.size());
		bool usesTimeInExpression = false;

		// Compile the expression strings.
		for(int i = 0; i < expressions.size(); i++) {

			QString expr = expressions[i];
			if(expr.isEmpty())
				throw Exception(CreateExpressionPropertyModifier::tr("The expression for component %1 is empty.").arg(i+1));

			try {

				// Configure parser to accept '.' in variable names.
				parsers[i].DefineNameChars("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.");

				// Let the muParser process the math expression.
				parsers[i].SetExpr(expr.toStdString());

				// Register variables
				for(int v = 0; v < variableNames.size(); v++)
					parsers[i].DefineVar(variableNames[v].toStdString(), &variables[v].value);

				// If the current animation time is used in the math expression then we have to
				// reduce the validity interval to the current time only.
				mu::varmap_type usedVariables = parsers[i].GetUsedVar();
				if(usedVariables.find("t") != usedVariables.end())
					usesTimeInExpression = true;

				// Add constants.
				parsers[i].DefineConst("pi", 3.1415926535897932);
				parsers[i].DefineConst("N", inputParticleCount);
				parsers[i].DefineConst("t", timestep);
			}
			catch(mu::Parser::exception_type& ex) {
				throw Exception(QString::fromStdString(ex.GetMsg()));
			}
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

	void run(size_t startIndex, size_t endIndex, ParticlePropertyObject* outputProperty, const int* selectionValues) {
		try {

			// Position pointers.
			if(selectionValues) selectionValues += startIndex;
			for(auto& v : variables)
				v.dataPointer += v.stride * startIndex;

			int integerDataType = qMetaTypeId<int>();
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

				// Skip unselected atoms if restricted to selected atoms.
				if(selectionValues) {
					if(!(*selectionValues++))
						continue;
				}

				for(int j = 0; j < parsers.size(); j++) {
					// Evaluate expression for the current atom.
					double value = parsers[j].Eval();

					// Store computed value in output channel.
					if(outputProperty->dataType() == integerDataType)
						outputProperty->setIntComponent(i, j, (int)value);
					else
						outputProperty->setFloatComponent(i, j, (FloatType)value);
				}
			}
		}
		catch(const mu::Parser::exception_type& ex) {
			errorMsg = QString::fromStdString(ex.GetMsg());
		}
	}

	QString errorMsg;

private:

	QVector<mu::Parser> parsers;
	std::vector<ExpressionVariable> variables;
};

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus CreateExpressionPropertyModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get list of available input variables.
	_variableNames = getVariableNames(input());

	// The current animation frame number.
	int currentFrame = AnimManager::instance().timeToFrame(time);

	// Create and initialize the worker threads.
	int nthreads = std::max(QThread::idealThreadCount(), 1);
	if((size_t)nthreads > inputParticleCount())
		nthreads = (int)inputParticleCount();

	QVector<CreateExpressionEvaluationKernel> workers(nthreads);
	for(QVector<CreateExpressionEvaluationKernel>::iterator worker = workers.begin(); worker != workers.end(); ++worker) {
		if(worker->initialize(expressions(), _variableNames, input(), currentFrame, (int)inputParticleCount()))
			validityInterval.intersect(TimeInterval(time));
	}

	// Prepare the deep copy of the output property.
	ParticlePropertyObject* outputProperty;
	if(propertyType() != ParticleProperty::UserProperty)
		outputProperty = outputStandardProperty(propertyType());
	else {
		size_t dataTypeSize;
		if(propertyDataType() == qMetaTypeId<int>())
			dataTypeSize = sizeof(int);
		else if(propertyDataType() == qMetaTypeId<FloatType>())
			dataTypeSize = sizeof(FloatType);
		else
			throw Exception(tr("New property has an invalid data type."));
		outputProperty = outputCustomProperty(propertyName(), propertyDataType(), dataTypeSize, propertyComponentCount());
	}
	OVITO_CHECK_OBJECT_POINTER(outputProperty);

	// Get the selection property if the application of the modifier is restricted to selected particles.
	const int* selectionValues = nullptr;
	if(onlySelectedParticles()) {
		ParticlePropertyObject* selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);
		if(!selProperty)
			throw Exception(tr("Evaluation has been restricted to selected particles but input does not contain a selection set."));
		OVITO_ASSERT(selProperty->size() == inputParticleCount());
		selectionValues = selProperty->constDataInt();
	}

	if(inputParticleCount() != 0) {

		// Shared memory management is not thread-safe. Make sure the deep copy of the data has been
		// made before the worker threads are started.
		outputProperty->data();

		// Spawn worker threads.
		QFutureSynchronizer<void> synchronizer;
		size_t chunkSize = std::max(inputParticleCount() / workers.size(), (size_t)1);
		for(int i = 0; i < workers.size(); i++) {
			// Setup data range.
			size_t startIndex = chunkSize * (size_t)i;
			size_t endIndex = std::min(startIndex + chunkSize, inputParticleCount());
			if(i == workers.size() - 1) endIndex = inputParticleCount();
			if(endIndex <= startIndex) continue;

			synchronizer.addFuture(QtConcurrent::run(&workers[i], &CreateExpressionEvaluationKernel::run, startIndex, endIndex, outputProperty, selectionValues));
		}
		synchronizer.waitForFinished();

		// Check for errors.
		for(auto& worker : workers) {
			if(worker.errorMsg.isEmpty() == false)
				throw Exception(worker.errorMsg);
		}

		outputProperty->changed();
	}

	return ObjectStatus::Success;
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void CreateExpressionPropertyModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Build list of available input variables.
	PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
	_variableNames = getVariableNames(input);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CreateExpressionPropertyModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	rollout = createRollout(tr("Create expression property"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* mainLayout = new QVBoxLayout(rollout);
	mainLayout->setContentsMargins(4,4,4,4);

	QGroupBox* propertiesGroupBox = new QGroupBox(tr("Property"));
	mainLayout->addWidget(propertiesGroupBox);
	QGridLayout* propertiesLayout = new QGridLayout(propertiesGroupBox);
	propertiesLayout->setContentsMargins(4,4,4,4);
	propertiesLayout->setColumnStretch(1, 1);
#ifndef Q_OS_MACX
	propertiesLayout->setSpacing(2);
#endif

	// Create the combo box with the standard property types.
	VariantComboBoxParameterUI* propertyTypeUI = new VariantComboBoxParameterUI(this, "propertyType");
	propertiesLayout->addWidget(new QLabel(tr("Output property:")), 0, 0);
	propertiesLayout->addWidget(propertyTypeUI->comboBox(), 0, 1, 1, 2);
	propertyTypeUI->comboBox()->addItem(tr("Custom property"), qVariantFromValue(ParticleProperty::UserProperty));
	QMap<QString, ParticleProperty::Type> standardProperties = ParticleProperty::standardPropertyList();
	for(auto p = standardProperties.begin(); p != standardProperties.end(); ++p) {
		if(ParticleProperty::standardPropertyComponentCount(p.value()) > 0) {
			propertyTypeUI->comboBox()->addItem(p.key(), qVariantFromValue(p.value()));
		}
	}

	// Create the field with the property name.
	propertyNameUI = new StringParameterUI(this, "propertyName");
	propertiesLayout->addWidget(new QLabel(tr("Name:")), 1, 0);
	propertiesLayout->addWidget(propertyNameUI->textBox(), 1, 1);

	// Create the combo box with the property types.
	propertyDataTypeUI = new VariantComboBoxParameterUI(this, "propertyDataType");
	propertiesLayout->addWidget(new QLabel(tr("Data type:")), 2, 0);
	propertiesLayout->addWidget(propertyDataTypeUI->comboBox(), 2, 1);
	propertyDataTypeUI->comboBox()->addItem(tr("Floating-point"), qMetaTypeId<FloatType>());
	propertyDataTypeUI->comboBox()->addItem(tr("Integer"), qMetaTypeId<int>());

	// Create the spinner for the number of components.
	numComponentsUI = new IntegerParameterUI(this, "propertyComponentCount");
	numComponentsUI->setMinValue(1);
	numComponentsUI->setMaxValue(16);
	propertiesLayout->addWidget(new QLabel(tr("Number of components:")), 3, 0);
	propertiesLayout->addLayout(numComponentsUI->createFieldLayout(), 3, 1);

	// Create the check box for the selection flag.
	BooleanParameterUI* selectionFlagUI = new BooleanParameterUI(this, PROPERTY_FIELD(CreateExpressionPropertyModifier::_onlySelectedParticles));
	propertiesLayout->addWidget(selectionFlagUI->checkBox(), 5, 0, 1, 2);

	expressionsGroupBox = new QGroupBox(tr("Expressions"));
	mainLayout->addWidget(expressionsGroupBox);
	expressionsLayout = new QVBoxLayout(expressionsGroupBox);
	expressionsLayout->setContentsMargins(4,4,4,4);
#ifndef Q_OS_MACX
	expressionsLayout->setSpacing(0);
#endif

	// Status label.
	mainLayout->addWidget(statusLabel());

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
bool CreateExpressionPropertyModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		updateEditorFields();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Updates the enabled/disabled status of the editor's controls.
******************************************************************************/
void CreateExpressionPropertyModifierEditor::updateEditorFields()
{
	CreateExpressionPropertyModifier* mod = static_object_cast<CreateExpressionPropertyModifier>(editObject());
	propertyNameUI->setEnabled(mod && mod->propertyType() == ParticleProperty::UserProperty);
	propertyDataTypeUI->setEnabled(mod && mod->propertyType() == ParticleProperty::UserProperty);
	numComponentsUI->setEnabled(mod && mod->propertyType() == ParticleProperty::UserProperty);
	if(!mod) return;

	const QStringList& expr = mod->expressions();
	while(expr.size() > expressionBoxes.size()) {
		QLabel* label = new QLabel();
		AutocompleteLineEdit* edit = new AutocompleteLineEdit();
		edit->setWordList(mod->lastVariableNames());
		expressionsLayout->insertWidget(expressionBoxes.size()*2, label);
		expressionsLayout->insertWidget(expressionBoxes.size()*2 + 1, edit);
		expressionBoxes.push_back(edit);
		expressionBoxLabels.push_back(label);
		connect(edit, SIGNAL(editingFinished()), this, SLOT(onExpressionEditingFinished()));
	}
	while(expr.size() < expressionBoxes.size()) {
		delete expressionBoxes.takeLast();
		delete expressionBoxLabels.takeLast();
	}
	OVITO_ASSERT(expressionBoxes.size() == expr.size());
	OVITO_ASSERT(expressionBoxLabels.size() == expr.size());

	QStringList standardPropertyComponentNames;
	if(mod->propertyType() != ParticleProperty::UserProperty) {
		standardPropertyComponentNames = ParticleProperty::standardPropertyComponentNames(mod->propertyType());
		if(standardPropertyComponentNames.empty())
			standardPropertyComponentNames.push_back(ParticleProperty::standardPropertyName(mod->propertyType()));
	}
	for(int i = 0; i < expr.size(); i++) {
		expressionBoxes[i]->setText(expr[i]);
		if(i < standardPropertyComponentNames.size())
			expressionBoxLabels[i]->setText(tr("%1:").arg(standardPropertyComponentNames[i]));
		else
			expressionBoxLabels[i]->setText(tr("Component %1:").arg(i+1));
	}

	QString labelText(tr("The following variables can be used in the expression:<ul>"));
	Q_FOREACH(QString s, mod->lastVariableNames()) {
		labelText.append(QString("<li>%1</li>").arg(s));
	}
	labelText.append(QString("<li>N (number of particles)</li>"));
	labelText.append(QString("<li>t (current animation frame)</li>"));
	labelText.append("</ul><p></p>");
	variableNamesList->setText(labelText);

	container()->updateRolloutsLater();
}

/******************************************************************************
* Is called when the user has typed in an expression.
******************************************************************************/
void CreateExpressionPropertyModifierEditor::onExpressionEditingFinished()
{
	QLineEdit* edit = (QLineEdit*)sender();
	int index = expressionBoxes.indexOf(edit);
	OVITO_ASSERT(index >= 0);

	CreateExpressionPropertyModifier* mod = static_object_cast<CreateExpressionPropertyModifier>(editObject());

	UndoableTransaction::handleExceptions(tr("Change expression"), [mod, index]() {
		QStringList expr = mod->expressions();
		expr[index] = edit->text();
		mod->setExpressions(expr);
	});
}

};	// End of namespace
