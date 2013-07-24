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
#include <core/utilities/units/UnitsManager.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/viewport/ViewportManager.h>

#include "SimulationCell.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, SimulationCell, SceneObject)
SET_OVITO_OBJECT_EDITOR(SimulationCell, SimulationCellEditor)
DEFINE_PROPERTY_FIELD(SimulationCell, _cellVector1, "CellVector1")
DEFINE_PROPERTY_FIELD(SimulationCell, _cellVector2, "CellVector2")
DEFINE_PROPERTY_FIELD(SimulationCell, _cellVector3, "CellVector3")
DEFINE_PROPERTY_FIELD(SimulationCell, _cellOrigin, "CellTranslation")
DEFINE_PROPERTY_FIELD(SimulationCell, _pbcX, "PeriodicX")
DEFINE_PROPERTY_FIELD(SimulationCell, _pbcY, "PeriodicY")
DEFINE_PROPERTY_FIELD(SimulationCell, _pbcZ, "PeriodicZ")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellVector1, "Cell vector 1")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellVector2, "Cell vector 2")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellVector3, "Cell vector 3")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _cellOrigin, "Cell origin")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _pbcX, "Periodic boundary conditions (X)")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _pbcY, "Periodic boundary conditions (Y)")
SET_PROPERTY_FIELD_LABEL(SimulationCell, _pbcZ, "Periodic boundary conditions (Z)")
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellVector1, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellVector2, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellVector3, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SimulationCell, _cellOrigin, WorldParameterUnit)

/******************************************************************************
* Creates the storage for the internal parameters.
******************************************************************************/
void SimulationCell::init()
{
	INIT_PROPERTY_FIELD(SimulationCell::_cellVector1);
	INIT_PROPERTY_FIELD(SimulationCell::_cellVector2);
	INIT_PROPERTY_FIELD(SimulationCell::_cellVector3);
	INIT_PROPERTY_FIELD(SimulationCell::_cellOrigin);
	INIT_PROPERTY_FIELD(SimulationCell::_pbcX);
	INIT_PROPERTY_FIELD(SimulationCell::_pbcY);
	INIT_PROPERTY_FIELD(SimulationCell::_pbcZ);
}

