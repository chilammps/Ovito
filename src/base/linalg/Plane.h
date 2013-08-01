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
 * \file Plane.h
 * \brief Contains definition of the Ovito::Plane_3 template class.
 */

#ifndef __OVITO_PLANE_H
#define __OVITO_PLANE_H

#include <base/Base.h>
#include "Vector3.h"
#include "Point3.h"
#include "Ray.h"
#include "AffineTransformation.h"

namespace Ovito {

/**
 * \brief A plane in three-dimensional space.
 *
 * The plane is defined by a normal vector and a distance value that specifies
 * the distance of the plane from the origin in the direction of the normal vector.
 *
 * \note This is a template class for general data types. Usually one wants to
 *       use this template class with the floating-point data type. There is
 *       a template instance called \c Plane3 that should normally be used.
 *
 * \author Alexander Stukowski
 */
template<typename T>
class Plane_3
{
public:
	/// \brief The unit normal vector.
	Vector_3<T> normal;
	/// \brief The distance of the plane from the origin.
	T dist;

	/////////////////////////////// Constructors /////////////////////////////////

	/// \brief This empty default constructor does not initialize the components!
	/// \note All components of the plane are left uninitialized by this constructor and
	///       will therefore have a random value!
	Plane_3() {}

	/// \brief Initializes the plane from a normal vector and a distance.
	/// \param n The normal vector. This must be a unit vector.
	/// \param d The distance of the plane from the origin in the direction of the normal vector \a n.
	Q_DECL_CONSTEXPR Plane_3(const Vector_3<T>& n, T d) : normal(n), dist(d) {}

	/// \brief Initializes the plane from a point and a normal vector.
	/// \param basePoint A point in the plane.
	/// \param n The normal vector. This must be a unit vector.
	Q_DECL_CONSTEXPR Plane_3(const Point_3<T>& basePoint, const Vector_3<T>& n) : normal(n), dist(normal.dot((basePoint - typename Point_3<T>::Origin()))) {}

	/// \brief Initializes the plane from three points (without normalization).
	/// \param p1 The first point in the plane.
	/// \param p2 The second point in the plane.
	/// \param p3 The third point in the plane.
	/// \note The three points must linearly independent of each other.
	/// \note The normal vector computed from the three points is normalized.
	Plane_3(const Point_3<T>& p1, const Point_3<T>& p2, const Point_3<T>& p3) {
		normal = (p2-p1).cross(p3-p1);
		T lsq = normal.squaredLength();
		if(lsq) dist = normal.dot(p1 - typename Point_3<T>::Origin()) / lsq;
		else dist = 0;
	}

	/// \brief Initializes the plane from three points with optional normalization.
	/// \param p1 The first point in the plane.
	/// \param p2 The second point in the plane.
	/// \param p3 The third point in the plane.
	/// \param normalize Controls the normalization of the calculated normal vector.
	///        If \a normalize is set to \c false then the normal vector can be normalized by
	///        a call to normalizePlane() at any later time.
	/// \note The three points must linearly independent of each other.
	Plane_3(const Point_3<T>& p1, const Point_3<T>& p2, const Point_3<T>& p3, bool normalize) {
		if(normalize) {
			normal = (p2-p1).cross(p3-p1).normalized();
			dist = normal.dot(p1 - typename Point_3<T>::Origin());
		}
		else {
			normal = (p2-p1).cross(p3-p1);
			T lsq = normal.squaredLength();
			if(lsq) dist = normal.dot(p1 - typename Point_3<T>::Origin()) / lsq;
			else dist = 0;
		}
	}

	/// \brief Initializes the plane from one point and two in-plane vectors with optional normalization.
	/// \param p The base point in the plane.
	/// \param v1 The first vector in the plane.
	/// \param v2 The second vector in the plane.
	/// \param normalize Controls the normalization of the calculated normal vector.
	///        If \a normalize is set to \c false then the normal vector can be normalized by
	///        a call to normalizePlane() at any later time.
	/// \note The two vectors must be linearly independent of each other.
	Plane_3(const Point_3<T>& p, const Vector_3<T>& v1, const Vector_3<T>& v2, bool normalize = true) {
		if(normalize)
			normal = v1.cross(v2).normalized();
		else
			normal = v1.cross(v2);
		dist = normal.dot(p - typename Point_3<T>::Origin());
	}

	/// \brief Scales the normal vector of the plane to unit length 1.
	void normalizePlane() {
		T len = normal.length();
		OVITO_ASSERT_MSG(len != T(0), "Plane_3::normalizePlane()", "The normal vector of the plane must not be the null vector.");
		dist *= len;
		normal /= len;
		OVITO_ASSERT(std::abs(normal.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON));
	}

    ////////////////////////////////// operators /////////////////////////////////

	/// \brief Flips the plane orientation.
	/// \return A new plane with reversed orientation.
	Q_DECL_CONSTEXPR Plane_3<T> operator-() const { return Plane_3<T>(-normal, -dist); }

	/// \brief Compares two planes for equality.
	/// \return \c true if the normal vectors and the distance value of both planes a equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Plane_3<T>& other) const { return normal == other.normal && dist == other.dist; }

	/////////////////////////////// Classification ///////////////////////////////

