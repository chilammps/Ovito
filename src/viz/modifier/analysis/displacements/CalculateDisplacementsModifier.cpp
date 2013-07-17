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
#include <core/scene/objects/SceneObject.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/FilenameParameterUI.h>
#include "CalculateDisplacementsModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, CalculateDisplacementsModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, CalculateDisplacementsModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(CalculateDisplacementsModifier, CalculateDisplacementsModifierEditor)
DEFINE_REFERENCE_FIELD(CalculateDisplacementsModifier, _referenceObject, "Reference Configuration", SceneObject)
DEFINE_PROPERTY_FIELD(CalculateDisplacementsModifier, _referenceShown, "ShowReferenceConfiguration")
DEFINE_PROPERTY_FIELD(CalculateDisplacementsModifier, _eliminateCellDeformation, "EliminateCellDeformation")
DEFINE_PROPERTY_FIELD(CalculateDisplacementsModifier, _assumeUnwrappedCoordinates, "AssumeUnwrappedCoordinates")
SET_PROPERTY_FIELD_LABEL(CalculateDisplacementsModifier, _referenceObject, "Reference Configuration")
SET_PROPERTY_FIELD_LABEL(CalculateDisplacementsModifier, _referenceShown, "Show reference configuration")
SET_PROPERTY_FIELD_LABEL(CalculateDisplacementsModifier, _eliminateCellDeformation, "Eliminate homogeneous cell deformation")
SET_PROPERTY_FIELD_LABEL(CalculateDisplacementsModifier, _assumeUnwrappedCoordinates, "Assume unwrapped coordinates")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CalculateDisplacementsModifier::CalculateDisplacementsModifier() :
	_referenceShown(false), _eliminateCellDeformation(false), _assumeUnwrappedCoordinates(false)
{
	INIT_PROPERTY_FIELD(CalculateDisplacementsModifier::_referenceObject);
	INIT_PROPERTY_FIELD(CalculateDisplacementsModifier::_referenceShown);
	INIT_PROPERTY_FIELD(CalculateDisplacementsModifier::_eliminateCellDeformation);
	INIT_PROPERTY_FIELD(CalculateDisplacementsModifier::_assumeUnwrappedCoordinates);

	OORef<LinkedFileObject> importObj(new LinkedFileObject());
	importObj->setAdjustAnimationIntervalEnabled(false);
	_referenceObject = importObj;
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval CalculateDisplacementsModifier::modifierValidity(TimePoint time)
{
	TimeInterval interval = ParticleModifier::modifierValidity(time);
	if(_referenceObject) {
		interval.intersect(_referenceObject->objectValidity(time));
		PipelineFlowState refState = _referenceObject->evaluate(time);
		interval.intersect(refState.stateValidity());
	}
	return interval;
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus CalculateDisplacementsModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the reference positions of the particles.
	if(!referenceConfiguration())
		throw Exception(tr("Cannot calculate displacement vectors. No reference configuration has been specified."));

	// Get the reference configuration.
	PipelineFlowState refState = referenceConfiguration()->evaluate(time);
	if(refState.isEmpty()) {
		if(refState.status().type() != ObjectStatus::Pending)
			throw Exception(tr("No reference configuration has been specified yet."));
		else
			return ObjectStatus(ObjectStatus::Pending, QString(), tr("Waiting for input data to become ready..."));
	}
	validityInterval.intersect(refState.stateValidity());

	// Get the reference position property.
	ParticlePropertyObject* refPosProperty = nullptr;
	for(const auto& o : refState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(property && property->type() == ParticleProperty::PositionProperty) {
			refPosProperty = property;
			break;
		}
	}
	if(!refPosProperty)
		throw Exception(tr("The reference configuration does not contain particle positions."));

	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Deformed and reference configuration must contain the same number of particles.
	if(posProperty->size() != refPosProperty->size()) {
		if(refState.status().type() != ObjectStatus::Pending)
			throw Exception(tr("Cannot calculate displacement vectors. Numbers of particles in reference configuration and current configuration do not match."));
		else
			return ObjectStatus(ObjectStatus::Pending, QString(), tr("Waiting for input data to become ready..."));
	}

	// Get simulation cells.
	SimulationCell* inputCell = expectSimulationCell();
	SimulationCell* refCell = refState.findObject<SimulationCell>();
	if(!refCell)
		throw Exception(tr("Reference configuration does not contain simulation cell info."));

	// If enabled, feed particle positions from reference configuration into geometry pipeline.
	ParticlePropertyObject* outputPosProperty = nullptr;
	if(referenceShown()) {
		outputPosProperty = outputStandardProperty(ParticleProperty::PositionProperty);
		OVITO_ASSERT(outputPosProperty->size() == refPosProperty->size());
		outputPosProperty->replaceStorage(refPosProperty->storage());
		outputSimulationCell()->setCellMatrix(refCell->cellMatrix());
	}

	// Create the displacement property.
	ParticlePropertyObject* displacementProperty = outputStandardProperty(ParticleProperty::DisplacementProperty);
	OVITO_ASSERT(displacementProperty->size() == refPosProperty->size());
	OVITO_ASSERT(displacementProperty->size() == posProperty->size());

	// Get simulation cell info.
	const std::array<bool, 3> pbc = inputCell->pbcFlags();
	AffineTransformation simCell;
	AffineTransformation simCellRef;
	if(referenceShown()) {
		simCellRef = inputCell->cellMatrix();
		simCell = refCell->cellMatrix();
	}
	else {
		simCellRef = refCell->cellMatrix();
		simCell = inputCell->cellMatrix();
	}

	AffineTransformation simCellInv;
	AffineTransformation simCellRefInv;
	if(eliminateCellDeformation()) {
		if(fabs(simCell.determinant()) < FLOATTYPE_EPSILON || fabs(simCellRef.determinant()) < FLOATTYPE_EPSILON)
			throw Exception(tr("Simulation cell is degenerate in either the deformed or the reference configuration."));

		simCellInv = simCell.inverse();
		simCellRefInv = simCellRef.inverse();
	}

	const Point3* u0 = refPosProperty->constDataPoint3();
	const Point3* u = posProperty->constDataPoint3();
	Vector3* d = displacementProperty->dataVector3();
	Vector3* d_end = d + displacementProperty->size();
	for(; d != d_end; ++d, ++u, ++u0) {
		if(eliminateCellDeformation()) {
			Point3 ru = simCellInv * (*u);
			Point3 ru0 = simCellRefInv * (*u0);
			Vector3 delta = ru - ru0;
			if(!assumeUnwrappedCoordinates()) {
				for(int k = 0; k < 3; k++) {
					if(!pbc[k]) continue;
					if(delta[k] > 0.5) delta[k] -= 1.0;
					else if(delta[k] < -0.5) delta[k] += 1.0;
				}
			}
			*d = simCellRef * delta;
		}
		else {
			*d = *u - *u0;
			if(!assumeUnwrappedCoordinates()) {
				for(int k = 0; k < 3; k++) {
					if(!pbc[k]) continue;
					if((*d + simCellRef.column(k)).squaredLength() < d->squaredLength())
						*d += simCellRef.column(k);
					else if((*d - simCellRef.column(k)).squaredLength() < d->squaredLength())
						*d -= simCellRef.column(k);
				}
			}
		}
		if(referenceShown()) {
			*d = -(*d);
		}
	}
	displacementProperty->changed();

	return refState.status().type();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CalculateDisplacementsModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Calculate displacements"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
#ifndef Q_WS_MAC
	layout->setSpacing(0);
#endif

	BooleanParameterUI* eliminateCellDeformationUI = new BooleanParameterUI(this, PROPERTY_FIELD(CalculateDisplacementsModifier::_eliminateCellDeformation));
	layout->addWidget(eliminateCellDeformationUI->checkBox());

	BooleanParameterUI* assumeUnwrappedUI = new BooleanParameterUI(this, PROPERTY_FIELD(CalculateDisplacementsModifier::_assumeUnwrappedCoordinates));
	layout->addWidget(assumeUnwrappedUI->checkBox());

	BooleanParameterUI* showReferenceUI = new BooleanParameterUI(this, PROPERTY_FIELD(CalculateDisplacementsModifier::_referenceShown));
	layout->addWidget(showReferenceUI->checkBox());

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());

	// Open a sub-editor for the displacement display object.
	//new SubObjectParameterUI(this, PROPERTY_FIELD(CalculateDisplacementsModifier::_displacementChannelPrototype), rolloutParams.after(rollout));

	// Open a sub-editor for the reference object.
	subObjectUI = new SubObjectParameterUI(this, PROPERTY_FIELD(CalculateDisplacementsModifier::_referenceObject));
}

};	// End of namespace
