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
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/ProgressManager.h>

#include "AsynchronousParticleModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AsynchronousParticleModifier, ParticleModifier)
DEFINE_PROPERTY_FIELD(AsynchronousParticleModifier, _autoUpdate, "AutoUpdate")
SET_PROPERTY_FIELD_LABEL(AsynchronousParticleModifier, _autoUpdate, "Automatic update")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AsynchronousParticleModifier::AsynchronousParticleModifier() : _autoUpdate(true),
	_cacheValidity(TimeInterval::empty()), _computationValidity(TimeInterval::empty())
{
	INIT_PROPERTY_FIELD(AsynchronousParticleModifier::_autoUpdate);

	connect(&_analysisOperationWatcher, &FutureWatcher::finished, this, &AsynchronousParticleModifier::backgroundJobFinished);
}

/******************************************************************************
* This method is called by the system when an item in the modification pipeline
* located before this modifier has changed.
******************************************************************************/
void AsynchronousParticleModifier::modifierInputChanged(ModifierApplication* modApp)
{
	ParticleModifier::modifierInputChanged(modApp);

	_cacheValidity.setEmpty();
	cancelBackgroundJob();
}

/******************************************************************************
* Cancels any running background job.
******************************************************************************/
void AsynchronousParticleModifier::cancelBackgroundJob()
{
	if(_analysisOperation.isValid()) {
		try {
			_analysisOperationWatcher.unsetFuture();
			_analysisOperation.cancel();
			_analysisOperation.waitForFinished();
		} catch(...) {}
		_analysisOperation.reset();
		if(status().type() == ObjectStatus::Pending)
			setStatus(ObjectStatus());
	}
	_computationValidity.setEmpty();
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus AsynchronousParticleModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	if(structureTypes().size() != NUM_STRUCTURE_TYPES)
		throw Exception(tr("The number of structure types has changed. Please remove this modifier from the modification pipeline and insert it again."));

	// Get input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCell* simCell = expectSimulationCell();

	if(autoUpdate() && !_cacheValidity.contains(time)) {

		if(!_computationValidity.contains(time)) {

			// Stop running job first.
			cancelBackgroundJob();

			// Start a background job to compute the modifier's results.
			_computationValidity.setInstant(time);
			_analysisOperation = runInBackground<QExplicitlySharedDataPointer<ParticleProperty>>(std::bind(&AsynchronousParticleModifier::performAnalysis, this, std::placeholders::_1, posProperty->storage(), simCell->data()));
			ProgressManager::instance().addTask(_analysisOperation);
			_analysisOperationWatcher.setFuture(_analysisOperation);
		}
	}

	if(!_structureProperty || !_cacheValidity.contains(time)) {
		if(!_computationValidity.contains(time))
			throw Exception(tr("The analysis has not been performed yet."));
		else
			return ObjectStatus::Pending;
	}

	if(inputParticleCount() != particleStructures().size()) {
		if(!_computationValidity.contains(time))
			throw Exception(tr("The number of input particles has changed. The cached analysis results have become invalid."));
		else
			return ObjectStatus::Pending;
	}

	// Get output property object.
	ParticleTypeProperty* structureProperty = static_object_cast<ParticleTypeProperty>(outputStandardProperty(ParticleProperty::StructureTypeProperty));

	// Insert structure types into output property.
	structureProperty->setParticleTypes(structureTypes());

	// Insert results into output property.
	structureProperty->replaceStorage(_structureProperty);

	// Build list of type colors.
	Color structureTypeColors[NUM_STRUCTURE_TYPES];
	for(int index = 0; index < NUM_STRUCTURE_TYPES; index++) {
		structureTypeColors[index] = structureTypes()[index]->color();
	}

	// Assign colors to particles based on structure type.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);
	OVITO_ASSERT(colorProperty->size() == particleStructures().size());
	const int* s = particleStructures().constDataInt();
	Color* c = colorProperty->dataColor();
	Color* c_end = c + colorProperty->size();
	for(; c != c_end; ++s, ++c) {
		OVITO_ASSERT(*s >= 0 && *s < NUM_STRUCTURE_TYPES);
		*c = structureTypeColors[*s];
		//typeCounters[*s]++;
	}
	colorProperty->changed();

	if(!_computationValidity.contains(time))
		return ObjectStatus::Success;
	else
		return ObjectStatus::Pending;
}

/******************************************************************************
* This is called when the background analysis task has finished.
******************************************************************************/
void AsynchronousParticleModifier::backgroundJobFinished()
{
	OVITO_ASSERT(!_computationValidity.isEmpty());
	ReferenceEvent::Type notificationType = ReferenceEvent::PendingOperationFailed;
	bool wasCanceled = _analysisOperation.isCanceled();
	ObjectStatus newStatus = status();

	if(!wasCanceled) {
		try {
			_structureProperty = _analysisOperation.result().data();
			_cacheValidity = _computationValidity;

			// Notify dependents that the background operation has succeeded and new data is available.
			notificationType = ReferenceEvent::PendingOperationSucceeded;
			newStatus = ObjectStatus::Success;
		}
		catch(Exception& ex) {
			// Transfer exception message to evaluation status.
			newStatus = ObjectStatus(ObjectStatus::Error, ex.messages().join(QChar('\n')));
		}
	}
	else {
		newStatus = ObjectStatus(ObjectStatus::Error, tr("Operation has been canceled by the user."));
	}
	if(!_structureProperty)
		_structureProperty = new ParticleProperty(0, ParticleProperty::StructureTypeProperty);

	// Reset everything.
	_analysisOperationWatcher.unsetFuture();
	_analysisOperation.reset();
	_computationValidity.setEmpty();

	// Set the new modifier status.
	setStatus(newStatus);

	// Notify dependents that the evaluation request was satisfied or not satisfied.
	notifyDependents(notificationType);
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void AsynchronousParticleModifier::performAnalysis(FutureInterface<QExplicitlySharedDataPointer<ParticleProperty>>& futureInterface, QSharedDataPointer<ParticleProperty> positions, SimulationCellData simCell)
{
	futureInterface.setProgressText(tr("Performing bond angle analysis"));
	futureInterface.setProgressRange(100);

	QExplicitlySharedDataPointer<ParticleProperty> output(new ParticleProperty(positions->size(), ParticleProperty::StructureTypeProperty));
	for(int i = 0; i < 100 && !futureInterface.isCanceled(); i++) {
		QThread::msleep(30);
		futureInterface.setProgressValue(i);
	}

	futureInterface.setResult(output);
}

};	// End of namespace