	/// \brief Classifies a point with respect to the plane.
	/// \param p The point to classify.
	/// \param tolerance A non-negative threshold value that is used to test whether the point is on the plane.
	/// \return 1 if \a p is on the POSITIVE side of the plane,
	///         -1 if \a p is on the NEGATIVE side or 0 if \a p is ON the plane within the tolerance.
	/// \sa pointDistance()
	int classifyPoint(const Point_3<T>& p, const T tolerance = T(FLOATTYPE_EPSILON)) const {
		OVITO_ASSERT_MSG(tolerance >= 0, "Plane_3::classifyPoint()", "Tolerance value must be non-negative.");
        T d = pointDistance(p);
		if(d < -tolerance) return -1;
		else if(d > tolerance) return 1;
		else return 0;
	}

	/// \brief Computes the signed distance of a point to the plane.
	/// \param p The input point.
	/// \note This method requires the plane normal to be a unit vector.
	/// \return The distance of the point to the plane. A positive value means that the point
	/// is on the positive side of the plane. A negative value means that the points is located in the
	/// back of the plane.
	/// \sa classifyPoint()
	Q_DECL_CONSTEXPR T pointDistance(const Point_3<T>& p) const {
		return (normal.x() * p.x() + normal.y() * p.y() + normal.z() * p.z()) - dist;
	}

	///////////////////////////////// Intersection ///////////////////////////////

	/// \brief Computes the intersection point of a ray with the plane.
	/// \param ray The input ray.
	/// \param epsilon A threshold value that determines whether the ray is considered parallel to the plane.
	/// \return The intersection point.
	/// \throw Exception if the plane and the ray direction are parallel.
	/// \note This method requires a unit plane normal vector.
	/// \sa intersectionT()
	Point_3<T> intersection(const Ray3& ray, T epsilon = T(0)) const {
		T t = intersectionT(ray, epsilon);
		if(t == std::numeric_limits<T>::max()) throw Exception("Error in Plane_3::intersection(): There is no intersection point. Ray is parallel to plane.");
		return ray.point(t);
	}

	/// \brief Computes the t value for a ray-plane intersection.
	///
	/// The returned t value is chosen so that (ray.base + t*ray.dir) == point of intersection.
	/// If there is no intersection point then the special value \c FLOATTYPE_MAX is returned.
	/// \param ray The input ray.
	/// \param epsilon A threshold value that determines whether the ray is considered parallel to the plane.
	/// \note This implementation requires a unit normal vector.
	/// \sa intersection()
	T intersectionT(const Ray3& ray, T epsilon = T(0)) const {
		// The plane's normal vector should be normalized.
		OVITO_ASSERT(std::abs(normal.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON));
		T dot = normal.dot(ray.dir);
		if(std::abs(dot) <= epsilon) return std::numeric_limits<T>::max();
		return -pointDistance(ray.base) / dot;
	}

    ////////////////////////////////// Projection /////////////////////////////////

	/// \brief Projects a point onto the plane.
	/// \param p The point to be projected.
	/// \return The projected point. This is the point on the plane that is
	///         is closest to the input point.
	Q_DECL_CONSTEXPR Point_3<T> projectPoint(const Point_3<T>& p) const {
		return p - pointDistance(p) * normal;
	}

    ////////////////////////////////// Utilities /////////////////////////////////

	/// \brief Returns a string representation of this plane.
	QString toString() const {
		return "[Normal: " + normal.toString() + " D: " + QString::number(dist) + "]";
	}
};

/// \brief Transforms a plane.
/// \param tm The transformation matrix.
/// \param ray The plane to be transformed.
/// \return A new plane with transformed normal vector and origin distance.
///         The normal vector is normalized after the transformation.
template<typename T>
inline Plane_3<T> operator*(const Matrix_34<T>& tm, const Plane_3<T>& plane) {
	Plane_3<T> p2;
	p2.normal = (tm * plane.normal).normalized();
	Point_3<T> base = tm * (typename Point_3<T>::Origin() + plane.normal * plane.dist);
	p2.dist = p2.normal.dot(base - typename Point_3<T>::Origin());
	return p2;
}

/// \brief Writes the plane to an output stream.
/// \param os The output stream.
/// \param p The plane to write to the output stream \a os.
/// \return The output stream \a os.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Plane_3<T>& p) {
	return os << '[' << p.normal.x() << ' ' << p.normal.y()  << ' ' << p.normal.z() << "], " << p.dist;
}

/// \brief Writes the plane to a binary output stream.
/// \param stream The output stream.
/// \param p The plane to write to the output stream \a stream.
/// \return The output stream \a stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Plane_3<T>& p)
{
	return stream << p.normal << p.dist;
}

/// \brief Loads a plane from a binary output stream.
/// \param stream The input stream.
/// \param p The plane variable.
/// \return The input stream \a stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Plane_3<T>& p)
{
	return stream >> p.normal >> p.dist;
}

/**
 * \fn typedef Plane3
 * \brief Template class instance of the Plane_3 class used for floating-point planes.
 */
typedef Plane_3<FloatType> Plane3;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Plane3);
Q_DECLARE_TYPEINFO(Ovito::Plane3, Q_PRIMITIVE_TYPE);

#endif // __OVITO_PLANE_H