IMPLEMENT_OVITO_OBJECT(Viz, SimulationCellEditor, PropertiesEditor)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SimulationCellEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create first rollout.
	QWidget* rollout = createRollout(tr("Simulation cell"), rolloutParams);

	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(8);

	{
		QGroupBox* pbcGroupBox = new QGroupBox(tr("Periodic boundary conditions"), rollout);
		layout1->addWidget(pbcGroupBox);

		QGridLayout* layout2 = new QGridLayout(pbcGroupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);

		BooleanParameterUI* pbcxPUI = new BooleanParameterUI(this, PROPERTY_FIELD(SimulationCell::_pbcX));
		pbcxPUI->checkBox()->setText("X");
		layout2->addWidget(pbcxPUI->checkBox(), 0, 0);

		BooleanParameterUI* pbcyPUI = new BooleanParameterUI(this, PROPERTY_FIELD(SimulationCell::_pbcY));
		pbcyPUI->checkBox()->setText("Y");
		layout2->addWidget(pbcyPUI->checkBox(), 0, 1);

		BooleanParameterUI* pbczPUI = new BooleanParameterUI(this, PROPERTY_FIELD(SimulationCell::_pbcZ));
		pbczPUI->checkBox()->setText("Z");
		layout2->addWidget(pbczPUI->checkBox(), 0, 2);
	}

	{
		QGroupBox* sizeGroupBox = new QGroupBox(tr("Size"), rollout);
		layout1->addWidget(sizeGroupBox);

		QGridLayout* layout2 = new QGridLayout(sizeGroupBox);
		layout2->setContentsMargins(4,4,4,4);
#ifndef Q_OS_MACX
		layout2->setSpacing(0);
#endif
		layout2->setColumnStretch(1, 1);

		QSignalMapper* signalMapperValueChanged = new QSignalMapper(this);
		QSignalMapper* signalMapperDragStart = new QSignalMapper(this);
		QSignalMapper* signalMapperDragStop = new QSignalMapper(this);
		QSignalMapper* signalMapperDragAbort = new QSignalMapper(this);
		for(int i = 0; i < 3; i++) {
			QLineEdit* textBox = new QLineEdit(rollout);
			simCellSizeSpinners[i] = new SpinnerWidget(rollout, textBox);
			simCellSizeSpinners[i]->setUnit(UnitsManager::instance().worldUnit());
			simCellSizeSpinners[i]->setMinValue(0.0);
			layout2->addWidget(textBox, i, 1);
			layout2->addWidget(simCellSizeSpinners[i], i, 2);

			connect(simCellSizeSpinners[i], SIGNAL(spinnerValueChanged()), signalMapperValueChanged, SLOT(map()));
			connect(simCellSizeSpinners[i], SIGNAL(spinnerDragStart()), signalMapperDragStart, SLOT(map()));
			connect(simCellSizeSpinners[i], SIGNAL(spinnerDragStop()), signalMapperDragStop, SLOT(map()));
			connect(simCellSizeSpinners[i], SIGNAL(spinnerDragAbort()), signalMapperDragAbort, SLOT(map()));

			signalMapperValueChanged->setMapping(simCellSizeSpinners[i], i);
			signalMapperDragStart->setMapping(simCellSizeSpinners[i], i);
			signalMapperDragStop->setMapping(simCellSizeSpinners[i], i);
			signalMapperDragAbort->setMapping(simCellSizeSpinners[i], i);
		}
		connect(signalMapperValueChanged, SIGNAL(mapped(int)), this, SLOT(onSizeSpinnerValueChanged(int)));
		connect(signalMapperDragStart, SIGNAL(mapped(int)), this, SLOT(onSizeSpinnerDragStart(int)));
		connect(signalMapperDragStop, SIGNAL(mapped(int)), this, SLOT(onSizeSpinnerDragStop(int)));
		connect(signalMapperDragAbort, SIGNAL(mapped(int)), this, SLOT(onSizeSpinnerDragAbort(int)));
		layout2->addWidget(new QLabel(tr("Width (X):")), 0, 0);
		layout2->addWidget(new QLabel(tr("Length (Y):")), 1, 0);
		layout2->addWidget(new QLabel(tr("Height (Z):")), 2, 0);

		connect(this, SIGNAL(contentsChanged(RefTarget*)), this, SLOT(updateSimulationBoxSize()));
	}

	// Create second rollout.
	rollout = createRollout(tr("Simulation cell vectors"), rolloutParams.collapse());

	layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(0);

	QString xyz[3] = { QString("X: "), QString("Y: "), QString("Z: ") };

	{	// First cell vector.
		layout1->addWidget(new QLabel(tr("Cell vector 1:"), rollout));
		QGridLayout* layout2 = new QGridLayout();
		layout2->setContentsMargins(0,0,0,0);
		layout2->setSpacing(0);
		layout2->setColumnStretch(1, 1);
		layout1->addLayout(layout2);
		for(int i = 0; i < 3; i++) {
			Vector3ParameterUI* vPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(SimulationCell::_cellVector1), i);
			layout2->addWidget(new QLabel(xyz[i]), i, 0);
			layout2->addWidget(vPUI->textBox(), i, 1);
			layout2->addWidget(vPUI->spinner(), i, 2);
		}
	}

	{	// Second cell vector.
		layout1->addWidget(new QLabel(tr("Cell vector 2:"), rollout));
		QGridLayout* layout2 = new QGridLayout();
		layout2->setContentsMargins(0,0,0,0);
		layout2->setSpacing(0);
		layout2->setColumnStretch(1, 1);
		layout1->addLayout(layout2);
		for(int i = 0; i < 3; i++) {
			Vector3ParameterUI* vPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(SimulationCell::_cellVector2), i);
			layout2->addWidget(new QLabel(xyz[i]), i, 0);
			layout2->addWidget(vPUI->textBox(), i, 1);
			layout2->addWidget(vPUI->spinner(), i, 2);
		}
	}

	{	// Third cell vector.
		layout1->addWidget(new QLabel(tr("Cell vector 3:"), rollout));
		QGridLayout* layout2 = new QGridLayout();
		layout2->setContentsMargins(0,0,0,0);
		layout2->setSpacing(0);
		layout2->setColumnStretch(1, 1);
		layout1->addLayout(layout2);
		for(int i = 0; i < 3; i++) {
			Vector3ParameterUI* vPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(SimulationCell::_cellVector3), i);
			layout2->addWidget(new QLabel(xyz[i]), i, 0);
			layout2->addWidget(vPUI->textBox(), i, 1);
			layout2->addWidget(vPUI->spinner(), i, 2);
		}
	}

	layout1->addSpacing(6);

	{	// Cell origin.
		layout1->addWidget(new QLabel(tr("Cell origin:"), rollout));
		QGridLayout* layout2 = new QGridLayout();
		layout2->setContentsMargins(0,0,0,0);
		layout2->setSpacing(0);
		layout2->setColumnStretch(1, 1);
		layout1->addLayout(layout2);
		for(size_t i=0; i<3; i++) {
			Vector3ParameterUI* vPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(SimulationCell::_cellOrigin), i);
			layout2->addWidget(new QLabel(xyz[i]), i, 0);
			layout2->addWidget(vPUI->textBox(), i, 1);
			layout2->addWidget(vPUI->spinner(), i, 2);
		}
	}
}

