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

#ifndef __OVITO_ASYNC_DISPLAY_OBJECT_H
#define __OVITO_ASYNC_DISPLAY_OBJECT_H

#include <core/Core.h>
#include <core/utilities/concurrent/Task.h>
#include "DisplayObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief Base class display objects that work asynchronously.
 */
class OVITO_CORE_EXPORT AsynchronousDisplayObject : public DisplayObject
{
public:

	/// Constructor.
	AsynchronousDisplayObject(DataSet* dataset);

	/// \brief Lets the display object prepare the data for rendering.
	virtual void prepare(TimePoint time, DataObject* dataObject, PipelineFlowState& flowState) override;

	/// Interrupts a running computation engine if there is one for this asynchronous display object.
	void stopRunningEngine();

	/// Asks this object to delete itself. Calls stopRunningEngine() first.
	virtual void deleteReferenceObject() override;

	/// \brief Returns a structure that describes the current status of the display object.
	virtual PipelineStatus status() const override { return _status; }

	/// Sets the status of this display object and generates a ReferenceEvent::ObjectStatusChanged event.
	void setStatus(const PipelineStatus& status);

private Q_SLOTS:

	/// Is called when the compute engine has finished.
	virtual void computeEngineFinished();

protected:

	/// Creates a computation engine that will prepare the data to be displayed.
	virtual std::shared_ptr<AsynchronousTask> createEngine(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState) = 0;

	/// Unpacks the results of the computation engine and stores them in the display object.
	virtual void transferComputationResults(AsynchronousTask* engine) = 0;

private:

	/// The currently running compute engine.
	std::shared_ptr<AsynchronousTask> _runningEngine;

	/// The watcher that is used to monitor the currently running compute engine.
	FutureWatcher _engineWatcher;

	/// The current status of the display object.
	PipelineStatus _status;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ASYNC_DISPLAY_OBJECT_H
