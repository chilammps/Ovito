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
#include <core/animation/AnimationSettings.h>
#include <core/viewport/Viewport.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/gui/properties/StringParameterUI.h>
#include "SelectExpressionModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Selection)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, SelectExpressionModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(SelectExpressionModifier, SelectExpressionModifierEditor);
DEFINE_PROPERTY_FIELD(SelectExpressionModifier, _expression, "Expression");
SET_PROPERTY_FIELD_LABEL(SelectExpressionModifier, _expression, "Boolean expression");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, SelectExpressionModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus SelectExpressionModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// The current animation frame number.
	int currentFrame = dataset()->animationSettings()->timeToFrame(time);

	// Initialize the evaluator class.
	ParticleExpressionEvaluator evaluator;
	evaluator.initialize(QStringList(expression()), input(), currentFrame);

	// Save list of available input variables, which will be displayed in the modifier's UI.
	_variableNames = evaluator.inputVariableNames();
	_variableTable = evaluator.inputVariableTable();

	// If the user has not yet entered an expression let him know which
	// data channels can be used in the expression.
	if(expression().isEmpty())
		return PipelineStatus(PipelineStatus::Warning, tr("Please enter a boolean expression."));

	// Check if expression contain an assignment ('=' operator).
	// This should be considered an error, because the user is probably referring the comparison operator '=='.
	if(expression().contains(QRegExp("[^=!><]=(?!=)")))
		throw Exception("The expression contains the assignment operator '='. Please use the comparison operator '==' instead.");

	// The number of selected particles.
	size_t nSelected = 0;

	// Get the deep copy of the output selection property.
	ParticlePropertyObject* selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);

	std::atomic_size_t nselected(0);
	if(inputParticleCount() != 0) {

		// Shared memory management is not thread-safe. Make sure the deep copy of the data has been
		// made before the worker threads are started.
		selProperty->data();

		evaluator.evaluate([selProperty, &nselected](size_t particleIndex, size_t componentIndex, double value) {
			if(value) {
				selProperty->setInt(particleIndex, 1);
				++nselected;
			}
			else {
				selProperty->setInt(particleIndex, 0);
			}
		});

		selProperty->changed();
	}

	if(evaluator.isTimeDependent())
		validityInterval.intersect(time);

	QString statusMessage = tr("%1 out of %2 particles selected (%3%)").arg(nselected).arg(inputParticleCount()).arg((FloatType)nselected * 100 / std::max(1,(int)inputParticleCount()), 0, 'f', 1);
	return PipelineStatus(PipelineStatus::Success, statusMessage);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void SelectExpressionModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Build list of available input variables.
	PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
	ParticleExpressionEvaluator evaluator;
	evaluator.createInputVariables(input);
	_variableNames = evaluator.inputVariableNames();
	_variableTable = evaluator.inputVariableTable();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SelectExpressionModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	QWidget* rollout = createRollout(tr("Expression select"), rolloutParams, "particles.modifiers.expression_select.html");

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

	QWidget* variablesRollout = createRollout(tr("Variables"), rolloutParams.after(rollout), "particles.modifiers.expression_select.html");
    QVBoxLayout* variablesLayout = new QVBoxLayout(variablesRollout);
    variablesLayout->setContentsMargins(4,4,4,4);
	variableNamesList = new QLabel();
	variableNamesList->setWordWrap(true);
	variableNamesList->setTextInteractionFlags(Qt::TextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard));
	variablesLayout->addWidget(variableNamesList);

	// Update input variables list if another modifier has been loaded into the editor.
	connect(this, &SelectExpressionModifierEditor::contentsReplaced, this, &SelectExpressionModifierEditor::updateEditorFields);
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

	variableNamesList->setText(mod->inputVariableTable());
	expressionLineEdit->setWordList(mod->inputVariableNames());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
