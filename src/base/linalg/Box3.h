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

/** 
 * \file Box3.h 
 * \brief Contains the definition of the Ovito::Box_3 template class.
 */

#ifndef __OVITO_BOX3_H
#define __OVITO_BOX3_H

#include <base/Base.h>
#include "Vector3.h"
#include "Point3.h"
#include "AffineTransformation.h"


namespace Ovito {

/**
 * \brief A bounding box in 3d space.
 *
 * This class stores an axis-aligned box in 3d.
 * It is defined by minimum and maximum coordinates in X, Y and Z direction.
 * 
 * There are two predefined instances of this template class: 
 * Box3 which is used for floating-point coordinates and \c Base::Box3I which is used
 * for integer coordinates. 
 */
template<typename T>
class Box_3
{
public:	
	/// The coordinates of the lower corner.
	Point_3<T> minc;
	/// The coordinates of the upper corner.
	Point_3<T> maxc;

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Creates an empty box.
	Box_3() : minc(std::numeric_limits<T>::max()), maxc(-std::numeric_limits<T>::max()) {}

	/// \brief Initializes the box with the minimum and maximum coordinates.
	/// \param minCorner A point that specifies the corner with minimum coordinates of the box.
	/// \param maxCorner A point that specifies the corner with maximum coordinates of the box.	
	Box_3(const Point_3<T>& minCorner, const Point_3<T>& maxCorner) : minc(minCorner), maxc(maxCorner) {
		OVITO_ASSERT_MSG(minc.x() <= maxc.x(), "Box_3 constructor", "X component of the minimum corner point must not be larger than the maximum corner point.");
		OVITO_ASSERT_MSG(minc.y() <= maxc.y(), "Box_3 constructor", "Y component of the minimum corner point must not be larger than the maximum corner point.");
		OVITO_ASSERT_MSG(minc.z() <= maxc.z(), "Box_3 constructor", "Z component of the minimum corner point must not be larger than the maximum corner point.");
	}

	/// \brief Crates a cubic box with the given center and half edge length.
	/// \param center The center of the cubic box.
	/// \param halfEdgeLength The half size of the cube.
	Box_3(const Point_3<T>& center, T halfEdgeLength) {
		minc.x() = center.x() - halfEdgeLength;
		minc.y() = center.y() - halfEdgeLength;
		minc.z() = center.z() - halfEdgeLength;
		maxc.x() = center.x() + halfEdgeLength;
		maxc.y() = center.y() + halfEdgeLength;
		maxc.z() = center.z() + halfEdgeLength;
	}

	///////////////////////////////// Attributes /////////////////////////////////

	/// \brief Checks whether this is an empty box.
	///
	/// The box is considered empty when one of the maximum corner coodinates is less
	/// then the minimum corner coordinate.
	/// \return true if this box is empty; false otherwise.
	constexpr bool isEmpty() const {
        return (minc.x() > maxc.x()) || (minc.y() > maxc.y()) || (minc.z() > maxc.z());
	}

	/// \brief Resets the box to the empty state.
	void setEmpty() {
		minc = Point_3<T>( std::numeric_limits<T>::max());
		maxc = Point_3<T>(-std::numeric_limits<T>::max());
	}

	/// \brief Computes the center of the box.
	/// \return The center of the box.
	constexpr Point_3<T> center() const {
		return (minc + maxc) / 2;
	}

	/// \brief Computes the size of the box.
	/// \return The difference between the maximum and minimum corner.
	constexpr Vector_3<T> size() const {
		return maxc - minc;
	}
	
	/// \brief Returns the size of the box in the given dimension.
	/// \param dimension The index of the dimension (0 - 2).
	constexpr T size(typename Point_3<T>::size_type dimension) const {
		return maxc[dimension] - minc[dimension]; 
	}

	/// Returns the size in X direction (Max.x() - Min.x()) of the box.
	constexpr T sizeX() const { return maxc.x() - minc.x(); }

	/// Returns the size in Y direction (Max.y() - Min.y()) of the box.
	constexpr T sizeY() const { return maxc.y() - minc.y(); }

	/// Returns the size in Z direction (Max.z() - Min.z()) of the box.
	constexpr T sizeZ() const { return maxc.z() - minc.z(); }

