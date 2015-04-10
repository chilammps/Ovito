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

#ifndef __OVITO_TRAJECTORY_OBJECT_H
#define __OVITO_TRAJECTORY_OBJECT_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DataObject.h>

namespace Ovito { namespace Particles {

/**
 * \brief Stores the trajectories of a set of particles.
 */
class OVITO_PARTICLES_EXPORT TrajectoryObject : public DataObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE TrajectoryObject(DataSet* dataset);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Particle trajectories"); }

	/// Returns the trajectory points.
	const QVector<Point3>& points() const { return _points; }

	/// Returns the number of independent trajectories stored in this data object.
	int trajectoryCount() const { return _trajectoryCount; }

	/// Returns the points in time where the trajectories have been sampled.
	const QVector<TimePoint>& sampleTimes() const { return _sampleTimes; }

	/// Replaces the stored trajectories with new data.
	void setTrajectories(int trajectoryCount, const QVector<Point3>& points, const QVector<TimePoint>& sampleTimes);

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

private:

	/// Stores the trajectory points of all particles.
	QVector<Point3> _points;

	/// The number of independent trajectories stored.
	int _trajectoryCount;

	/// The points in time where the trajectories have been sampled.
	QVector<TimePoint> _sampleTimes;

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace

#endif // __OVITO_TRAJECTORY_OBJECT_H
