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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include "DislocationSegment.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, DislocationSegment, RefTarget);
DEFINE_REFERENCE_FIELD(DislocationSegment, _cluster, "Cluster", Cluster);
DEFINE_REFERENCE_FIELD(DislocationSegment, _burgersVectorFamily, "BurgersVectorFamily", BurgersVectorFamily);
DEFINE_PROPERTY_FIELD(DislocationSegment, _burgersVector, "BurgersVector");
DEFINE_PROPERTY_FIELD(DislocationSegment, _isVisible, "IsVisible");

/******************************************************************************
* Constructs a new dislocation segment.
******************************************************************************/
DislocationSegment::DislocationSegment(DataSet* dataset) : RefTarget(dataset),
	_isClosedLoop(false), _burgersVector(Vector3::Zero()), _length(0), _isVisible(true)
{
	INIT_PROPERTY_FIELD(DislocationSegment::_cluster);
	INIT_PROPERTY_FIELD(DislocationSegment::_burgersVectorFamily);
	INIT_PROPERTY_FIELD(DislocationSegment::_burgersVector);
	INIT_PROPERTY_FIELD(DislocationSegment::_isVisible);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void DislocationSegment::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);

	OVITO_ASSERT(_line.size() == _coreSize.size());
	stream.beginChunk(0x01);
	stream << _isClosedLoop;
	stream << _line;
	stream << _coreSize;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void DislocationSegment::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream >> _isClosedLoop;
	stream >> _line;
	stream >> _coreSize;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> DislocationSegment::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<DislocationSegment> clone = static_object_cast<DislocationSegment>(RefTarget::clone(deepCopy, cloneHelper));

	clone->_isClosedLoop = this->_isClosedLoop;
	clone->_line = this->_line;
	clone->_coreSize = this->_coreSize;
	clone->_length = this->_length;

	return clone;
}

/******************************************************************************
* Sets the Burgers vector of the segment and the cluster the segment is embedded in.
******************************************************************************/
void DislocationSegment::setBurgersVector(const Vector3& burgersVector, Cluster* cluster)
{
	OVITO_CHECK_POINTER(cluster);
	_burgersVector = burgersVector;
	_cluster = cluster;
	// Determine the Burgers vector family the segment belongs to.
	BurgersVectorFamily* newFamily = NULL;
	Q_FOREACH(BurgersVectorFamily* family, cluster->pattern()->burgersVectorFamilies()) {
		if(family->isMember(burgersVector)) {
			newFamily = family;
			break;
		}
	}
	if(newFamily == NULL)
		newFamily = cluster->pattern()->defaultBurgersVectorFamily();
	setBurgersVectorFamily(newFamily);
}

/******************************************************************************
* Returns the length of the dislocation segment.
******************************************************************************/
FloatType DislocationSegment::length()
{
	if(_length == 0) {
		for(auto v1 = _line.cbegin(), v2 = v1 + 1; v2 != _line.cend(); v1 = v2, ++v2)
			_length += (*v2 - *v1).length();
	}
	return _length;
}

/******************************************************************************
* Checks if the given floating point number is integer.
******************************************************************************/
static bool isInteger(FloatType v, int& intPart)
{
	static const FloatType epsilon = 1e-2f;
	FloatType ip;
	FloatType frac = std::modf(v, &ip);
	if(frac >= -epsilon && frac <= epsilon) intPart = (int)ip;
	else if(frac >= FloatType(1)-epsilon) intPart = (int)ip + 1;
	else if(frac <= FloatType(-1)+epsilon) intPart = (int)ip - 1;
	else return false;
	return true;
}

/******************************************************************************
* Generates a pretty string representation of the Burgers vector.
******************************************************************************/
QString DislocationSegment::formatBurgersVector(const Vector3& b)
{
	FloatType smallestCompnt = FLOATTYPE_MAX;
	for(int i = 0; i < 3; i++) {
		FloatType c = std::abs(b[i]);
		if(c < smallestCompnt && c > 1e-3)
			smallestCompnt = c;
	}
	if(smallestCompnt != FLOATTYPE_MAX) {
		FloatType m = FloatType(1) / smallestCompnt;
		for(int f = 1; f <= 11; f++) {
			int multiplier;
			if(!isInteger(m*f, multiplier)) continue;
			if(multiplier < 80) {
				Vector3 bm = b * (FloatType)multiplier;
				Vector3I bmi;
				if(isInteger(bm.x(),bmi.x()) && isInteger(bm.y(),bmi.y()) && isInteger(bm.z(),bmi.z())) {
					return QString("1/%1[%2 %3 %4]")
							.arg(multiplier)
							.arg(bmi.x()).arg(bmi.y()).arg(bmi.z());
				}
			}
		}
	}

	return QString("%1 %2 %3")
			.arg(QLocale::c().toString(b.x(), 'f'), 7)
			.arg(QLocale::c().toString(b.y(), 'f'), 7)
			.arg(QLocale::c().toString(b.z(), 'f'), 7);
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
