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

/** 
 * \file
 * \brief Contains the definition of the Ovito::Box_3 class template.
 */

#ifndef __OVITO_BOX3_H
#define __OVITO_BOX3_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"
#include "Point3.h"
#include "AffineTransformation.h"


namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief An axis-aligned box in 3d space.
 *
 * The box is defined by the lower and upper X, Y and Z coordinates (#minc and #maxc fields).
 * 
 * The template parameter \a T specifies the data type used for the box coordinates.
 * Two standard instantiations of Box_3 for floating-point and integer coordinates are predefined:
 *
 * \code
 *      typedef Box_3<FloatType>  Box3;
 *      typedef Box_3<int>        Box3I;
 * \endcode
 *
 * A box is considered empty if any of its lower coordinates is greater than the corresponding
 * upper coordinate.
 *
 * \sa Box_2
 * \sa Vector_3, Point_3
 */
template<typename T>
class Box_3
{
public:	

	/// The lower XYZ coordinates of the box.
	Point_3<T> minc;
	/// The upper XYZ coordinates of the box.
	Point_3<T> maxc;

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief Constructs an empty box.
	Box_3() : minc(std::numeric_limits<T>::max()), maxc(std::numeric_limits<T>::lowest()) {}

	/// \brief Initializes the box with lower and upper coordinates.
	/// \param lower The corner of the box that specifies the lower boundary coordinates.
	/// \param upper The corner of the box that specifies the upper boundary coordinates.
	Box_3(const Point_3<T>& lower, const Point_3<T>& upper) : minc(lower), maxc(upper) {
		OVITO_ASSERT_MSG(minc.x() <= maxc.x(), "Box_3 constructor", "Lower X coordinate must not be larger than upper X coordinate.");
		OVITO_ASSERT_MSG(minc.y() <= maxc.y(), "Box_3 constructor", "Lower Y coordinate must not be larger than upper Y coordinate.");
		OVITO_ASSERT_MSG(minc.z() <= maxc.z(), "Box_3 constructor", "Lower Z coordinate must not be larger than upper Z coordinate.");
	}

	/// \brief Crates a cubic box.
	/// \param center The center of the cubic box.
	/// \param halfEdgeLength One half of the edge length of the cube.
	Box_3(const Point_3<T>& center, T halfEdgeLength) {
		OVITO_ASSERT(halfEdgeLength >= 0);
		minc.x() = center.x() - halfEdgeLength;
		minc.y() = center.y() - halfEdgeLength;
		minc.z() = center.z() - halfEdgeLength;
		maxc.x() = center.x() + halfEdgeLength;
		maxc.y() = center.y() + halfEdgeLength;
		maxc.z() = center.z() + halfEdgeLength;
	}

	///////////////////////////////// Attributes /////////////////////////////////

	/// \brief Checks whether this is box is empty.
	///
	/// The box is considered empty if one of the upper boundary coordinates is smaller than
	/// the corresponding lower boundary coordinate.
	Q_DECL_CONSTEXPR bool isEmpty() const {
        return (minc.x() > maxc.x()) || (minc.y() > maxc.y()) || (minc.z() > maxc.z());
	}

	/// \brief Resets the box to an empty state.
	void setEmpty() {
		minc = Point_3<T>(std::numeric_limits<T>::max());
		maxc = Point_3<T>(std::numeric_limits<T>::lowest());
	}

	/// \brief Computes the center of the box.
	Q_DECL_CONSTEXPR Point_3<T> center() const {
		return Point_3<T>((minc.x() + maxc.x()) / 2, (minc.y() + maxc.y()) / 2, (minc.z() + maxc.z()) / 2);
	}

	/// \brief Computes the size of the box.
	/// \return The difference between the upper and lower boundary coordinates.
	Q_DECL_CONSTEXPR Vector_3<T> size() const {
		return maxc - minc;
	}
	
	/// \brief Returns the size of the box in the given dimension.
	/// \param dimension The dimension (0 - 2).
	/// \return The difference between the upper and lower boundary of the box in the given dimension.
	Q_DECL_CONSTEXPR T size(typename Point_3<T>::size_type dimension) const {
		return maxc[dimension] - minc[dimension]; 
	}

	/// Returns the box size in X direction (maxc.x() - minc.x()) of the box.
	Q_DECL_CONSTEXPR T sizeX() const { return maxc.x() - minc.x(); }

	/// Returns the box size in Y direction (maxc.y() - minc.y()) of the box.
	Q_DECL_CONSTEXPR T sizeY() const { return maxc.y() - minc.y(); }

	/// Returns the box size in Z direction (maxc.z() - minc.z()) of the box.
	Q_DECL_CONSTEXPR T sizeZ() const { return maxc.z() - minc.z(); }

	/// \brief Returns the position of one of the eight corners of the box.
	/// \param i The index of the corner in the range 0 to 7.
	/// \return The coordinates of the i-th corner of the box.
	Point_3<T> operator[](typename Point_3<T>::size_type i) const {
		OVITO_ASSERT_MSG(!isEmpty(), "Box_3::operator[]", "Cannot compute the corners of an empty box.");
		OVITO_ASSERT_MSG(i >= 0 && i < 8, "Box_3::operator[]", "Corner index out of range.");
		const Point_3<T>* const c = &minc;
		OVITO_ASSERT(&c[1] == &maxc);
		return Point_3<T>(c[i&1].x(), c[(i>>1)&1].y(), c[(i>>2)&1].z());
	}

	/////////////////////////////// Classification ///////////////////////////////
    
	/// \brief Checks whether a point is located inside the box.
	/// \param p The input point.
	/// \return \c true if the point \a p is inside or on the boundaries of the box; \c false if it is outside the box.
	Q_DECL_CONSTEXPR bool contains(const Point_3<T>& p) const {
		return p.x() >= minc.x() && p.x() <= maxc.x() &&
			   p.y() >= minc.y() && p.y() <= maxc.y() &&
			   p.z() >= minc.z() && p.z() <= maxc.z();
	}
	
	/// \brief Classifies a point with respect to the box.
	/// \param p The input point.
	/// \param epsilon This threshold is used to test whether the point is on the boundary of the box.
	/// \return -1 if \a p is outside the box; 0 if \a p is on the boundary of the box within the specified tolerance; +1 if inside the box.
	int classifyPoint(const Point_3<T>& p, T epsilon = T(FLOATTYPE_EPSILON)) const {
		if(p.x() > maxc.x() + epsilon || p.y() > maxc.y() + epsilon || p.z() > maxc.z() + epsilon) return -1;
		if(p.x() < minc.x() - epsilon || p.y() < minc.y() - epsilon || p.z() < minc.z() - epsilon) return -1;
		if(p.x() < maxc.x() - epsilon && p.x() > minc.x() + epsilon &&
		   p.y() < maxc.y() - epsilon && p.y() > minc.y() + epsilon &&
		   p.z() < maxc.z() - epsilon && p.z() > minc.z() + epsilon) return 1;
		return 0;
	}

	/// \brief Tests if another box is contained in this box.
	/// \param b The other box.
	/// \return \c true if the box \a b is completely inside this box.
	bool containsBox(const Box_3<T>& b) const {
		return (b.minc.x() >= minc.x() && b.maxc.x() <= maxc.x()) &&
			   (b.minc.y() >= minc.y() && b.maxc.y() <= maxc.y()) &&
			   (b.minc.z() >= minc.z() && b.maxc.z() <= maxc.z());
	}

	/// \brief Tests whether the intersection of two boxes is not empty.
	/// \param b The other box.
	/// \return \c true if the box \a b is not completely outside of this box;
	///         \c false if the two boxes do not overlap or are empty.
	bool intersects(const Box_3<T>& b) const {
		if(maxc.x() <= b.minc.x() || minc.x() >= b.maxc.x()) return false;
		if(maxc.y() <= b.minc.y() || minc.y() >= b.maxc.y()) return false;
		if(maxc.z() <= b.minc.z() || minc.z() >= b.maxc.z()) return false;
		if(isEmpty() || b.isEmpty()) return false;
		return true;
	}

    //////////////////////////////// Modification ////////////////////////////////

	/// \brief Extends this box to include the given point.
	/// \param p The point which should be included in this box after the method returns.
	/// \sa addPoints(), addBox()
	inline void addPoint(const Point_3<T>& p) {
		if(p.x() < minc.x()) minc.x() = p.x();
		if(p.x() > maxc.x()) maxc.x() = p.x();
		if(p.y() < minc.y()) minc.y() = p.y();
		if(p.y() > maxc.y()) maxc.y() = p.y();
		if(p.z() < minc.z()) minc.z() = p.z();
		if(p.z() > maxc.z()) maxc.z() = p.z();
	}

	/// \brief Extends the box to include the given set of points.
	/// \param points Pointer to the first element of an array of points.
	/// \param count The number of points in the array.
	/// \sa addPoint()
	void addPoints(const Point_3<T>* points, std::size_t count) {
		const Point_3<T>* const points_end = points + count;
		for(; points != points_end; ++points)
			addPoint(*points);
	}

	/// \brief Extends this box to include the given box.
	/// \param b The other box.
	/// \sa addPoint()
	void addBox(const Box_3& b) {
		minc.x() = std::min(minc.x(), b.minc.x()); maxc.x() = std::max(maxc.x(), b.maxc.x());
		minc.y() = std::min(minc.y(), b.minc.y()); maxc.y() = std::max(maxc.y(), b.maxc.y());
		minc.z() = std::min(minc.z(), b.minc.z()); maxc.z() = std::max(maxc.z(), b.maxc.z());
	}

	/// \brief Clips this box to the boundaries of another box.
	/// \param b The other box.
	///
	/// If the two boxes do not overlap, the resulting box will be empty.
	void clip(const Box_3& b) {
		minc.x() = std::max(minc.x(), b.minc.x()); maxc.x() = std::min(maxc.x(), b.maxc.x());
		minc.y() = std::max(minc.y(), b.minc.y()); maxc.y() = std::min(maxc.y(), b.maxc.y());
		minc.z() = std::max(minc.z(), b.minc.z()); maxc.z() = std::min(maxc.z(), b.maxc.z());
	}

	/// \brief Computes the bounding box after transforming the corners of this box by the given matrix.
	/// \param tm An affine transformation matrix that will be applied to the corners of this box.
	/// \return The axis-aligned bounding box that fully contains the transformed input box.
	///
	/// Transforming any empty box will result in an empty box.
	Box_3 transformed(const AffineTransformationT<T>& tm) const {
		if(isEmpty()) return *this;
		Box_3 b;
		const Point_3<T>* const c = &minc;
		OVITO_ASSERT(&c[1] == &maxc);
		for(size_t i = 0; i < 8; i++)
			b.addPoint(tm * Point_3<T>(c[i&1].x(), c[(i>>1)&1].y(), c[(i>>2)&1].z()));
		return b;
	}

	/// \brief Scales the box size by a scalar factor and returns the new scaled box.
	/// The center of the box is not changed.
	Box_3 centerScale(T factor) const {
		if(isEmpty()) return *this;
		Point_3<T> c = center();
		return Box_3(c + ((minc - c) * factor), c + ((maxc - c) * factor));
	}

	/// \brief Returns a copy of this with a padding added to each side of the box.
	/// \param amount The amount of padding to add to each side.
	/// \return The padded box.
	///
	/// \note An empty box will remain empty, and no padding is added.
	Box_3 padBox(T amount) const {
		if(isEmpty()) return *this;		
		return Box_3(minc - Vector_3<T>(amount), maxc + Vector_3<T>(amount));
	}

    ////////////////////////////////// Utilities /////////////////////////////////
	
	/// Generates a string representation of the box.
	QString toString() const {
		return "[Min: " + minc.toString() + " Max: " + maxc.toString() + "]";
	}
};

/// \brief Transforms a box. Returns the axis-aligned bounding box that contains all transformed corner points.
/// \sa Box_3::transformed()
/// \relates Box_3
template<typename T>
inline Box_3<T> operator*(const AffineTransformationT<T>& tm, const Box_3<T>& box) {
	return box.transformed(tm);
}

/// \brief Prints a box to a text output stream.
/// \relates Box_3
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Box_3<T> &b) {
	return os << '[' << b.minc << "] - [" << b.maxc << ']';
}

