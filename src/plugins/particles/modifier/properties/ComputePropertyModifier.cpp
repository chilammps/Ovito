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
#include <plugins/particles/util/ParticlePropertyParameterUI.h>
#include <core/gui/widgets/general/AutocompleteLineEdit.h>
#include <core/animation/AnimationSettings.h>
#include <core/scene/pipeline/PipelineObject.h>
#include "ComputePropertyModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Properties)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ComputePropertyModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(ComputePropertyModifier, ComputePropertyModifierEditor);
DEFINE_PROPERTY_FIELD(ComputePropertyModifier, _expressions, "Expressions");
DEFINE_PROPERTY_FIELD(ComputePropertyModifier, _outputProperty, "OutputProperty");
DEFINE_PROPERTY_FIELD(ComputePropertyModifier, _onlySelectedParticles, "OnlySelectedParticles");
SET_PROPERTY_FIELD_LABEL(ComputePropertyModifier, _expressions, "Expressions");
SET_PROPERTY_FIELD_LABEL(ComputePropertyModifier, _outputProperty, "Output property");
SET_PROPERTY_FIELD_LABEL(ComputePropertyModifier, _onlySelectedParticles, "Compute only for selected particles");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ComputePropertyModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Sets the number of vector components of the property to create.
******************************************************************************/
void ComputePropertyModifier::setPropertyComponentCount(int newComponentCount)
{
	if(newComponentCount == propertyComponentCount()) return;

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
* Is called when the value of a property of this object has changed.
******************************************************************************/
void ComputePropertyModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	if(field == PROPERTY_FIELD(ComputePropertyModifier::_outputProperty)) {
		if(outputProperty().type() != ParticleProperty::UserProperty)
			setPropertyComponentCount(ParticleProperty::standardPropertyComponentCount(outputProperty().type()));
		else
			setPropertyComponentCount(1);
	}
	ParticleModifier::propertyChanged(field);
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus ComputePropertyModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// The current animation frame number.
	int currentFrame = dataset()->animationSettings()->timeToFrame(time);

	// Initialize the evaluator class.
	ParticleExpressionEvaluator evaluator;
	evaluator.initialize(expressions(), input(), currentFrame);

	// Save list of available input variables, which will be displayed in the modifier's UI.
	_inputVariableNames = evaluator.inputVariableNames();
	_inputVariableTable = evaluator.inputVariableTable();

	// Prepare the deep copy of the output property.
	ParticlePropertyObject* prop;
	if(outputProperty().type() != ParticleProperty::UserProperty) {
		prop = outputStandardProperty(outputProperty().type(), onlySelectedParticles());
	}
	else if(!outputProperty().name().isEmpty() && propertyComponentCount() > 0)
		prop = outputCustomProperty(outputProperty().name(), qMetaTypeId<FloatType>(), sizeof(FloatType), propertyComponentCount(), sizeof(FloatType) * propertyComponentCount(), onlySelectedParticles());
	else
		throw Exception(tr("Output property has not been specified."));
	OVITO_CHECK_OBJECT_POINTER(prop);
	if(prop->componentCount() != propertyComponentCount())
		throw Exception(tr("Invalid number of components."));

	// Get the selection property if the application of the modifier is restricted to selected particles.
	std::function<bool(size_t)> selectionFilter;
	if(onlySelectedParticles()) {
		ParticlePropertyObject* selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);
		if(!selProperty)
			throw Exception(tr("Evaluation has been restricted to selected particles, but no particle selection is defined."));
		OVITO_ASSERT(selProperty->size() == inputParticleCount());
		selectionFilter = [selProperty](size_t particleIndex) -> bool {
			return selProperty->getInt(particleIndex);
		};
	}

	if(inputParticleCount() != 0) {

		// Shared memory management is not thread-safe. Make sure the deep copy of the data has been
		// made before the worker threads are started.
		prop->data();

		if(prop->dataType() == qMetaTypeId<int>()) {
			evaluator.evaluate([prop](size_t particleIndex, size_t componentIndex, double value) {
				// Store computed integer value.
				prop->setIntComponent(particleIndex, componentIndex, (int)value);
			}, selectionFilter);
		}
		else {
			evaluator.evaluate([prop](size_t particleIndex, size_t componentIndex, double value) {
				// Store computed float value.
				prop->setFloatComponent(particleIndex, componentIndex, (FloatType)value);
			}, selectionFilter);
		}

		prop->changed();
	}

	if(evaluator.isTimeDependent())
		validityInterval.intersect(time);

	return PipelineStatus::Success;
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void ComputePropertyModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Generate list of available input variables.
	PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
	ParticleExpressionEvaluator evaluator;
	evaluator.createInputVariables(input);
	_inputVariableNames = evaluator.inputVariableNames();
	_inputVariableTable = evaluator.inputVariableTable();
}

