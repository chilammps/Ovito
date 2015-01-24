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

#ifndef __OVITO_CA_DISLOCATION_SEGMENT_H
#define __OVITO_CA_DISLOCATION_SEGMENT_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/reference/RefTarget.h>
#include "../patterns/BurgersVectorFamily.h"
#include "../clusters/Cluster.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/**
 * \brief Stores a dislocation segment.
 */
class OVITO_CRYSTALANALYSIS_EXPORT DislocationSegment : public RefTarget
{
public:

	/// \brief Constructs a new dislocation segment.
	Q_INVOKABLE DislocationSegment(DataSet* dataset);

	/// Returns the sequence of space points that make up the dislocation segment.
	const QVector<Point3>& line() const { return _line; }

	/// Returns the core size array.
	const QVector<int>& coreSize() const { return _coreSize; }

	/// Sets the sequence of space points that make up the dislocation segment.
	void setLine(const QVector<Point3>& line, const QVector<int>& coreSize) {
		_line = line;
		_coreSize = coreSize;
		_length = 0;
	}

	/// Returns true if this segment is a closed loop.
	bool isClosedLoop() const { return _isClosedLoop; }

	/// Marks this segment as a closed loop.
	void setIsClosedLoop(bool isLoop) { _isClosedLoop = isLoop; }

	/// Returns true if this segment is an infinite dislocation line passing through a periodic boundary.
	/// A segment is considered infinite if it is a closed loop but its start and end points do not coincide.
	bool isInfiniteLine() const {
		return isClosedLoop() && !(_line.back() - _line.front()).isZero();
	}

	/// Returns the cluster the segment is embedded in.
	Cluster* cluster() const { return _cluster; }

	/// Returns the Burgers vector the segment.
	const Vector3& burgersVector() const { return _burgersVector; }

	/// Sets the Burgers vector of the segment and the cluster the segment is embedded in.
	void setBurgersVector(const Vector3& burgersVector, Cluster* cluster);

	/// Returns the Burgers vector family this segment belongs to.
	BurgersVectorFamily* burgersVectorFamily() const { return _burgersVectorFamily; }

	/// Changes the Burgers vector family this segment belongs to.
	void setBurgersVectorFamily(BurgersVectorFamily* family) { _burgersVectorFamily = family; }

	/// Returns the length of the dislocation segment.
	FloatType length();

	/// Returns whether this dislocation segment is shown.
	bool isVisible() const { return _isVisible; }

	/// Shows/hides this dislocation segment.
	void setVisible(bool visible) { _isVisible = visible; }

	/// Generates a pretty string representation of a Burgers vector.
	static QString formatBurgersVector(const Vector3& b);

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

protected:

	/// The piecewise linear curve in space.
	QVector<Point3> _line;

	/// The core radius along the dislocation line.
	QVector<int> _coreSize;

	/// Indicates if this segment is a closed loop.
	bool _isClosedLoop;

	/// The Burgers vector of the dislocation segment. It is expressed in the coordinate system of
	/// the crystal cluster which the segment is embedded in.
	PropertyField<Vector3> _burgersVector;

	/// The length of the dislocation segment.
	FloatType _length;

	/// The cluster in which the segment is embedded in.
	ReferenceField<Cluster> _cluster;

	/// The Burgers vector family this segment belongs to.
	ReferenceField<BurgersVectorFamily> _burgersVectorFamily;

	/// Controls the visibility of this dislocation segment.
	PropertyField<bool> _isVisible;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_cluster);
	DECLARE_REFERENCE_FIELD(_burgersVectorFamily);
	DECLARE_PROPERTY_FIELD(_burgersVector);
	DECLARE_PROPERTY_FIELD(_isVisible);
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CA_DISLOCATION_SEGMENT_H
