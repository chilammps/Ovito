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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/properties/AffineTransformationParameterUI.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/AnimationSettings.h>
#include <plugins/particles/objects/SurfaceMesh.h>
#include "AffineTransformationModifier.h"

#include <QtConcurrent>

namespace Ovito { namespace Particles { namespace Modifiers { namespace Modify {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, AffineTransformationModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(AffineTransformationModifier, Internal::AffineTransformationModifierEditor);
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _transformationTM, "Transformation");
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _applyToParticles, "ApplyToParticles");
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _toSelectionOnly, "SelectionOnly");
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _applyToSimulationBox, "ApplyToSimulationBox");
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _targetCell, "DestinationCell");
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _relativeMode, "RelativeMode");
DEFINE_PROPERTY_FIELD(AffineTransformationModifier, _applyToSurfaceMesh, "ApplyToSurfaceMesh");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _transformationTM, "Transformation");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _applyToParticles, "Transform particles");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _toSelectionOnly, "Selected particles only");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _applyToSimulationBox, "Transform simulation cell");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _targetCell, "Destination cell geometry");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _relativeMode, "Relative transformation");
SET_PROPERTY_FIELD_LABEL(AffineTransformationModifier, _applyToSurfaceMesh, "Transform surface mesh");

namespace Internal {
	IMPLEMENT_OVITO_OBJECT(Particles, AffineTransformationModifierEditor, ParticleModifierEditor);
}

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AffineTransformationModifier::AffineTransformationModifier(DataSet* dataset) : ParticleModifier(dataset),
	_applyToParticles(true), _toSelectionOnly(false), _applyToSimulationBox(false),
	_transformationTM(AffineTransformation::Identity()), _targetCell(AffineTransformation::Zero()),
	_relativeMode(true), _applyToSurfaceMesh(true)
{
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_transformationTM);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_applyToParticles);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_toSelectionOnly);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_applyToSimulationBox);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_targetCell);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_relativeMode);
	INIT_PROPERTY_FIELD(AffineTransformationModifier::_applyToSurfaceMesh);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a PipelineObject.
******************************************************************************/
void AffineTransformationModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Take the simulation cell from the input object as the default destination cell geometry for absolute scaling.
	if(targetCell() == AffineTransformation::Zero()) {
		PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
		SimulationCellObject* cell = input.findObject<SimulationCellObject>();
		if(cell)
			setTargetCell(cell->cellMatrix());
	}
}

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
PipelineStatus AffineTransformationModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	AffineTransformation tm;
	if(relativeMode()) {
		tm = transformation();
		if(applyToSimulationBox()) {
			AffineTransformation deformedCell = tm * expectSimulationCell()->cellMatrix();
			outputSimulationCell()->setCellMatrix(deformedCell);
		}
	}
	else {
		AffineTransformation oldCell = expectSimulationCell()->cellMatrix();
		if(oldCell.determinant() == 0)
			throw Exception(tr("Input simulation cell is degenerate."));
		tm = targetCell() * oldCell.inverse();
		if(applyToSimulationBox())
			outputSimulationCell()->setCellMatrix(targetCell());
	}

	if(applyToParticles()) {
		expectStandardProperty(ParticleProperty::PositionProperty);
		ParticlePropertyObject* posProperty = outputStandardProperty(ParticleProperty::PositionProperty, true);

		if(selectionOnly()) {
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
			Point3* const pbegin = posProperty->dataPoint3();
			Point3* const pend = pbegin + posProperty->size();

			// Check if the matrix describes a pure translation. If yes, we can
			// simply add vectors instead of computing full matrix products.
			Vector3 translation = tm.translation();
			if(tm == AffineTransformation::translation(translation)) {
				for(Point3* p = pbegin; p != pend; ++p)
					*p += translation;
			}
			else {
				QtConcurrent::blockingMap(pbegin, pend, [tm](Point3& p) { p = tm * p; });
			}
		}

		posProperty->changed();
	}

	if(applyToSurfaceMesh()) {
		for(int index = 0; index < input().objects().size(); index++) {
			// Apply transformation to vertices of surface mesh.
			if(SurfaceMesh* inputSurface = dynamic_object_cast<SurfaceMesh>(input().objects()[index])) {
				OORef<SurfaceMesh> outputSurface = cloneHelper()->cloneObject(inputSurface, false);
				for(HalfEdgeMesh::Vertex* vertex : outputSurface->mesh().vertices())
					vertex->pos() = tm * vertex->pos();
				outputSurface->notifyDependents(ReferenceEvent::TargetChanged);
				output().replaceObject(inputSurface, outputSurface);
			}
		}
	}

	return PipelineStatus::Success;
}

