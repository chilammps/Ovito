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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <plugins/particles/objects/SurfaceMesh.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include "SmoothSurfaceModifier.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, SmoothSurfaceModifier, Modifier);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, SmoothSurfaceModifierEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(SmoothSurfaceModifier, SmoothSurfaceModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(SmoothSurfaceModifier, _smoothingLevel, "SmoothingLevel", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(SmoothSurfaceModifier, _smoothingLevel, "Smoothing level");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
SmoothSurfaceModifier::SmoothSurfaceModifier(DataSet* dataset) : Modifier(dataset), _smoothingLevel(8)
{
	INIT_PROPERTY_FIELD(SmoothSurfaceModifier::_smoothingLevel);
}

/******************************************************************************
* Asks the modifier whether it can be applied to the given input data.
******************************************************************************/
bool SmoothSurfaceModifier::isApplicableTo(const PipelineFlowState& input)
{
	return (input.findObject<SurfaceMesh>() != nullptr);
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus SmoothSurfaceModifier::modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state)
{
	if(_smoothingLevel <= 0)
		return PipelineStatus::Success;

	// Get simulation cell geometry and periodic boundary flags.
	SimulationCell cell;
	if(SimulationCellObject* simulationCellObj = state.findObject<SimulationCellObject>())
		cell = simulationCellObj->data();
	else
		cell.setPbcFlags(false, false, false);

	CloneHelper cloneHelper;
	for(int index = 0; index < state.objects().size(); index++) {
		if(SurfaceMesh* inputSurface = dynamic_object_cast<SurfaceMesh>(state.objects()[index])) {
			OORef<SurfaceMesh> outputSurface = cloneHelper.cloneObject(inputSurface, false);
			outputSurface->smoothMesh(cell, _smoothingLevel);
			state.replaceObject(inputSurface, outputSurface);
		}
	}
	return PipelineStatus::Success;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SmoothSurfaceModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the first rollout.
	QWidget* rollout = createRollout(tr("Smooth surface"), rolloutParams);

    QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);
	layout->setColumnStretch(1, 1);

	IntegerParameterUI* smoothingLevelUI = new IntegerParameterUI(this, PROPERTY_FIELD(SmoothSurfaceModifier::_smoothingLevel));
	layout->addWidget(smoothingLevelUI->label(), 0, 0);
	layout->addLayout(smoothingLevelUI->createFieldLayout(), 0, 1);
	smoothingLevelUI->setMinValue(0);
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