	/// \brief Returns the position of one of the eight corners of the box corner.
	/// \param i The index of the corner (0 - 7).
	/// \return The coordinate of the i-th corner of the box.
	Point_3<T> operator[](int i) const {
		OVITO_ASSERT_MSG(!isEmpty(), "Box_3::operator[]", "Cannot calculate the corner of an empty box.");
		switch(i) {
			case 0: return Point_3<T>(minc.x(), minc.y(), minc.z());
			case 1: return Point_3<T>(maxc.x(), minc.y(), minc.z());
			case 2: return Point_3<T>(minc.x(), maxc.y(), minc.z());
			case 3: return Point_3<T>(maxc.x(), maxc.y(), minc.z());
			case 4: return Point_3<T>(minc.x(), minc.y(), maxc.z());
			case 5: return Point_3<T>(maxc.x(), minc.y(), maxc.z());
			case 6: return Point_3<T>(minc.x(), maxc.y(), maxc.z());
			case 7: return Point_3<T>(maxc.x(), maxc.y(), maxc.z());
			default:				
				OVITO_ASSERT_MSG(false, "Box3::operator[]", "Corner index out of range.");
                throw std::invalid_argument("Corner index out of range.");
				return Point_3<T>::Origin();
		}
	}

	/////////////////////////////// Classification ///////////////////////////////
    
	/// \brief Checks whether a point is inside the box.
	/// \param p The point to test.
	/// \return true if the given point is inside or on the edge of the bounding box; false if it is completely outside the box.
	bool contains(const Point_3<T>& p) const {
		if(p.x() < minc.x() || p.x() > maxc.x()) return false;
		if(p.y() < minc.y() || p.y() > maxc.y()) return false;
		if(p.z() < minc.z() || p.z() > maxc.z()) return false;
		return true;
	}
	
	/// \brief Classifies the given point with respect to the box.
	///
	/// Returns -1 if the point is outside of the box.
	/// Returns 0 if the point is on the boundary of the box within the given tolerance.
	/// Returns +1 if the point is inside of the box.	
	int classifyPoint(const Point_3<T>& p, T epsilon = T(FLOATTYPE_EPSILON)) const {
		if(p.x() > maxc.x() + epsilon || p.y() > maxc.y() + epsilon || p.z() > maxc.z() + epsilon) return -1;
		if(p.x() < minc.x() - epsilon || p.y() < minc.y() - epsilon || p.z() < minc.z() - epsilon) return -1;
		if(p.x() < maxc.x() - epsilon && p.x() > minc.x() + epsilon &&
		   p.y() < maxc.y() - epsilon && p.y() > minc.y() + epsilon &&
		   p.z() < maxc.z() - epsilon && p.z() > minc.z() + epsilon) return 1;
		return 0;
	}

	/// \brief Checks whether another box is contained in this box.
	/// \return true if the given box is completely inside the bounding box.
	bool containsBox(const Box_3<T>& b) const {
		return (b.minc.x() >= minc.x() && b.maxc.x() <= maxc.x()) &&
			(b.minc.y() >= minc.y() && b.maxc.y() <= maxc.y()) &&
			(b.minc.z() >= minc.z() && b.maxc.z() <= maxc.z());
	}

	/// \brief Checks wehther the intersection of two boxes is not empty.
	/// \return true if the given box is not completely outside of this box.
	bool intersects(const Box_3<T>& b) const {
		if(maxc.x() <= b.minc.x() || minc.x() >= b.maxc.x()) return false;
		if(maxc.y() <= b.minc.y() || minc.y() >= b.maxc.y()) return false;
		if(maxc.z() <= b.minc.z() || minc.z() >= b.maxc.z()) return false;
		if(isEmpty() || b.isEmpty()) return false;
		return true;
	}

    //////////////////////////////// Modification ////////////////////////////////

	/// \brief Enlarges the box to include the given point.
	/// \sa addPoints(), addBox()
	void addPoint(const Point_3<T>& p) {
		minc.x() = std::min(minc.x(), p.x()); maxc.x() = std::max(maxc.x(), p.x());
		minc.y() = std::min(minc.y(), p.y()); maxc.y() = std::max(maxc.y(), p.y());
		minc.z() = std::min(minc.z(), p.z()); maxc.z() = std::max(maxc.z(), p.z());
	}

	/// \brief Enlarges the box to include the given points.
	/// \param points Pointer to the first element of an array of points.
	/// \param count The number of points in the array.
	/// \sa addPoint()
	void addPoints(const Point_3<T>* points, std::size_t count) {
		for(; count != 0; count--, points++) {
			minc.x() = std::min(minc.x(), points->X); maxc.x() = std::max(maxc.x(), points->X);
			minc.y() = std::min(minc.y(), points->Y); maxc.y() = std::max(maxc.y(), points->Y);
			minc.z() = std::min(minc.z(), points->Z); maxc.z() = std::max(maxc.z(), points->Z);
		}
	}

