///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include <core/utilities/concurrent/TaskManager.h>
#include "AsynchronousParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, AsynchronousParticleModifier, ParticleModifier);

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AsynchronousParticleModifier::AsynchronousParticleModifier(DataSet* dataset) : ParticleModifier(dataset),
		_cacheValidity(TimeInterval::empty())
{
	connect(&_engineWatcher, &FutureWatcher::finished, this, &AsynchronousParticleModifier::computeEngineFinished);
}

/******************************************************************************
* This method is called by the system when the upstream modification pipeline
* has changed.
******************************************************************************/
void AsynchronousParticleModifier::upstreamPipelineChanged(ModifierApplication* modApp)
{
	ParticleModifier::upstreamPipelineChanged(modApp);
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
	stopRunningEngine();
	_cacheValidity.setEmpty();
}

/******************************************************************************
* Cancels any running background job.
******************************************************************************/
void AsynchronousParticleModifier::stopRunningEngine()
{
	if(!_runningEngine)
		return;

	try {
		_engineWatcher.unsetFuture();
		_runningEngine->cancel();
		_runningEngine->waitForFinished();
	} catch(...) {}
	_runningEngine.reset();

	if(status().type() == PipelineStatus::Pending)
		setStatus(PipelineStatus());
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus AsynchronousParticleModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	if(input().status().type() != PipelineStatus::Pending) {
		if(!_cacheValidity.contains(time)) {
			if(!_runningEngine || !_runningEngine->validityInterval().contains(time)) {

				// Stop running engine first.
				stopRunningEngine();

				try {
					// Create the compute engine for this modifier.
					_runningEngine = createEngine(time, input().stateValidity());
				}
				catch(const PipelineStatus& status) {
					return status;
				}
				// Start compute engine.
				dataset()->container()->taskManager().runTaskAsync(_runningEngine);
				_engineWatcher.setFutureInterface(_runningEngine);
			}
		}
	}

	if(!_runningEngine || !_runningEngine->validityInterval().contains(time)) {
		if(!_cacheValidity.contains(time)) {
			if(input().status().type() != PipelineStatus::Pending)
				throw Exception(tr("The modifier results have not been computed yet."));
			else
				return PipelineStatus(PipelineStatus::Warning, tr("Waiting for input data to become ready..."));
		}
		else {
			if(_computationStatus.type() == PipelineStatus::Error)
				return _computationStatus;

			validityInterval.intersect(_cacheValidity);
			return applyComputationResults(time, validityInterval);
		}
	}
	else {
		if(_cacheValidity.contains(time)) {
			validityInterval.intersect(_cacheValidity);
			applyComputationResults(time, validityInterval);
		}
		else {
			// Try to apply old results even though they are outdated.
			validityInterval.intersect(time);
			try {
				applyComputationResults(time, validityInterval);
			}
			catch(const Exception&) { /* Ignore problems. */ }
		}

		return PipelineStatus(PipelineStatus::Pending, tr("Results are being computed..."));
	}
}

/******************************************************************************
* Is called when the modifier's compute engine has finished.
******************************************************************************/
void AsynchronousParticleModifier::computeEngineFinished()
{
	OVITO_ASSERT(_runningEngine);

	if(!_runningEngine->isCanceled()) {
		try {
			// Throw exception if compute engine aborted with an error.
			_runningEngine->waitForFinished();

			// Store results of compute engine for later use.
			transferComputationResults(_runningEngine.get());

			// Notify dependents that the background operation has succeeded and new data is available.
			_computationStatus = PipelineStatus::Success;
		}
		catch(const Exception& ex) {
			// Transfer exception message into evaluation status.
			_computationStatus = PipelineStatus(PipelineStatus::Error, ex.messages().join(QChar('\n')));
		}
		_cacheValidity = _runningEngine->validityInterval();
	}
	else {
		_computationStatus = PipelineStatus(PipelineStatus::Error, tr("Computation has been canceled by the user."));
		_cacheValidity.setEmpty();
	}

	// Reset everything.
	_engineWatcher.unsetFuture();
	_runningEngine.reset();

	// Set the new modifier status.
	setStatus(_computationStatus);

	// Notify dependents that the evaluation request was satisfied or not satisfied.
	notifyDependents(ReferenceEvent::PendingStateChanged);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void AsynchronousParticleModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);
	stream.beginChunk(0x02);
	// For future use.
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void AsynchronousParticleModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);
	stream.expectChunkRange(0, 2);
	// For future use.
	stream.closeChunk();
}

/******************************************************************************
* Asks this object to delete itself.
******************************************************************************/
void AsynchronousParticleModifier::deleteReferenceObject()
{
	// Interrupt running compute engine when modifier is deleted.
	stopRunningEngine();

	ParticleModifier::deleteReferenceObject();
}


OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
