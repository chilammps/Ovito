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
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <viz/util/OnTheFlyNeighborListBuilder.h>

#include "CreateBondsModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, CreateBondsModifier, AsynchronousParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, CreateBondsModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(CreateBondsModifier, CreateBondsModifierEditor)
DEFINE_PROPERTY_FIELD(CreateBondsModifier, _cutoff, "Cutoff")
DEFINE_FLAGS_REFERENCE_FIELD(CreateBondsModifier, _bondsDisplay, "BondsDisplay", BondsDisplay, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(CreateBondsModifier, _bondsObj, "BondsObject", BondsObject, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _cutoff, "Cutoff radius")
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _bondsDisplay, "Bonds display")
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _bondsObj, "Bonds")
SET_PROPERTY_FIELD_UNITS(CreateBondsModifier, _cutoff, WorldParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CreateBondsModifier::CreateBondsModifier() : _cutoff(3.2)
{
	INIT_PROPERTY_FIELD(CreateBondsModifier::_cutoff);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_bondsDisplay);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_bondsObj);

	// Load the default cutoff radius stored in the application settings.
	QSettings settings;
	settings.beginGroup("viz/bonds");
	setCutoff(settings.value("DefaultCutoff", _cutoff).value<FloatType>());
	settings.endGroup();

	// Create the output object.
	_bondsObj = new BondsObject();

	// Create the display object for bonds rendering and assign it to the scene object.
	_bondsDisplay = new BondsDisplay();
	_bondsObj->setDisplayObject(_bondsDisplay);
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CreateBondsModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute results when the parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(CreateBondsModifier::_cutoff))
			invalidateCachedResults();
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Handles reference events sent by reference targets of this object.
******************************************************************************/
bool CreateBondsModifier::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Do not propagate messages from the attached output and display objects.
	if(source == _bondsDisplay || source == _bondsObj)
		return false;

	return AsynchronousParticleModifier::referenceEvent(source, event);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> CreateBondsModifier::createEngine(TimePoint time)
{
	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCell* simCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<BondGenerationEngine>(posProperty->storage(), simCell->data(), cutoff());
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CreateBondsModifier::BondGenerationEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Generating bonds"));

	// Prepare the neighbor list.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(_positions.data(), _simCell) || futureInterface.isCanceled())
		return;

#if 0
	// Create output storage.
	ParticleProperty* output = structures();

	// Perform analysis on each particle.
	parallelFor(particleCount, futureInterface, [&neighborListBuilder, output](size_t index) {
		output->setInt(index, determineStructureFixed(neighborListBuilder, index));
	});
#endif
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void CreateBondsModifier::retrieveModifierResults(Engine* engine)
{
	BondGenerationEngine* eng = static_cast<BondGenerationEngine*>(engine);
	if(eng->bonds() && bondsObject())
		bondsObject()->setStorage(eng->bonds());
}

/******************************************************************************
* This lets the modifier insert the previously computed results into the pipeline.
******************************************************************************/
ObjectStatus CreateBondsModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	// Insert output object into pipeline.
	if(bondsObject())
		output().addObject(bondsObject());

	return ObjectStatus::Success;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CreateBondsModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Create bonds"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(6);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(0,0,0,0);
	gridlayout->setColumnStretch(1, 1);

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(CreateBondsModifier::_cutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);
	connect(cutoffRadiusPUI->spinner(), SIGNAL(spinnerValueChanged()), this, SLOT(memorizeCutoff()));

	layout1->addLayout(gridlayout);

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	// Open a sub-editor for the bonds display object.
	new SubObjectParameterUI(this, PROPERTY_FIELD(CreateBondsModifier::_bondsDisplay), rolloutParams.after(rollout));
}

/******************************************************************************
* Stores the current cutoff radius in the application settings
* so it can be used as default value for new modifiers in the future.
******************************************************************************/
void CreateBondsModifierEditor::memorizeCutoff()
{
	if(!editObject()) return;
	CreateBondsModifier* modifier = static_object_cast<CreateBondsModifier>(editObject());

	QSettings settings;
	settings.beginGroup("viz/bonds");
	settings.setValue("DefaultCutoff", modifier->cutoff());
	settings.endGroup();
}

};	// End of namespace
