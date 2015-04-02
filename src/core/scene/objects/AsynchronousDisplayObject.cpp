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
#include <core/dataset/DataSetContainer.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include "AsynchronousDisplayObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AsynchronousDisplayObject, DisplayObject);

/******************************************************************************
* Constructs the display object.
******************************************************************************/
AsynchronousDisplayObject::AsynchronousDisplayObject(DataSet* dataset) : DisplayObject(dataset)
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
	// Create a compute engine which prepares the data for rendering.
	std::shared_ptr<AsynchronousTask> engine = createEngine(time, dataObject, flowState);
	if(engine) {

		// Stop any running engine first.
		stopRunningEngine();

		// Set status.
		setStatus(PipelineStatus(PipelineStatus::Pending, tr("Data is being prepared for rendering...")));

		// Start new compute engine.
		_runningEngine = engine;
		dataset()->container()->taskManager().runTaskAsync(_runningEngine);
		_engineWatcher.setFutureInterface(_runningEngine);
	}

	// Mark the pipeline output as pending as long as we are preparing the data for display.
	if(_runningEngine && flowState.status().type() != PipelineStatus::Pending && flowState.status().type() != PipelineStatus::Error)
		flowState.setStatus(status());
}

/******************************************************************************
* Is called when the modifier's compute engine has finished.
******************************************************************************/
void AsynchronousDisplayObject::computeEngineFinished()
{
	OVITO_ASSERT(_runningEngine);

	PipelineStatus newStatus;
	if(!_runningEngine->isCanceled()) {
		try {
			// Throw exception if compute engine aborted with an error.
			_runningEngine->waitForFinished();

			// Store results of compute engine for later use.
			transferComputationResults(_runningEngine.get());

			// Notify dependents that the background operation has succeeded and the data is ready.
			newStatus = PipelineStatus::Success;
		}
		catch(const Exception& ex) {
			// Transfer error message into status.
			newStatus = PipelineStatus(PipelineStatus::Error, ex.messages().join(QChar('\n')));
		}
	}
	else {
		newStatus = PipelineStatus(PipelineStatus::Error, tr("Operation has been canceled by the user."));

		// Let derived display object know that compute task has been canceled.
		transferComputationResults(nullptr);
	}

	// Reset everything.
	_engineWatcher.unsetFuture();
	_runningEngine.reset();

	// Set the new status.
	setStatus(newStatus);

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

