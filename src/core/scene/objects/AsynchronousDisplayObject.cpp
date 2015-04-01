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

#include <core/Core.h>
#include <core/utilities/concurrent/TaskManager.h>
#include "AsynchronousDisplayObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AsynchronousDisplayObject, DisplayObject);

/******************************************************************************
* Constructs the display object.
******************************************************************************/
AsynchronousDisplayObject::AsynchronousDisplayObject(DataSet* dataset) : DisplayObject(dataset),
		_cacheValidity(TimeInterval::empty())
{
	connect(&_engineWatcher, &FutureWatcher::finished, this, &AsynchronousDisplayObject::computeEngineFinished);
}

/******************************************************************************
* Sets the status of this display object.
******************************************************************************/
void AsynchronousDisplayObject::setStatus(const PipelineStatus& status)
{
	if(status == _status) return;
	_status = status;
	notifyDependents(ReferenceEvent::ObjectStatusChanged);
}

/******************************************************************************
* Cancels any running background job.
******************************************************************************/
void AsynchronousDisplayObject::stopRunningEngine()
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
* Lets the display object prepare the data for rendering.
******************************************************************************/
void AsynchronousDisplayObject::prepare(TimePoint time, DataObject* dataObject, PipelineFlowState& flowState)
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
void AsynchronousDisplayObject::computeEngineFinished()
{
	OVITO_ASSERT(_runningEngine);

	if(!_runningEngine->isCanceled()) {
		try {
			// Throw exception if compute engine aborted with an error.
			_runningEngine->waitForFinished();

			// Store results of compute engine for later use.
			transferComputationResults(_runningEngine.get());

			// Notify dependents that the background operation has succeeded and data is ready.
			_computationStatus = PipelineStatus::Success;
		}
		catch(const Exception& ex) {
			// Transfer error message into status.
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

	// Set the new status.
	setStatus(_computationStatus);

	// Notify dependents that new data is available.
	notifyDependents(ReferenceEvent::PendingStateChanged);
}

/******************************************************************************
* Asks this object to delete itself.
******************************************************************************/
void AsynchronousDisplayObject::deleteReferenceObject()
{
	// Interrupt running compute engine when object is deleted.
	stopRunningEngine();

	DisplayObject::deleteReferenceObject();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

