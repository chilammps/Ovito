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

#include <plugins/particles/Particles.h>
#include <plugins/particles/util/ParticleExpressionEvaluator.h>
#include <core/gui/widgets/general/AutocompleteLineEdit.h>
#include <core/animation/AnimationSettings.h>
#include <core/scene/pipeline/PipelineObject.h>
#include "CreateExpressionPropertyModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CreateExpressionPropertyModifier, ParticleModifier);
IMPLEMENT_OVITO_OBJECT(Particles, CreateExpressionPropertyModifierEditor, ParticleModifierEditor);
SET_OVITO_OBJECT_EDITOR(CreateExpressionPropertyModifier, CreateExpressionPropertyModifierEditor);
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _expressions, "Expressions");
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _propertyType, "PropertyType");
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _propertyName, "PropertyName");
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _propertyDataType, "PropertyDataType");
DEFINE_PROPERTY_FIELD(CreateExpressionPropertyModifier, _onlySelectedParticles, "OnlySelectedParticles");
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _expressions, "Expressions");
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _propertyType, "Property type");
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _propertyName, "Property name");
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _propertyDataType, "Data type");
SET_PROPERTY_FIELD_LABEL(CreateExpressionPropertyModifier, _onlySelectedParticles, "Compute only for selected particles");

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
* This modifies the input object.
******************************************************************************/
ObjectStatus CreateExpressionPropertyModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// The current animation frame number.
	int currentFrame = dataset()->animationSettings()->timeToFrame(time);

	// Initialize the evaluator class.
	ParticleExpressionEvaluator evaluator;
	evaluator.initialize(expressions(), input(), currentFrame);

	// Save list of available input variables, which will be displayed in the modifier's UI.
	_variableNames = evaluator.inputVariableNames();
	_variableTable = evaluator.inputVariableTable();

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
	std::function<bool(size_t)> selectionFilter;
	if(onlySelectedParticles()) {
		ParticlePropertyObject* selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);
		if(!selProperty)
			throw Exception(tr("Evaluation has been restricted to selected particles but no selection set has been defined."));
		OVITO_ASSERT(selProperty->size() == inputParticleCount());
		selectionFilter = [selProperty](size_t particleIndex) -> bool {
			return selProperty->getInt(particleIndex);
		};
	}

	if(inputParticleCount() != 0) {

		// Shared memory management is not thread-safe. Make sure the deep copy of the data has been
		// made before the worker threads are started.
		outputProperty->data();

		if(outputProperty->dataType() == qMetaTypeId<int>()) {
			evaluator.evaluate([outputProperty](size_t particleIndex, size_t componentIndex, double value) {
				// Store computed integer value.
				outputProperty->setIntComponent(particleIndex, componentIndex, (int)value);
			}, selectionFilter);
		}
		else {
			evaluator.evaluate([outputProperty](size_t particleIndex, size_t componentIndex, double value) {
				// Store computed float value.
				outputProperty->setFloatComponent(particleIndex, componentIndex, (FloatType)value);
			}, selectionFilter);
		}

		outputProperty->changed();
	}

	if(evaluator.isTimeDependent())
		validityInterval.intersect(time);

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
	PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
	ParticleExpressionEvaluator evaluator;
	evaluator.createInputVariables(input);
	_variableNames = evaluator.inputVariableNames();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CreateExpressionPropertyModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	rollout = createRollout(tr("Compute property"), rolloutParams, "particles.modifiers.compute_property.html");

    // Create the rollout contents.
	QVBoxLayout* mainLayout = new QVBoxLayout(rollout);
	mainLayout->setContentsMargins(4,4,4,4);

	QGroupBox* propertiesGroupBox = new QGroupBox(tr("Property"));
	mainLayout->addWidget(propertiesGroupBox);
	QGridLayout* propertiesLayout = new QGridLayout(propertiesGroupBox);
	propertiesLayout->setContentsMargins(4,4,4,4);
	propertiesLayout->setColumnStretch(1, 1);
	propertiesLayout->setSpacing(4);

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
	expressionsLayout->setSpacing(1);

	// Status label.
	mainLayout->addWidget(statusLabel());

	QWidget* variablesRollout = createRollout(tr("Variables"), rolloutParams.after(rollout), "particles.modifiers.compute_property.html");
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
		edit->setWordList(mod->inputVariableNames());
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

	variableNamesList->setText(mod->inputVariableTable());

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

	undoableTransaction(tr("Change expression"), [mod, edit, index]() {
		QStringList expr = mod->expressions();
		expr[index] = edit->text();
		mod->setExpressions(expr);
	});
}

};	// End of namespace
