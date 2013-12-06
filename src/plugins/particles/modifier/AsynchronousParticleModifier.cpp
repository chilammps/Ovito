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
#include <core/viewport/Viewport.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/DataSetContainer.h>
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/TaskManager.h>
#include "AsynchronousParticleModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, AsynchronousParticleModifier, ParticleModifier)
DEFINE_PROPERTY_FIELD(AsynchronousParticleModifier, _autoUpdate, "AutoUpdate")
DEFINE_PROPERTY_FIELD(AsynchronousParticleModifier, _saveResults, "SaveResults")
SET_PROPERTY_FIELD_LABEL(AsynchronousParticleModifier, _autoUpdate, "Automatic update")
SET_PROPERTY_FIELD_LABEL(AsynchronousParticleModifier, _saveResults, "Save results")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AsynchronousParticleModifier::AsynchronousParticleModifier(DataSet* dataset) : ParticleModifier(dataset),
	_autoUpdate(true), _saveResults(false),
	_cacheValidity(TimeInterval::empty()), _computationValidity(TimeInterval::empty()), _needsUpdate(true)
{
	INIT_PROPERTY_FIELD(AsynchronousParticleModifier::_autoUpdate);
	INIT_PROPERTY_FIELD(AsynchronousParticleModifier::_saveResults);

	connect(&_backgroundOperationWatcher, &FutureWatcher::finished, this, &AsynchronousParticleModifier::backgroundJobFinished);
}

/******************************************************************************
* This method is called by the system when an item in the modification pipeline
* located before this modifier has changed.
******************************************************************************/
void AsynchronousParticleModifier::modifierInputChanged(ModifierApplication* modApp)
{
	ParticleModifier::modifierInputChanged(modApp);
	invalidateCachedResults();
}

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool AsynchronousParticleModifier::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged || event->type() == ReferenceEvent::PendingStateChanged) {
		invalidateCachedResults();
	}
	return ParticleModifier::referenceEvent(source, event);
}

/******************************************************************************
* Invalidates the modifier's result cache so that the results will be recomputed
* next time the modifier is evaluated.
******************************************************************************/
void AsynchronousParticleModifier::invalidateCachedResults()
{
	if(autoUpdateEnabled()) {
		_needsUpdate = true;
		cancelBackgroundJob();
	}
}

/******************************************************************************
* Cancels any running background job.
******************************************************************************/
void AsynchronousParticleModifier::cancelBackgroundJob()
{
	if(_backgroundOperation.isValid()) {
		try {
			_backgroundOperationWatcher.unsetFuture();
			_backgroundOperation.cancel();
			_backgroundOperation.waitForFinished();
		} catch(...) {}
		_backgroundOperation.reset();
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
	if(autoUpdateEnabled() && _needsUpdate && input().status().type() != ObjectStatus::Pending) {

		if(!_computationValidity.contains(time)) {

			// Stop running job first.
			cancelBackgroundJob();

			// Create the engine that will compute the results.
			try {
				std::shared_ptr<Engine> engine = createEngine(time);
				OVITO_CHECK_POINTER(engine.get());

				// Start a background job that runs the engine to compute the modifier's results.
				_computationValidity.setInstant(time);
				_backgroundOperation = dataset()->container()->taskManager().runInBackground<std::shared_ptr<Engine>>(std::bind(&AsynchronousParticleModifier::runEngine, this, std::placeholders::_1, engine));
				_backgroundOperationWatcher.setFuture(_backgroundOperation);
			}
			catch(const ObjectStatus& status) {
				return status;
			}
		}
	}

	if(!_computationValidity.contains(time)) {
		if(_needsUpdate) {
			if(input().status().type() != ObjectStatus::Pending)
				throw Exception(tr("The modifier results have not been computed yet."));
			else
				return ObjectStatus(ObjectStatus::Warning, tr("Waiting for input data to become ready..."));
		}
	}
	else {

		if(_cacheValidity.contains(time)) {
			applyModifierResults(time, validityInterval);
		}

		return ObjectStatus(ObjectStatus::Pending, tr("Results are being computed..."));
	}

	if(_asyncStatus.type() == ObjectStatus::Error)
		return _asyncStatus;

	return applyModifierResults(time, validityInterval);
}

/******************************************************************************
* This function is executed in a background thread to compute the modifier results.
******************************************************************************/
void AsynchronousParticleModifier::runEngine(FutureInterface<std::shared_ptr<Engine>>& futureInterface, std::shared_ptr<Engine> engine)
{
	// Let the engine object do the actual work.
	engine->compute(futureInterface);

	// Pass engine back to caller since it carries the results.
	if(!futureInterface.isCanceled())
		futureInterface.setResult(engine);
}

/******************************************************************************
* This is called when the background analysis task has finished.
******************************************************************************/
void AsynchronousParticleModifier::backgroundJobFinished()
{
	OVITO_ASSERT(!_computationValidity.isEmpty());
	bool wasCanceled = _backgroundOperation.isCanceled();

	if(!wasCanceled) {
		_cacheValidity = _computationValidity;
		_needsUpdate = false;
		try {
			std::shared_ptr<Engine> engine = _backgroundOperation.result();
			retrieveModifierResults(engine.get());

			// Notify dependents that the background operation has succeeded and new data is available.
			_asyncStatus = ObjectStatus::Success;
		}
		catch(const Exception& ex) {
			// Transfer exception message to evaluation status.
			_asyncStatus = ObjectStatus(ObjectStatus::Error, ex.messages().join(QChar('\n')));
		}
	}
	else {
		_asyncStatus = ObjectStatus(ObjectStatus::Error, tr("Operation has been canceled by the user."));
	}

	// Reset everything.
	_backgroundOperationWatcher.unsetFuture();
	_backgroundOperation.reset();
	_computationValidity.setEmpty();

	// Set the new modifier status.
	setStatus(_asyncStatus);

	// Notify dependents that the evaluation request was satisfied or not satisfied.
	notifyDependents(ReferenceEvent::PendingStateChanged);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void AsynchronousParticleModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);
	stream.beginChunk(0x01);
	stream << (storeResultsWithScene() ? _cacheValidity : TimeInterval::empty());
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void AsynchronousParticleModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream >> _cacheValidity;
	_needsUpdate = _cacheValidity.isEmpty();
	stream.closeChunk();
}

};	// End of namespace
