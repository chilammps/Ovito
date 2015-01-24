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
 * \brief Contains the definition of the Ovito::Ray_3 class template.
 */

#ifndef __OVITO_RAY_H
#define __OVITO_RAY_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"
#include "Point3.h"
#include "AffineTransformation.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief An infinite ray in 3d space, defined by a base point and a direction vector.
 * 
 * This class stores an infinite ray in three-dimensional space, which is described by
 * a base point #base and a direction vector #dir. The direction vector does not have to be
 * a unit vector.
 *
 * The template parameter \a T specifies the data type used for spatial coordinates.
 * The standard template instantiation for floating-point coordinates is predefined:
 *
 * \code
 *      typedef Ray_3<FloatType>  Ray3;
 * \endcode
 */
template<typename T>
class Ray_3
{
public:

	/// A base point on the ray.
	Point_3<T> base;
	
	/// The direction vector.
	Vector_3<T> dir;

	/////////////////////////////// Constructors /////////////////////////////////

	/// Empty default constructor that does not initialize the fields of the object for performance reasons!
	/// Both the base point and the direction vector are undefined.
	Ray_3() {}

	/// \brief Initializes the ray with a base point and a direction vector.
	/// \param b A point through which the ray passes.
	/// \param d The ray's direction vector.
	Q_DECL_CONSTEXPR Ray_3(const Point_3<T>& b, const Vector_3<T>& d) : base(b), dir(d) {}

	/// \brief Initializes the ray from two points.
	/// \param a The first point on the ray
	/// \param b The second point on the ray.
	///
	/// The direction of the ray is initialized to the vector connecting \a a and \a b.
	Q_DECL_CONSTEXPR Ray_3(const Point_3<T>& a, const Point_3<T>& b) : base(a), dir(b - a) {}
	
	////////////////////////////////// Queries ///////////////////////////////////
	
	/// \brief Returns a point on the ray at the specified position.
	/// \param t Specifies the position along the ray.
	/// \return The point (#base + #dir * \a t).
	Q_DECL_CONSTEXPR Point_3<T> point(T t) const { return base + dir * t; }

    /////////////////////////////// Unary operators //////////////////////////////

	/// \brief Flips the ray's direction.
	/// \return A new ray with a reversed direction vector.
	Q_DECL_CONSTEXPR Ray_3 operator-() const { return Ray_3(base, -dir); }

    ////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Generates a string representation of this ray.
	QString toString() const {
		return QString("[Base: %1 Dir: %2]").arg(base.toString(), dir.toString());
	}
};

/// \brief Transforms a ray.
/// \param tm The affine transformation matrix.
/// \param ray The ray to be transformed.
/// \return A new ray with transformed base point and direction vector.
///         The direction vector is automatically normalized after the transformation.
/// \relates Ray_3
template<typename T>
inline Ray_3<T> operator*(const AffineTransformationT<T>& tm, const Ray_3<T>& ray) {
	return { tm * ray.base, (tm * ray.dir).normalized() };
}

/// \brief Prints a ray to an output stream.
/// \param os The output stream.
/// \param p The ray to write to the output stream \a os.
/// \return The output stream \a os.
/// \relates Ray_3
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Ray_3<T> &r) {
	return os << '[' << r.base.x() << ' ' << r.base.y()  << ' ' << r.base.z() << "], (" << r.dir.x() << ' ' << r.dir.y()  << ' ' << r.dir.z() << ')';
}

/// \brief Prints a ray to a Qt debug stream.
/// \relates Ray_3
template<typename T>
inline QDebug operator<<(QDebug dbg, const Ray_3<T>& r) {
    dbg.nospace() << "[" << r.base.x() << " " << r.base.y() << " " << r.base.z() << "], (" << r.dir.x() << " " << r.dir.y()  << " " << r.dir.z() << ')';
    return dbg.space();
}

/// \brief Writes a ray to a binary output stream.
/// \param stream The output stream.
/// \param p The ray to write to the output stream \a stream.
/// \return The output stream \a stream.
/// \relates Ray_3
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Ray_3<T>& r) {
	return stream << r.base << r.dir;
}

/// \brief Reads a a from a binary input stream.
/// \param stream The input stream.
/// \param p Reference to a ray variable where the parsed data will be stored.
/// \return The input stream \a stream.
/// \relates Ray_3
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Ray_3<T>& r) {
	return stream >> r.base >> r.dir;
}

/// \brief Writes a ray to a Qt data stream.
/// \relates Ray_3
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Ray_3<T>& r) {
	return stream << r.base << r.dir;
}

/// \brief Reads a ray from a Qt data stream.
/// \relates Ray_3
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Ray_3<T>& r) {
	return stream >> r.base >> r.dir;
}

/**
 * \brief Instantiation of the Ray_3 class template with the default floating-point type.
 * \relates Ray_3
 */
typedef Ray_3<FloatType> Ray3;

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Ray3);
Q_DECLARE_METATYPE(Ovito::Ray3*);
Q_DECLARE_TYPEINFO(Ovito::Ray3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Ray3*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_RAY_H