/******************************************************************************
* After the user has changed a spinner value, this method changes the
* simulation cell geometry.
******************************************************************************/
void SimulationCellEditor::changeSimulationBoxSize(int dim)
{
	OVITO_ASSERT(dim >=0 && dim < 3);

	SimulationCell* cell = static_object_cast<SimulationCell>(editObject());
	if(!cell) return;

	AffineTransformation cellTM = cell->cellMatrix();
	FloatType newSize = simCellSizeSpinners[dim]->floatValue();
	cellTM.column(3)[dim] -= 0.5 * (newSize - cellTM(dim, dim));
	cellTM(dim, dim) = newSize;
	cell->setCellMatrix(cellTM);
}

/******************************************************************************
* After the simulation cell size has changed, updates the UI controls.
******************************************************************************/
void SimulationCellEditor::updateSimulationBoxSize()
{
	SimulationCell* cell = static_object_cast<SimulationCell>(editObject());
	if(!cell) return;

	AffineTransformation cellTM = cell->cellMatrix();
	for(int dim = 0; dim < 3; dim++) {
		if(simCellSizeSpinners[dim]->isDragging() == false)
			simCellSizeSpinners[dim]->setFloatValue(cellTM(dim,dim));
	}
}

/******************************************************************************
* Is called when a spinner's value has changed.
******************************************************************************/
void SimulationCellEditor::onSizeSpinnerValueChanged(int dim)
{
	ViewportSuspender noVPUpdate;
	if(!UndoManager::instance().isRecording()) {
		UndoableTransaction::handleExceptions(tr("Change simulation cell size"), [this, dim]() {
			changeSimulationBoxSize(dim);
		});
	}
	else {
		UndoManager::instance().currentCompoundOperation()->clear();
		changeSimulationBoxSize(dim);
	}
}

/******************************************************************************
* Is called when the user begins dragging a spinner interactively.
******************************************************************************/
void SimulationCellEditor::onSizeSpinnerDragStart(int dim)
{
	OVITO_ASSERT(!UndoManager::instance().isRecording());
	UndoManager::instance().beginCompoundOperation(tr("Change simulation cell size"));
}

/******************************************************************************
* Is called when the user stops dragging a spinner interactively.
******************************************************************************/
void SimulationCellEditor::onSizeSpinnerDragStop(int dim)
{
	OVITO_ASSERT(UndoManager::instance().isRecording());
	UndoManager::instance().endCompoundOperation();
}

/******************************************************************************
* Is called when the user aborts dragging a spinner interactively.
******************************************************************************/
void SimulationCellEditor::onSizeSpinnerDragAbort(int dim)
{
	OVITO_ASSERT(UndoManager::instance().isRecording());
	UndoManager::instance().currentCompoundOperation()->clear();
	UndoManager::instance().endCompoundOperation();
}

};
