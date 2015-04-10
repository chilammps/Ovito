///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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
#include "TrajectoryObject.h"
#include "TrajectoryDisplay.h"

namespace Ovito { namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, TrajectoryObject, DataObject);

/******************************************************************************
* Default constructor.
******************************************************************************/
TrajectoryObject::TrajectoryObject(DataSet* dataset) : DataObject(dataset), _trajectoryCount(0)
{
	addDisplayObject(new TrajectoryDisplay(dataset));
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void TrajectoryObject::saveToStream(ObjectSaveStream& stream)
{
	DataObject::saveToStream(stream);

	stream.beginChunk(0x01);
	stream << _trajectoryCount;
	stream << _sampleTimes;
	stream << _points;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void TrajectoryObject::loadFromStream(ObjectLoadStream& stream)
{
	DataObject::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream >> _trajectoryCount;
	stream >> _sampleTimes;
	stream >> _points;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> TrajectoryObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<TrajectoryObject> clone = static_object_cast<TrajectoryObject>(DataObject::clone(deepCopy, cloneHelper));

	// Shallow copy the internal data.
	clone->_points = this->_points;
	clone->_trajectoryCount = this->_trajectoryCount;
	clone->_sampleTimes = this->_sampleTimes;

	return clone;
}

/******************************************************************************
* Replaces the stored trajectories with new data.
******************************************************************************/
void TrajectoryObject::setTrajectories(int trajectoryCount, const QVector<Point3>& points, const QVector<TimePoint>& sampleTimes)
{
	OVITO_ASSERT(trajectoryCount >= 0);
	OVITO_ASSERT(points.size() == trajectoryCount * sampleTimes.size());

	class ReplaceTrajectoryOperation : public UndoableOperation {
	public:
		ReplaceTrajectoryOperation(TrajectoryObject* obj) :
			_obj(obj), _points(obj->points()), _trajectoryCount(obj->trajectoryCount()), _sampleTimes(obj->sampleTimes()) {}
		virtual void undo() override {
			auto points = _obj->points();
			auto trajectoryCount = _obj->trajectoryCount();
			auto sampleTimes = _obj->sampleTimes();
			_obj->setTrajectories(_trajectoryCount, _points, _sampleTimes);
			_points = std::move(points);
			_trajectoryCount = trajectoryCount;
			_sampleTimes = std::move(sampleTimes);
		}
	private:
		OORef<TrajectoryObject> _obj;
		QVector<Point3> _points;
		int _trajectoryCount;
		QVector<TimePoint> _sampleTimes;
	};

	// Make a backup of the old trajectories so they may be restored.
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new ReplaceTrajectoryOperation(this));

	_trajectoryCount = trajectoryCount;
	_points = points;
	_sampleTimes = sampleTimes;

	notifyDependents(ReferenceEvent::TargetChanged);
}

}	// End of namespace
}	// End of namespace