/******************************************************************************
* Allows the object to parse the serialized contents of a property field in a custom way.
******************************************************************************/
bool ComputePropertyModifier::loadPropertyFieldFromStream(ObjectLoadStream& stream, const ObjectLoadStream::SerializedPropertyField& serializedField)
{
	// This is to maintain compatibility with old file format.
	if(serializedField.identifier == "PropertyName") {
		QString propertyName;
		stream >> propertyName;
		setOutputProperty(ParticlePropertyReference(outputProperty().type(), propertyName));
		return true;
	}
	else if(serializedField.identifier == "PropertyType") {
		int propertyType;
		stream >> propertyType;
		setOutputProperty(ParticlePropertyReference((ParticleProperty::Type)propertyType, outputProperty().name()));
		return true;
	}
	return ParticleModifier::loadPropertyFieldFromStream(stream, serializedField);
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ComputePropertyModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	rollout = createRollout(tr("Compute property"), rolloutParams, "particles.modifiers.compute_property.html");

    // Create the rollout contents.
	QVBoxLayout* mainLayout = new QVBoxLayout(rollout);
	mainLayout->setContentsMargins(4,4,4,4);

	QGroupBox* propertiesGroupBox = new QGroupBox(tr("Output property"), rollout);
	mainLayout->addWidget(propertiesGroupBox);
	QVBoxLayout* propertiesLayout = new QVBoxLayout(propertiesGroupBox);
	propertiesLayout->setContentsMargins(6,6,6,6);
	propertiesLayout->setSpacing(4);

	// Output property
	ParticlePropertyParameterUI* outputPropertyUI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(ComputePropertyModifier::_outputProperty), false, false);
	propertiesLayout->addWidget(outputPropertyUI->comboBox());

	// Create the check box for the selection flag.
	BooleanParameterUI* selectionFlagUI = new BooleanParameterUI(this, PROPERTY_FIELD(ComputePropertyModifier::_onlySelectedParticles));
	propertiesLayout->addWidget(selectionFlagUI->checkBox());

	expressionsGroupBox = new QGroupBox(tr("Expression(s)"));
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
	variableNamesList->setTextInteractionFlags(Qt::TextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard));
	variablesLayout->addWidget(variableNamesList);

	// Update input variables list if another modifier has been loaded into the editor.
	connect(this, &ComputePropertyModifierEditor::contentsReplaced, this, &ComputePropertyModifierEditor::updateEditorFields);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool ComputePropertyModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		updateEditorFields();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Updates the enabled/disabled status of the editor's controls.
******************************************************************************/
void ComputePropertyModifierEditor::updateEditorFields()
{
	ComputePropertyModifier* mod = static_object_cast<ComputePropertyModifier>(editObject());
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
		connect(edit, &AutocompleteLineEdit::editingFinished, this, &ComputePropertyModifierEditor::onExpressionEditingFinished);
	}
	while(expr.size() < expressionBoxes.size()) {
		delete expressionBoxes.takeLast();
		delete expressionBoxLabels.takeLast();
	}
	OVITO_ASSERT(expressionBoxes.size() == expr.size());
	OVITO_ASSERT(expressionBoxLabels.size() == expr.size());

	QStringList standardPropertyComponentNames;
	if(mod->outputProperty().type() != ParticleProperty::UserProperty) {
		standardPropertyComponentNames = ParticleProperty::standardPropertyComponentNames(mod->outputProperty().type());
		if(standardPropertyComponentNames.empty())
			standardPropertyComponentNames.push_back(ParticleProperty::standardPropertyName(mod->outputProperty().type()));
	}
	for(int i = 0; i < expr.size(); i++) {
		expressionBoxes[i]->setText(expr[i]);
		if(i < standardPropertyComponentNames.size())
			expressionBoxLabels[i]->setText(tr("%1:").arg(standardPropertyComponentNames[i]));
		else if(expr.size() == 1)
			expressionBoxLabels[i]->setText(mod->outputProperty().name());
		else
			expressionBoxLabels[i]->setText(tr("Component %1:").arg(i+1));
	}

	variableNamesList->setText(mod->inputVariableTable());

	container()->updateRolloutsLater();
}

/******************************************************************************
* Is called when the user has typed in an expression.
******************************************************************************/
void ComputePropertyModifierEditor::onExpressionEditingFinished()
{
	QLineEdit* edit = (QLineEdit*)sender();
	int index = expressionBoxes.indexOf(edit);
	OVITO_ASSERT(index >= 0);

	ComputePropertyModifier* mod = static_object_cast<ComputePropertyModifier>(editObject());

	undoableTransaction(tr("Change expression"), [mod, edit, index]() {
		QStringList expr = mod->expressions();
		expr[index] = edit->text();
		mod->setExpressions(expr);
	});
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
