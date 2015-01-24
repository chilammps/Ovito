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
#include <plugins/crystalanalysis/data/dislocations/DislocationNetwork.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include "ShiftModifier.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, ShiftModifier, Modifier);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, ShiftModifierEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(ShiftModifier, ShiftModifierEditor);
DEFINE_REFERENCE_FIELD(ShiftModifier, _translation, "Translation", Controller);
SET_PROPERTY_FIELD_LABEL(ShiftModifier, _translation, "Translation");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ShiftModifier::ShiftModifier(DataSet* dataset) : Modifier(dataset)
{
	INIT_PROPERTY_FIELD(ShiftModifier::_translation);
	_translation = ControllerManager::instance().createVector3Controller(dataset);
}

/******************************************************************************
* Asks the modifier whether it can be applied to the given input data.
******************************************************************************/
bool ShiftModifier::isApplicableTo(const PipelineFlowState& input)
{
	return (input.findObject<SurfaceMesh>() != nullptr)
			|| (input.findObject<DislocationNetwork>() != nullptr);
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval ShiftModifier::modifierValidity(TimePoint time)
{
	TimeInterval interval = Modifier::modifierValidity(time);
	interval.intersect(_translation->validityInterval(time));
	return interval;
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus ShiftModifier::modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state)
{
	TimeInterval validityInterval = TimeInterval::infinite();

	// Get translation vector.
	Vector3 t = Vector3::Zero();
	if(_translation) _translation->getVector3Value(time, t, validityInterval);
	state.intersectStateValidity(validityInterval);

	if(t == Vector3::Zero())
		return PipelineStatus::Success;

	CloneHelper cloneHelper;

	for(int index = 0; index < state.objects().size(); index++) {

		// Apply translation to vertices of surface mesh.
		if(SurfaceMesh* inputSurface = dynamic_object_cast<SurfaceMesh>(state.objects()[index])) {
			OORef<SurfaceMesh> outputSurface = cloneHelper.cloneObject(inputSurface, false);
			for(HalfEdgeMesh::Vertex* vertex : outputSurface->mesh().vertices())
				vertex->pos() += t;
			outputSurface->notifyDependents(ReferenceEvent::TargetChanged);
			state.replaceObject(inputSurface, outputSurface);
		}

		// Apply translation to dislocation lines.
		else if(DislocationNetwork* inputDislocations = dynamic_object_cast<DislocationNetwork>(state.objects()[index])) {
			OORef<DislocationNetwork> outputDislocations = cloneHelper.cloneObject(inputDislocations, false);
			for(DislocationSegment* segment : outputDislocations->segments()) {
				QVector<Point3> line = segment->line();
				for(Point3& p : line)
					p += t;
				segment->setLine(line, segment->coreSize());
			}
			outputDislocations->notifyDependents(ReferenceEvent::TargetChanged);
			state.replaceObject(inputDislocations, outputDislocations);
		}
	}

	return PipelineStatus::Success;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ShiftModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the first rollout.
	QWidget* rollout = createRollout(tr("Shift"), rolloutParams);

    QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);
	layout->setColumnStretch(1, 1);

	// Translation vector parameter.
	for(int i = 0; i < 3; i++) {
		Vector3ParameterUI* translationPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(ShiftModifier::_translation), i);
		layout->addWidget(translationPUI->label(), i+1, 0);
		layout->addLayout(translationPUI->createFieldLayout(), i+1, 1);
	}
}

}	// End of namespace
}	// End of namespace
}	// End of namespace