/// \brief Prints a box to a Qt debug stream.
/// \relates Box_3
template<typename T>
inline QDebug operator<<(QDebug dbg, const Box_3<T>& b) {
    dbg.nospace() << "[" << b.minc << "] - [" << b.maxc << "]";
    return dbg.space();
}

/// \brief Writes a box to a binary output stream.
/// \relates Box_3
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Box_3<T>& b)
{
	return stream << b.minc << b.maxc;
}

/// \brief Reads a box from a binary input stream.
/// \relates Box_3
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Box_3<T>& b)
{
	return stream >> b.minc >> b.maxc;
}

/// \brief Writes a box to a Qt data stream.
/// \relates Box_3
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Box_3<T>& b) {
	return stream << b.minc << b.maxc;
}

/// \brief Reads a box from a Qt data stream.
/// \relates Box_3
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Box_3<T>& b) {
	return stream >> b.minc >> b.maxc;
}


/** 
 * \brief Template class instance of the Box_3 class used for floating-point calculations based on Point3. 
 * \relates Box_3
 */
typedef Box_3<FloatType> Box3;

/** 
 * \brief Template class instance of the Box_3 class used for integer calculations based on Point3I. 
 * \relates Box_3
 */
typedef Box_3<int> Box3I;

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Box3);
Q_DECLARE_METATYPE(Ovito::Box3I);
Q_DECLARE_METATYPE(Ovito::Box3*);
Q_DECLARE_METATYPE(Ovito::Box3I*);
Q_DECLARE_TYPEINFO(Ovito::Box3, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box3I, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box3*, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Box3I*, Q_MOVABLE_TYPE);

#endif // __OVITO_BOX3_H