	/// \brief Enlarges this box to include the given box.
	/// \sa addPoint()
	void addBox(const Box_3& b) {
		minc.x() = std::min(minc.x(), b.minc.x()); maxc.x() = std::max(maxc.x(), b.maxc.x());
		minc.y() = std::min(minc.y(), b.minc.y()); maxc.y() = std::max(maxc.y(), b.maxc.y());
		minc.z() = std::min(minc.z(), b.minc.z()); maxc.z() = std::max(maxc.z(), b.maxc.z());
	}

	/// \brief Computes the intersection of this box and a second box.
	/// 
	/// This box is clipped to the boundary of the given box.
	void clip(const Box_3& b) {
		minc.x() = std::max(minc.x(), b.minc.x()); maxc.x() = std::min(maxc.x(), b.maxc.x());
		minc.y() = std::max(minc.y(), b.minc.y()); maxc.y() = std::min(maxc.y(), b.maxc.y());
		minc.z() = std::max(minc.z(), b.minc.z()); maxc.z() = std::min(maxc.z(), b.maxc.z());
	}

	/// \brief Computes the bounding box transformed by the given matrix.
	/// \return The axis-aligned bounding box that contains the transformed input box.
	Box_3 transformed(const Matrix_34<T>& tm) const {
		if(isEmpty()) return *this;
		Box_3 b;
		b.addPoint(tm * Point_3<T>(minc.x(), minc.y(), minc.z()));
		b.addPoint(tm * Point_3<T>(maxc.x(), minc.y(), minc.z()));
		b.addPoint(tm * Point_3<T>(minc.x(), maxc.y(), minc.z()));
		b.addPoint(tm * Point_3<T>(maxc.x(), maxc.y(), minc.z()));
		b.addPoint(tm * Point_3<T>(minc.x(), minc.y(), maxc.z()));
		b.addPoint(tm * Point_3<T>(maxc.x(), minc.y(), maxc.z()));
		b.addPoint(tm * Point_3<T>(minc.x(), maxc.y(), maxc.z()));
		b.addPoint(tm * Point_3<T>(maxc.x(), maxc.y(), maxc.z()));
		return b;
	}

	/// \brief Scales the box by the given scalar factor. 
	/// The center of the box is taken as scaling center.
	Box_3 centerScale(T factor) const {
		if(isEmpty()) return *this;
		Point_3<T> c = center();
		return Box_3(c + ((minc - c) * factor), c + ((maxc - c) * factor));
	}

	/// \brief Adds the given amount of padding to each side of the box.
	/// \return The padded box.
	Box_3 padBox(T amount) const {
		if(isEmpty()) return *this;		
		return Box_3(minc - Vector_3<T>(amount), maxc + Vector_3<T>(amount));
	}

    ////////////////////////////////// Utilities /////////////////////////////////
	
	/// Returns a string representation of this box.
	QString toString() const {
			return "[Min: " + minc.toString() + " Max: " + maxc.toString() + "]";
	}
};

/// \brief Transforms a box.
/// \sa Box_3::transformed()
template<typename T>
inline Box_3<T> operator*(const Matrix_34<T>& tm, const Box_3<T>& box) {
	return box.transformed(tm);
}

/// \brief Prints the box to a text output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Box_3<T> &b) {
	return os << '[' << b.minc << "] - [" << b.maxc << ']';
}

/// \brief Writes a box to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Box_3<T>& b)
{
	return stream << b.minc << b.maxc;
}

/// \brief Reads a box from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Box_3<T>& b)
{
	return stream >> b.minc >> b.maxc;
}

/** 
 * \typedef Box_3<FloatType> Box3
 * \brief Template class instance of the Box_3 class used for floating-point calculations based on Point3. 
 */
typedef Box_3<FloatType> Box3;

/** 
 * \typedef Box3I
 * \brief Template class instance of the Box_3 class used for integer calculations based on Point3I. 
 */
typedef Box_3<int> Box3I;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Box3)
Q_DECLARE_METATYPE(Ovito::Box3I)
Q_DECLARE_TYPEINFO(Ovito::Box3, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box3I, Q_MOVABLE_TYPE);

#endif // __OVITO_BOX3_H
