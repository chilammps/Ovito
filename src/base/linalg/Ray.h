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
 * \file Ray.h 
 * \brief Contains definition of the Ovito::Ray3 template class.
 */

#ifndef __OVITO_RAY_H
#define __OVITO_RAY_H

#include <base/Base.h>
#include "Vector3.h"
#include "Point3.h"
#include "AffineTransformation.h"

namespace Ovito {

/**
 * \brief Describes an infinite ray in 3d space.
 * 
 * A ray is defined by a base point and a direction vector.
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

	/// \brief This empty default constructor does not initialize the components!
	/// \note All components of the ray are left uninitialized by this constructor and 
	///       will therefore have a random value!
	Ray_3() {}

	/// \brief Initializes the ray with a base point and a direction vector.
	/// \param b A point through which the ray passes.
	/// \param d A direction vector. This vector should have length 1.
	Q_DECL_CONSTEXPR Ray_3(const Point_3<T>& b, const Vector_3<T>& d) : base(b), dir(d) {}

	/// \brief Initializes the ray from two points.
	/// \param a The first point on the ray
	/// \param b The second point on the ray.
	///
	/// The direction vector of the ray is initialized to the vector pointing from
	/// point \a a to point \a b.
	Q_DECL_CONSTEXPR Ray_3(const Point_3<T>& a, const Point_3<T>& b) : base(a), dir(b - a) {}
	
	////////////////////////////////// Queries ///////////////////////////////////
	
	/// \brief Returns a point on the ray at the given parameter position.
	/// \param t The parameter that specifies the position on the ray
	///        starting at the base point and going into the direction specified
	///        by the ray vector.
	/// \return The point base + dir * t.
	Q_DECL_CONSTEXPR Point_3<T> point(T t) const { return base + dir * t; }

    /////////////////////////////// Unary operators //////////////////////////////

	/// \brief Flips the ray direction.
	/// \return A new ray with reversed direction vector.
	Q_DECL_CONSTEXPR Ray_3 operator-() const { return Ray_3(base, -dir); }

    ////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Returns a string representation of this ray.
	QString toString() const {
		return QString("[Base: %1 Dir: %2]").arg(base.toString(), dir.toString());
	}
};

/// \brief Transforms a ray.
/// \param tm The transformation matrix.
/// \param ray The ray to be transformed.
/// \return A new ray with transformed base point and direction vector.
///         The direction is normalized after the transformation.
template<typename T>
inline Ray_3<T> operator*(const Matrix_34<T>& tm, const Ray_3<T>& ray) {
	return { tm * ray.base, (tm * ray.dir).normalized() };
}

/// \brief Writes the ray to an output stream.
/// \param os The output stream.
/// \param p The ray to write to the output stream \a os.
/// \return The output stream \a os.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Ray_3<T> &r) {
	return os << '[' << r.base.x() << ' ' << r.base.y()  << ' ' << r.base.z() << "], (" << r.dir.x() << ' ' << r.dir.y()  << ' ' << r.dir.z() << ')';
}

/// \brief Writes the ray to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Ray_3<T>& r) {
    dbg.nospace() << "[" << r.base.x() << " " << r.base.y() << " " << r.base.z() << "], (" << r.dir.x() << " " << r.dir.y()  << " " << r.dir.z() << ')';
    return dbg.space();
}

/// \brief Writes the ray to a binary output stream.
/// \param stream The output stream.
/// \param p The ray to write to the output stream \a stream.
/// \return The output stream \a stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Ray_3<T>& r)
{
	return stream << r.base << r.dir;
}

/// \brief Reads a ray from a binary input stream.
/// \param stream The input stream.
/// \param p Reference to a ray variable where the parsed data will be stored.
/// \return The input stream \a stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Ray_3<T>& r)
{
	return stream >> r.base >> r.dir;
}

/// \brief Writes a ray to a Qt data stream.
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Ray_3<T>& r) {
	return stream << r.base << r.dir;
}

/// \brief Reads a ray from a Qt data stream.
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Ray_3<T>& r) {
	return stream >> r.base >> r.dir;
}

/**
 * \fn typedef Ray3
 * \brief Template class instance of the Ray_3 class.
 */
typedef Ray_3<FloatType> Ray3;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Ray3);
Q_DECLARE_METATYPE(Ovito::Ray3*);
Q_DECLARE_TYPEINFO(Ovito::Ray3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Ray3*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_RAY_H