namespace Internal {

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AffineTransformationModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the first rollout.
	QWidget* rollout = createRollout(tr("Affine transformation"), rolloutParams, "particles.modifiers.affine_transformation.html");

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
	connect(applyToParticlesUI->checkBox(), &QCheckBox::toggled, selectionUI->buttonFalse(), &QRadioButton::setEnabled);

	selectionUI->buttonTrue()->setText(tr("Only to selected particles"));
	selectionUI->buttonTrue()->setEnabled(false);
	layout->addWidget(selectionUI->buttonTrue(), 3, 1);
	connect(applyToParticlesUI->checkBox(), &QCheckBox::toggled, selectionUI->buttonTrue(), &QRadioButton::setEnabled);

	BooleanParameterUI* applyToSurfaceMeshUI = new BooleanParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_applyToSurfaceMesh));
	layout->addWidget(applyToSurfaceMeshUI->checkBox(), 4, 0, 1, 2);

	// Create the second rollout.
	rollout = createRollout(tr("Transformation"), rolloutParams.after(rollout), "particles.modifiers.affine_transformation.html");

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

			connect(spinner, &SpinnerWidget::spinnerValueChanged, this, &AffineTransformationModifierEditor::onSpinnerValueChanged);
			connect(spinner, &SpinnerWidget::spinnerDragStart, this, &AffineTransformationModifierEditor::onSpinnerDragStart);
			connect(spinner, &SpinnerWidget::spinnerDragStop, this, &AffineTransformationModifierEditor::onSpinnerDragStop);
			connect(spinner, &SpinnerWidget::spinnerDragAbort, this, &AffineTransformationModifierEditor::onSpinnerDragAbort);
			connect(relativeModeUI->buttonTrue(), &QRadioButton::toggled, spinner, &SpinnerWidget::setEnabled);
			connect(relativeModeUI->buttonTrue(), &QRadioButton::toggled, lineEdit, &QLineEdit::setEnabled);
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
			destinationCellUI = new AffineTransformationParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_targetCell), r, v);
			destinationCellUI->setEnabled(false);
			layout->addWidget(destinationCellUI->textBox(), v*2+1, r*3+0);
			layout->addWidget(destinationCellUI->spinner(), v*2+1, r*3+1);
			connect(relativeModeUI->buttonFalse(), &QRadioButton::toggled, destinationCellUI, &AffineTransformationParameterUI::setEnabled);
		}
	}

	layout->addWidget(new QLabel(tr("Cell origin:")), 6, 0, 1, 8);
	for(size_t r = 0; r < 3; r++) {
		destinationCellUI = new AffineTransformationParameterUI(this, PROPERTY_FIELD(AffineTransformationModifier::_targetCell), r, 3);
		destinationCellUI->setEnabled(false);
		layout->addWidget(destinationCellUI->textBox(), 7, r*3+0);
		layout->addWidget(destinationCellUI->spinner(), 7, r*3+1);
		connect(relativeModeUI->buttonFalse(), &QRadioButton::toggled, destinationCellUI, &AffineTransformationParameterUI::setEnabled);
	}

	// Update spinner values when a new object has been loaded into the editor.
	connect(this, &PropertiesEditor::contentsChanged, this, &AffineTransformationModifierEditor::updateUI);
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
	if(!dataset()->undoStack().isRecording()) {
		UndoableTransaction transaction(dataset()->undoStack(), tr("Change parameter"));
		updateParameterValue();
		transaction.commit();
	}
	else {
		dataset()->undoStack().resetCurrentCompoundOperation();
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
	OVITO_ASSERT(!dataset()->undoStack().isRecording());
	dataset()->undoStack().beginCompoundOperation(tr("Change parameter"));
}

/******************************************************************************
* Is called when the user stops dragging the spinner interactively.
******************************************************************************/
void AffineTransformationModifierEditor::onSpinnerDragStop()
{
	OVITO_ASSERT(dataset()->undoStack().isRecording());
	dataset()->undoStack().endCompoundOperation();
}

/******************************************************************************
* Is called when the user aborts dragging the spinner interactively.
******************************************************************************/
void AffineTransformationModifierEditor::onSpinnerDragAbort()
{
	OVITO_ASSERT(dataset()->undoStack().isRecording());
	dataset()->undoStack().endCompoundOperation(false);
}

}	// End of namespace

}}}}	// End of namespace
