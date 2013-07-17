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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/properties/AffineTransformationParameterUI.h>
#include <core/gui/undo/UndoManager.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/pipeline/PipelineObject.h>
#include "AffineTransformationModifier.h"

#include <QtConcurrent>

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AffineTransformationModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, AffineTransformationModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(AffineTransformationModifier, AffineTransformationModifierEditor)
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _transformationTM, "Transformation")
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _applyToParticles, "ApplyToParticles")
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _toSelectionOnly, "SelectionOnly")
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _applyToSimulationBox, "ApplyToSimulationBox")
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _destinationCell, "DestinationCell")
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _relativeMode, "RelativeMode")
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _transformationTM, "Transformation")
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _applyToParticles, "Transform particles")
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _toSelectionOnly, "Selected particles only")
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _applyToSimulationBox, "Transform simulation cell")
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _destinationCell, "Destination cell geometry")
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _relativeMode, "Relative transformation")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AffineTransformationModifier::AffineTransformationModifier() :
	_applyToParticles(true), _toSelectionOnly(false), _applyToSimulationBox(false),
	_transformationTM(AffineTransformation::Identity()), _destinationCell(AffineTransformation::Zero()),
	_relativeMode(true)
{
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_transformationTM);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_applyToParticles);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_toSelectionOnly);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_applyToSimulationBox);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_destinationCell);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_relativeMode);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a PipelineObject.
******************************************************************************/
void AffineTransformationModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Take the simulation cell from the input object as the default destination cell geometry for absolute scaling.
	if((AffineTransformation)_destinationCell == AffineTransformation::Zero()) {
		PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
		SimulationCell* cell = input.findObject<SimulationCell>();
		if(cell) {
			_destinationCell = cell->cellMatrix();
		}
	}
}

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
ObjectStatus AffineTransformationModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	AffineTransformation tm;
	if(_relativeMode) {
		tm = _transformationTM;
		if(applyToSimulationBox()) {
			AffineTransformation deformedCell = tm * expectSimulationCell()->cellMatrix();
			outputSimulationCell()->setCellMatrix(deformedCell);
		}
	}
	else {
		AffineTransformation oldCell = expectSimulationCell()->cellMatrix();
		if(oldCell.determinant() == 0.0)
			throw Exception(tr("Input simulation cell is degenerate."));
		tm = (AffineTransformation)_destinationCell * oldCell.inverse();
		if(applyToSimulationBox())
			outputSimulationCell()->setCellMatrix(_destinationCell);
	}

	if(applyToParticles()) {
		expectStandardProperty(ParticleProperty::PositionProperty);
		ParticlePropertyObject* posProperty = outputStandardProperty(ParticleProperty::PositionProperty);

		if(toSelectionOnly()) {
			ParticlePropertyObject* selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);
			if(selProperty) {
				const int* sbegin = selProperty->constDataInt();
				Point3* pbegin = posProperty->dataPoint3();
				Point3* pend = pbegin + posProperty->size();
				QtConcurrent::blockingMap(pbegin, pend, [tm, pbegin, sbegin](Point3& p) {
					if(sbegin[&p - pbegin])
						p = tm * p;
				});
			}
		}
		else {
			Point3* pbegin = posProperty->dataPoint3();
			Point3* pend = pbegin + posProperty->size();
			QtConcurrent::blockingMap(pbegin, pend, [tm](Point3& p) { p = tm * p; });
		}

		posProperty->changed();
	}

	return ObjectStatus();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AffineTransformationModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the first rollout.
	QWidget* rollout = createRollout(tr("Affine transformation"), rolloutParams);

    QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);
	layout->setColumnStretch(0, 5);
	layout->setColumnStretch(1, 95);

	BooleanParameterUI* applyToSimulationBoxUI = new BooleanParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_applyToSimulationBox));
	layout->addWidget(applyToSimulationBoxUI->checkBox(), 0, 0, 1, 2);

	BooleanParameterUI* applyToParticlesUI = new BooleanParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_applyToParticles));
	layout->addWidget(applyToParticlesUI->checkBox(), 1, 0, 1, 2);

	BooleanRadioButtonParameterUI* selectionUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_toSelectionOnly));

	selectionUI->buttonFalse()->setText(tr("All particles"));
	selectionUI->buttonFalse()->setEnabled(false);
	layout->addWidget(selectionUI->buttonFalse(), 2, 1);
	connect(applyToParticlesUI->checkBox(), SIGNAL(toggled(bool)), selectionUI->buttonFalse(), SLOT(setEnabled(bool)));

	selectionUI->buttonTrue()->setText(tr("Only to selected particles"));
	selectionUI->buttonTrue()->setEnabled(false);
	layout->addWidget(selectionUI->buttonTrue(), 3, 1);
	connect(applyToParticlesUI->checkBox(), SIGNAL(toggled(bool)), selectionUI->buttonTrue(), SLOT(setEnabled(bool)));

	// Create the second rollout.
	rollout = createRollout(tr("Transformation"), rolloutParams.after(rollout));

	QVBoxLayout* topLayout = new QVBoxLayout(rollout);
	topLayout->setContentsMargins(8,8,8,8);
	topLayout->setSpacing(4);

	BooleanRadioButtonParameterUI* relativeModeUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_relativeMode));

	relativeModeUI->buttonTrue()->setText(tr("Transformation matrix:"));
	topLayout->addWidget(relativeModeUI->buttonTrue());

    layout = new QGridLayout();
	layout->setContentsMargins(30,4,4,4);
	layout->setHorizontalSpacing(0);
	layout->setVerticalSpacing(2);
	topLayout->addLayout(layout);

	layout->addWidget(new QLabel(tr("Rotate/Scale/Shear:")), 0, 0, 1, 8);
	for(int col = 0; col < 3; col++) {
		layout->setColumnStretch(col*3 + 0, 1);
		if(col < 2) layout->setColumnMinimumWidth(col*3 + 2, 4);
		for(int row=0; row<4; row++) {
			QLineEdit* lineEdit = new QLineEdit(rollout);
			SpinnerWidget* spinner = new SpinnerWidget(rollout);
			lineEdit->setEnabled(false);
			spinner->setEnabled(false);
			if(row < 3) {
				elementSpinners[row][col] = spinner;
				spinner->setProperty("column", col);
				spinner->setProperty("row", row);
			}
			else {
				elementSpinners[col][row] = spinner;
				spinner->setProperty("column", row);
				spinner->setProperty("row", col);
			}
			spinner->setTextBox(lineEdit);

			int gridRow = (row == 3) ? 5 : (row+1);
			layout->addWidget(lineEdit, gridRow, col*3 + 0);
			layout->addWidget(spinner, gridRow, col*3 + 1);

			connect(spinner, SIGNAL(spinnerValueChanged()), this, SLOT(onSpinnerValueChanged()));
			connect(spinner, SIGNAL(spinnerDragStart()), this, SLOT(onSpinnerDragStart()));
			connect(spinner, SIGNAL(spinnerDragStop()), this, SLOT(onSpinnerDragStop()));
			connect(spinner, SIGNAL(spinnerDragAbort()), this, SLOT(onSpinnerDragAbort()));
			connect(relativeModeUI->buttonTrue(), SIGNAL(toggled(bool)), spinner, SLOT(setEnabled(bool)));
			connect(relativeModeUI->buttonTrue(), SIGNAL(toggled(bool)), lineEdit, SLOT(setEnabled(bool)));
		}
	}
	layout->addWidget(new QLabel(tr("Translation:")), 4, 0, 1, 8);

	relativeModeUI->buttonFalse()->setText(tr("Transform to target box:"));
	topLayout->addWidget(relativeModeUI->buttonFalse());

    layout = new QGridLayout();
	layout->setContentsMargins(30,4,4,4);
	layout->setHorizontalSpacing(0);
	layout->setVerticalSpacing(2);
	layout->setColumnStretch(0, 1);
	layout->setColumnStretch(3, 1);
	layout->setColumnStretch(6, 1);
	layout->setColumnMinimumWidth(2, 4);
	layout->setColumnMinimumWidth(5, 4);
	topLayout->addLayout(layout);
	AffineTransformationParameterUI* destinationCellUI;

	for(size_t v = 0; v < 3; v++) {
		layout->addWidget(new QLabel(tr("Cell vector %1:").arg(v+1)), v*2, 0, 1, 8);
		for(size_t r = 0; r < 3; r++) {
			destinationCellUI = new AffineTransformationParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_destinationCell), r, v);
			destinationCellUI->setEnabled(false);
			layout->addWidget(destinationCellUI->textBox(), v*2+1, r*3+0);
			layout->addWidget(destinationCellUI->spinner(), v*2+1, r*3+1);
			connect(relativeModeUI->buttonFalse(), SIGNAL(toggled(bool)), destinationCellUI, SLOT(setEnabled(bool)));
		}
	}

	layout->addWidget(new QLabel(tr("Cell origin:")), 6, 0, 1, 8);
	for(size_t r = 0; r < 3; r++) {
		destinationCellUI = new AffineTransformationParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_destinationCell), r, 3);
		destinationCellUI->setEnabled(false);
		layout->addWidget(destinationCellUI->textBox(), 7, r*3+0);
		layout->addWidget(destinationCellUI->spinner(), 7, r*3+1);
		connect(relativeModeUI->buttonFalse(), SIGNAL(toggled(bool)), destinationCellUI, SLOT(setEnabled(bool)));
	}

	// Update spinner values when a new object has been loaded into the editor.
	connect(this, SIGNAL(contentsChanged(RefTarget*)), this, SLOT(updateUI()));

	// Also update the displayed values when the animation time has changed.
	connect(&AnimManager::instance(), SIGNAL(timeChanged(TimePoint)), this, SLOT(updateUI()));
}

/******************************************************************************
* This method updates the displayed matrix values.
******************************************************************************/
void AffineTransformationModifierEditor::updateUI()
{
	AffineTransformationModifier* mod = dynamic_object_cast<AffineTransformationModifier>(editObject());
	if(!mod) return;

	const AffineTransformation& tm = mod->transformation();

	for(int row = 0; row < 3; row++) {
		for(int column = 0; column < 4; column++) {
			if(!elementSpinners[row][column]->isDragging())
				elementSpinners[row][column]->setFloatValue(tm(row, column));
		}
	}
}

/******************************************************************************
* Is called when the spinner value has changed.
******************************************************************************/
void AffineTransformationModifierEditor::onSpinnerValueChanged()
{
	ViewportSuspender noVPUpdate;
	if(!UndoManager::instance().isRecording()) {
		UndoManager::instance().beginCompoundOperation(tr("Change Parameter"));
		updateParameterValue();
		UndoManager::instance().endCompoundOperation();
	}
	else {
		UndoManager::instance().currentCompoundOperation()->clear();
		updateParameterValue();
	}
}

/******************************************************************************
* Takes the value entered by the user and stores it in transformation controller.
******************************************************************************/
void AffineTransformationModifierEditor::updateParameterValue()
{
	AffineTransformationModifier* mod = dynamic_object_cast<AffineTransformationModifier>(editObject());
	if(!mod) return;

	// Get the spinner whose value has changed.
	SpinnerWidget* spinner = qobject_cast<SpinnerWidget*>(sender());
	OVITO_CHECK_POINTER(spinner);

	AffineTransformation tm = mod->transformation();

	int column = spinner->property("column").toInt();
	int row = spinner->property("row").toInt();

	tm(row, column) = spinner->floatValue();
	mod->setTransformation(tm);
}

/******************************************************************************
* Is called when the user begins dragging the spinner interactively.
******************************************************************************/
void AffineTransformationModifierEditor::onSpinnerDragStart()
{
	OVITO_ASSERT(!UndoManager::instance().isRecording());
	UndoManager::instance().beginCompoundOperation(tr("Change Parameter"));
}

/******************************************************************************
* Is called when the user stops dragging the spinner interactively.
******************************************************************************/
void AffineTransformationModifierEditor::onSpinnerDragStop()
{
	OVITO_ASSERT(UndoManager::instance().isRecording());
	UndoManager::instance().endCompoundOperation();
}

/******************************************************************************
* Is called when the user aborts dragging the spinner interactively.
******************************************************************************/
void AffineTransformationModifierEditor::onSpinnerDragAbort()
{
	OVITO_ASSERT(UndoManager::instance().isRecording());
	UndoManager::instance().currentCompoundOperation()->clear();
	UndoManager::instance().endCompoundOperation();
}

};	// End of namespace
