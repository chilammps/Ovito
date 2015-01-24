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
 * \brief Contains the definition of the Ovito::AffineTransformationT class template.
 */

#ifndef __OVITO_AFFINE_TRANSFORMATION_H
#define __OVITO_AFFINE_TRANSFORMATION_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"
#include "Vector4.h"
#include "Point3.h"
#include "Matrix3.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A 3x4 matrix, which describes an affine transformation in 3d space.
 *
 * The matrix is stored in column-major order. AffineTransformationT is derived from std::array<Vector_3<T>, 4>.
 * Thus, it is an array of four column vectors with three elements each.
 *
 * The template parameter \a T specifies the data type of the matrix elements.
 * The typedef \c AffineTransformation for matrices with floating-point elements is predefined:
 *
 * \code
 *      typedef AffineTransformationT<FloatType> AffineTransformation;
 * \endcode
 *
 * The first three columns of the 3x4 matrix store the linear part of the affine transformation.
 * The fourth column stores the translation vector.
 *
 * \sa Vector_3, Point_3
 * \sa Matrix_3, Matrix_4
 */
template<typename T>
class AffineTransformationT : public std::array<Vector_3<T>,4>
{
public:

	/// An empty type that denotes a 3x4 matrix with all elements equal to zero.
	struct Zero {};

	/// An empty type that denotes the identity transformation.
	struct Identity {};

	/// The type of a single element of the matrix.
	typedef T element_type;

	/// The type of a single column of the matrix.
	typedef Vector_3<T> column_type;

	using typename std::array<Vector_3<T>, 4>::size_type;
	using typename std::array<Vector_3<T>, 4>::difference_type;
	using typename std::array<Vector_3<T>, 4>::value_type;
	using typename std::array<Vector_3<T>, 4>::iterator;
	using typename std::array<Vector_3<T>, 4>::const_iterator;

public:

	/// \brief Empty default constructor that does not initialize the matrix elements (for performance reasons).
	///        The matrix elements will have an undefined value and need to be initialized later.
	AffineTransformationT() {}

	/// \brief Constructor that initializes 9 elements of the left 3x3 submatrix to the given values.
	///        The translation (4th column) is set to zero.
	/// \note Matrix elements are specified in row-major order, i.e. row by row.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR AffineTransformationT(
						T m11, T m12, T m13,
					    T m21, T m22, T m23,
					    T m31, T m32, T m33)
		: std::array<Vector_3<T>,4>{{
			Vector_3<T>(m11,m21,m31),
			Vector_3<T>(m12,m22,m32),
			Vector_3<T>(m13,m23,m33),
			typename Vector_3<T>::Zero()}} {}
#else
	AffineTransformationT(
		T m11, T m12, T m13,
		T m21, T m22, T m23,
		T m31, T m32, T m33)
		{ (*this)[0] = Vector_3<T>(m11,m21,m31); 
		  (*this)[1] = Vector_3<T>(m12,m22,m32);
		  (*this)[2] = Vector_3<T>(m13,m23,m33);
		  (*this)[3] = typename Vector_3<T>::Zero(); }
#endif

	/// \brief Constructor that initializes the elements of the matrix to the given values.
	/// \note Elements are specified in row-major order, i.e. row by row.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR AffineTransformationT(
						T m11, T m12, T m13, T m14,
					    T m21, T m22, T m23, T m24,
					    T m31, T m32, T m33, T m34)
		: std::array<Vector_3<T>,4>{{
			Vector_3<T>(m11,m21,m31),
			Vector_3<T>(m12,m22,m32),
			Vector_3<T>(m13,m23,m33),
			Vector_3<T>(m14,m24,m34)}} {}
#else
	AffineTransformationT(
					T m11, T m12, T m13, T m14,
					T m21, T m22, T m23, T m24,
					T m31, T m32, T m33, T m34)
		{ (*this)[0] = Vector_3<T>(m11,m21,m31); 
		  (*this)[1] = Vector_3<T>(m12,m22,m32);
		  (*this)[2] = Vector_3<T>(m13,m23,m33);
		  (*this)[3] = Vector_3<T>(m14,m24,m34); }
#endif

	/// \brief Constructor that initializes the matrix from four column vectors.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR AffineTransformationT(const column_type& c1, const column_type& c2, const column_type& c3, const column_type& c4)
		: std::array<Vector_3<T>,4>{{c1, c2, c3, c4}} {}
#else
	AffineTransformationT(const column_type& c1, const column_type& c2, const column_type& c3, const column_type& c4)
		{ (*this)[0] = c1; (*this)[1] = c2; (*this)[2] = c3; (*this)[3] = c4; } 
#endif

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR AffineTransformationT(Zero)
		: std::array<Vector_3<T>,4>{{
			typename Vector_3<T>::Zero(),
			typename Vector_3<T>::Zero(),
			typename Vector_3<T>::Zero(),
			typename Vector_3<T>::Zero()}} {}
#else
	AffineTransformationT(Zero)
		{ this->fill(typename Vector_3<T>::Zero()); }
#endif

	/// \brief Initializes the matrix to the identity matrix.
	/// All diagonal elements are set to one and all off-diagonal elements are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR AffineTransformationT(Identity)
		: std::array<Vector_3<T>,4>{{
			Vector_3<T>(T(1),T(0),T(0)),
			Vector_3<T>(T(0),T(1),T(0)),
			Vector_3<T>(T(0),T(0),T(1)),
			Vector_3<T>(T(0),T(0),T(0))}} {}
#else
	AffineTransformationT(Identity)
		{ (*this)[0] = Vector_3<T>(T(1),T(0),T(0)); 
		  (*this)[1] = Vector_3<T>(T(0),T(1),T(0));
		  (*this)[2] = Vector_3<T>(T(0),T(0),T(1));
		  (*this)[3] = Vector_3<T>(T(0),T(0),T(0)); }
#endif

	/// \brief Initializes the 3x4 matrix from a 3x3 matrix.
	/// The translation vector (4th column) is set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	explicit Q_DECL_CONSTEXPR AffineTransformationT(const Matrix_3<T>& tm)
		: std::array<Vector_3<T>,4>{{tm.column(0), tm.column(1), tm.column(2), typename Vector_3<T>::Zero()}} {}
#else
	explicit AffineTransformationT(const Matrix_3<T>& tm)
		{ (*this)[0] = tm.column(0); (*this)[1] = tm.column(1); (*this)[2] = tm.column(2); (*this)[3] = typename Vector_3<T>::Zero(); } 
#endif

	/// \brief Returns the number of rows of this matrix.
	static Q_DECL_CONSTEXPR size_type row_count() { return 3; }

	/// \brief Returns the number of columns of this matrix.
	static Q_DECL_CONSTEXPR size_type col_count() { return 4; }

	/// \brief Returns a matrix element.
	/// \param row The row of the element.
	/// \param col The column of the element.
	/// \return The value of the matrix element.
	inline Q_DECL_CONSTEXPR T operator()(size_type row, size_type col) const {
		return (*this)[col][row];
	}

	/// \brief Returns a modifiable reference to a matrix element.
	/// \param row The row of the element.
	/// \param col The column of the element.
	inline T& operator()(size_type row, size_type col) {
		return (*this)[col][row];
	}

	/// \brief Returns a column vector of the matrix.
	/// \param col The index of the column.
	/// \return A vector of three elements.
	inline Q_DECL_CONSTEXPR const column_type& column(size_type col) const {
		return (*this)[col];
	}

	/// \brief Returns a modifiable reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return A reference to a vector of three elements.
	inline column_type& column(size_type col) {
		return (*this)[col];
	}
	
	/// \brief Returns a row of the matrix.
	/// \param row The row to return.
	/// \return A vector of four elements.
	Q_DECL_CONSTEXPR Vector_4<T> row(size_type row) const {
		return { (*this)[0][row], (*this)[1][row], (*this)[2][row], (*this)[3][row] };
	}

	/// \brief Returns the translational part of the transformation, which is stored in the fourth column.
	Q_DECL_CONSTEXPR const column_type& translation() const { return column(3); }

	/// \brief Returns a modifiable reference to the translational part of the transformation, which is stored in the fourth column.
	column_type& translation() { return column(3); }

	/// \brief Sets all elements of the matrix to zero.
	void setZero() {
		for(size_type i = 0; i < col_count(); i++)
			(*this)[i].setZero();
	}

	/// \brief Sets all elements of the matrix to zero.
	AffineTransformationT& operator=(Zero) {
		setZero();
		return *this;
	}

	/// \brief Sets the matrix to the identity matrix.
	void setIdentity() {
		(*this)[0] = Vector_3<T>(T(1),T(0),T(0));
		(*this)[1] = Vector_3<T>(T(0),T(1),T(0));
		(*this)[2] = Vector_3<T>(T(0),T(0),T(1));
		(*this)[3].setZero();
	}

	/// \brief Sets the matrix to the identity matrix.
	AffineTransformationT& operator=(Identity) {
		setIdentity();
		return *this;
	}

	/// Returns a pointer to the 12 elements of the matrix (stored in column-major order).
	const element_type* elements() const {
		OVITO_STATIC_ASSERT(sizeof(*this) == sizeof(element_type)*12);
		return column(0).data();
	}

	/// Returns a pointer to the 12 elements of the matrix (stored in column-major order).
	element_type* elements() {
		OVITO_STATIC_ASSERT(sizeof(*this) == sizeof(element_type)*12);
		return column(0).data();
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Compares two matrices for exact equality.
	/// \return true if all elements are equal; false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const AffineTransformationT& b) const {
		return (b[0] == (*this)[0]) && (b[1] == (*this)[1]) && (b[2] == (*this)[2]) && (b[3] == (*this)[3]);
	}

	/// \brief Compares two matrices for inequality.
	/// \return true if not all elements are equal; false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const AffineTransformationT& b) const {
		return !(*this == b);
	}

	////////////////////////////////// Computations ///////////////////////////////////

	/// \brief Computes the determinant of the matrix.
	Q_DECL_CONSTEXPR inline T determinant() const {
		return(((*this)[0][0]*(*this)[1][1] - (*this)[0][1]*(*this)[1][0])*((*this)[2][2])
			  -((*this)[0][0]*(*this)[1][2] - (*this)[0][2]*(*this)[1][0])*((*this)[2][1])
			  +((*this)[0][1]*(*this)[1][2] - (*this)[0][2]*(*this)[1][1])*((*this)[2][0]));
	}

	/// \brief Computes the inverse of the matrix. 
	/// \throw Exception if the matrix is not invertible because it is singular.
	AffineTransformationT inverse() const {
		// Compute inverse of 3x3 sub-matrix.
		// Then multiply with inverse translation.
		T det = determinant();
		OVITO_ASSERT_MSG(det != T(0), "AffineTransformation::inverse()", "Singular matrix cannot be inverted: Determinant is zero.");
		if(det == T(0)) throw Exception("Affine transformation cannot be inverted: determinant is zero.");

		AffineTransformationT inv(
						((*this)[1][1]*(*this)[2][2] - (*this)[1][2]*(*this)[2][1])/det,
						((*this)[2][0]*(*this)[1][2] - (*this)[1][0]*(*this)[2][2])/det,
						((*this)[1][0]*(*this)[2][1] - (*this)[1][1]*(*this)[2][0])/det,
						T(0),
						((*this)[2][1]*(*this)[0][2] - (*this)[0][1]*(*this)[2][2])/det,
						((*this)[0][0]*(*this)[2][2] - (*this)[2][0]*(*this)[0][2])/det,
						((*this)[0][1]*(*this)[2][0] - (*this)[0][0]*(*this)[2][1])/det,
						T(0),
						((*this)[0][1]*(*this)[1][2] - (*this)[1][1]*(*this)[0][2])/det,
						((*this)[0][2]*(*this)[1][0] - (*this)[0][0]*(*this)[1][2])/det,
						((*this)[0][0]*(*this)[1][1] - (*this)[1][0]*(*this)[0][1])/det,
						T(0));
		inv.translation() = inv * (-translation());
		return inv;
	}

	/// \brief Computes the inverse of the matrix.
	/// \param result A reference to an output matrix that will receive the computed inverse.
	/// \param epsilon A threshold that is used to determine if the matrix is invertible. The matrix is considered singular if |det|<=epsilon.
	/// \return \c false if the matrix is not invertible because it is singular; \c true if the inverse has been calculated
	///         and was stored in \a result.
	/// \sa determinant()
	bool inverse(AffineTransformationT& result, T epsilon = T(FLOATTYPE_EPSILON)) const {
		T det = determinant();
		if(std::abs(det) <= epsilon) return false;
		result = AffineTransformationT(
						((*this)[1][1]*(*this)[2][2] - (*this)[1][2]*(*this)[2][1])/det,
						((*this)[2][0]*(*this)[1][2] - (*this)[1][0]*(*this)[2][2])/det,
						((*this)[1][0]*(*this)[2][1] - (*this)[1][1]*(*this)[2][0])/det,
						T(0),
						((*this)[2][1]*(*this)[0][2] - (*this)[0][1]*(*this)[2][2])/det,
						((*this)[0][0]*(*this)[2][2] - (*this)[2][0]*(*this)[0][2])/det,
						((*this)[0][1]*(*this)[2][0] - (*this)[0][0]*(*this)[2][1])/det,
						T(0),
						((*this)[0][1]*(*this)[1][2] - (*this)[1][1]*(*this)[0][2])/det,
						((*this)[0][2]*(*this)[1][0] - (*this)[0][0]*(*this)[1][2])/det,
						((*this)[0][0]*(*this)[1][1] - (*this)[1][0]*(*this)[0][1])/det,
						T(0));
		result.translation() = result * (-translation());
		return true;
	}

	/// Orthonormalizes the matrix.
	///
    /// Algorithm uses Gram-Schmidt orthogonalization.  If this matrix is
    /// M = [m0|m1|m2], then the orthonormal output matrix is Q = [q0|q1|q2], with
    ///
    ///     q0 = m0/|m0|
    ///     q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
    ///     q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
    ///
    /// where |V| denotes length of vector V and A*B denotes dot
    /// product of vectors A and B.
	void orthonormalize() {

		// Compute q0.
		(*this)[0].normalize();

	    // Compute q1.
		T dot0 = (*this)[0].dot((*this)[1]);
		(*this)[1][0] -= dot0 * (*this)[0][0];
		(*this)[1][1] -= dot0 * (*this)[0][1];
		(*this)[1][2] -= dot0 * (*this)[0][2];
		(*this)[1].normalize();

	    // compute q2
	    dot0 = (*this)[0].dot((*this)[2]);
	    T dot1 = (*this)[1].dot((*this)[2]);
	    (*this)[2][0] -= dot0*(*this)[0][0] + dot1*(*this)[1][0];
	    (*this)[2][1] -= dot0*(*this)[0][1] + dot1*(*this)[1][1];
	    (*this)[2][2] -= dot0*(*this)[0][2] + dot1*(*this)[1][2];
	    (*this)[2].normalize();
	}

	/// Computes the product of the matrix and a point and returns one coordinate of the transformed point.
	/// \param p The point to transform with the matrix. It is implicitly extended to a 4-vector with the last element being 1.
	/// \param index The component (0-2) of the transformed point to return.
	/// \return ((*this)*p)[index]
	inline Q_DECL_CONSTEXPR T prodrow(const Point_3<T>& p, typename Point_3<T>::size_type index) const {
		return (*this)[0][index] * p[0] + (*this)[1][index] * p[1] + (*this)[2][index] * p[2] + (*this)[3][index];
	}

	/// Computes the product of the matrix and a vector and returns one component of the resulting vector.
	/// \param v The vector to transform with the matrix. It is implicitly extended to a 4-vector with the last element being 0.
	/// \param index The component (0-2) of the transformed vector to return.
	/// \return ((*this)*v)[index]
	inline Q_DECL_CONSTEXPR T prodrow(const Vector_3<T>& v, typename Vector_3<T>::size_type index) const {
		return (*this)[0][index] * v[0] + (*this)[1][index] * v[1] + (*this)[2][index] * v[2];
	}

	/// Returns the upper left 3x3 submatrix of this 3x4 matrix containing only the linear transformation but not the translation.
	inline Q_DECL_CONSTEXPR Matrix_3<T> linear() const {
		return Matrix_3<T>((*this)[0], (*this)[1], (*this)[2]);
	}

	////////////////////////////////// Generation ///////////////////////////////////
	
	/// \brief Generates a matrix describing a rotation around the X axis.
	/// \param angle The rotation angle in radians.
	static inline AffineTransformationT rotationX(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return AffineTransformationT{T(1), T(0), T(0), T(0),
									 T(0),    c,   -s, T(0),
									 T(0),    s,    c, T(0)};
	}

	/// \brief Generates a matrix describing a rotation around the Y axis.
	/// \param angle The rotation angle in radians.
	static inline AffineTransformationT rotationY(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return AffineTransformationT{   c, T(0),    s, T(0),
									 T(0), T(1), T(0), T(0),
									   -s, T(0),    c, T(0)};
	}

	/// \brief Generates a matrix describing a rotation around the Z axis.
	/// \param angle The rotation angle in radians.
	static inline AffineTransformationT rotationZ(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return AffineTransformationT{   c,   -s, T(0), T(0),
										s,    c, T(0), T(0),
									 T(0), T(0), T(1), T(0)};
	}

	/// Generates a pure rotation matrix from an axis-angle representation.
	static AffineTransformationT rotation(const RotationT<T>& rot);

	/// Generates a pure rotation matrix from a quaternion.
	static AffineTransformationT rotation(const QuaternionT<T>& q);

	/// Generates a pure translation matrix.
	static Q_DECL_CONSTEXPR AffineTransformationT translation(const Vector_3<T>& t) {
		return AffineTransformationT(T(1), T(0), T(0), t.x(),
						 	 	 	 T(0), T(1), T(0), t.y(),
						 	 	 	 T(0), T(0), T(1), t.z());
	}

	/// Generates a diagonal scaling matrix.
	/// \param s The value of the three diagonal elements.
	static Q_DECL_CONSTEXPR AffineTransformationT scaling(T s) {
		return AffineTransformationT(
						 s, T(0), T(0), T(0),
						 T(0),    s, T(0), T(0),
						 T(0), T(0),    s, T(0));
	}

	/// Generates a pure scaling matrix.
	static AffineTransformationT scaling(const ScalingT<T>& scaling);

	/// Generates pure shear matrix (shear in the x-y plane).
	static Q_DECL_CONSTEXPR AffineTransformationT shear(T gammaX, T gammaY) {
		return AffineTransformationT(
				         T(1), T(0), gammaX, T(0),
						 T(0), T(1), gammaY, T(0),
						 T(0), T(0), T(1),   T(0));
	}

	/// Generates a matrix from an OpenGL matrix.
	static AffineTransformationT fromOpenGL(const T tm[16]) {
		OVITO_ASSERT(tm[3] == 0 && tm[7] == 0 && tm[11] == 0 && tm[15] == 1);
		return AffineTransformationT{tm[0], tm[4], tm[8], tm[12],
									 tm[1], tm[5], tm[9], tm[13],
									 tm[2], tm[6], tm[10], tm[14]};
	}

	/// \brief Generates a look-at-matrix. 
	/// \param camera The position of the camera.
	/// \param target The target position where to camera should point to.
	/// \param upVector A vector specifying the up direction that determines the rotation of the camera
	///                 around the view axis.
	/// \return A transformation from world space to view space.
	static AffineTransformationT lookAt(const Point_3<T>& camera, const Point_3<T>& target, const Vector_3<T>& upVector) {
		return lookAlong(camera, target - camera, upVector);
	}

	/// \brief Generates a look-along-matrix.
	/// \param camera The position of the camera.
	/// \param direction The viewing direction.
	/// \param upVector A vector specifying the up direction that determines the rotation of the camera
	///                 around the view axis.
	/// \return A transformation from world space to view space.
	static AffineTransformationT lookAlong(const Point_3<T>& camera, const Vector_3<T>& direction, const Vector_3<T>& upVector) {
		auto zaxis = -direction.normalized();
		auto xaxis = upVector.cross(zaxis);
		if(xaxis == typename Vector_3<T>::Zero()) {
			xaxis = Vector_3<T>(0,1,0).cross(zaxis);
			if(xaxis == typename Vector_3<T>::Zero()) {
				xaxis = Vector_3<T>(0,0,1).cross(zaxis);
				OVITO_ASSERT(xaxis != typename Vector_3<T>::Zero());
			}
		}
		xaxis.normalize();
		auto yaxis = zaxis.cross(xaxis);

		return AffineTransformationT{
					xaxis.x(), xaxis.y(), xaxis.z(), -xaxis.dot(camera - typename Point_3<T>::Origin()),
					yaxis.x(), yaxis.y(), yaxis.z(), -yaxis.dot(camera - typename Point_3<T>::Origin()),
					zaxis.x(), zaxis.y(), zaxis.z(), -zaxis.dot(camera - typename Point_3<T>::Origin()) };
	}
	
	///////////////////////////////// Information ////////////////////////////////

	/// \brief Tests whether the matrix is a pure rotation matrix.
	/// \return \c true if the matrix is a pure rotation matrix; \c false otherwise.
	///
	/// The matrix A is a pure rotation matrix if:
	///   1. det(A) = 1  and
	///   2. A * A^T = I
	Q_DECL_CONSTEXPR bool isRotationMatrix(T epsilon = T(FLOATTYPE_EPSILON)) const {
		return
			translation().isZero(epsilon) &&
			(std::abs((*this)[0][0]*(*this)[1][0] + (*this)[0][1]*(*this)[1][1] + (*this)[0][2]*(*this)[1][2]) <= epsilon) &&
			(std::abs((*this)[0][0]*(*this)[2][0] + (*this)[0][1]*(*this)[2][1] + (*this)[0][2]*(*this)[2][2]) <= epsilon) &&
			(std::abs((*this)[1][0]*(*this)[2][0] + (*this)[1][1]*(*this)[2][1] + (*this)[1][2]*(*this)[2][2]) <= epsilon) &&
			(std::abs((*this)[0][0]*(*this)[0][0] + (*this)[0][1]*(*this)[0][1] + (*this)[0][2]*(*this)[0][2] - T(1)) <= epsilon) &&
			(std::abs((*this)[1][0]*(*this)[1][0] + (*this)[1][1]*(*this)[1][1] + (*this)[1][2]*(*this)[1][2] - T(1)) <= epsilon) &&
			(std::abs((*this)[2][0]*(*this)[2][0] + (*this)[2][1]*(*this)[2][1] + (*this)[2][2]*(*this)[2][2] - T(1)) <= epsilon) &&
			(std::abs(determinant() - T(1)) <= epsilon);
	}

	/// \brief Converts the matrix to a Qt 4x4 matrix.
	operator QMatrix4x4() const {
		return QMatrix4x4(
				(*this)(0,0), (*this)(0,1), (*this)(0,2), (*this)(0,3),
				(*this)(1,0), (*this)(1,1), (*this)(1,2), (*this)(1,3),
				(*this)(2,0), (*this)(2,1), (*this)(2,2), (*this)(2,3),
				0, 0, 0, 1);
	}
};

/// Computes the product of a 3x4 matrix and a Vector3 (which is automatically extended to a 4-vector with the last element being 0).
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR Vector_3<T> operator*(const AffineTransformationT<T>& m, const Vector_3<T>& v)
{
	return Vector_3<T>{ m(0,0) * v[0] + m(0,1) * v[1] + m(0,2) * v[2],
						m(1,0) * v[0] + m(1,1) * v[1] + m(1,2) * v[2],
						m(2,0) * v[0] + m(2,1) * v[1] + m(2,2) * v[2] };
}

/// Computes the product of a 3x4 matrix and a Point3 (which is extended to a 4-vector with the last element being 1).
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR Point_3<T> operator*(const AffineTransformationT<T>& m, const Point_3<T>& p)
{
	return Point_3<T>{ m(0,0) * p[0] + m(0,1) * p[1] + m(0,2) * p[2] + m(0,3),
						m(1,0) * p[0] + m(1,1) * p[1] + m(1,2) * p[2] + m(1,3),
						m(2,0) * p[0] + m(2,1) * p[1] + m(2,2) * p[2] + m(2,3) };
}

/// Computes the product of two 3x4 matrices. The last row of the extended 4x4 matrix is assumed to be (0,0,0,1).
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR AffineTransformationT<T> operator*(const AffineTransformationT<T>& a, const AffineTransformationT<T>& b)
{
#if 1
	return AffineTransformationT<T>(
			a(0,0)*b(0,0) + a(0,1)*b(1,0) + a(0,2)*b(2,0),
			a(0,0)*b(0,1) + a(0,1)*b(1,1) + a(0,2)*b(2,1),
			a(0,0)*b(0,2) + a(0,1)*b(1,2) + a(0,2)*b(2,2),
			a(0,0)*b(0,3) + a(0,1)*b(1,3) + a(0,2)*b(2,3) + a(0,3),

			a(1,0)*b(0,0) + a(1,1)*b(1,0) + a(1,2)*b(2,0),
			a(1,0)*b(0,1) + a(1,1)*b(1,1) + a(1,2)*b(2,1),
			a(1,0)*b(0,2) + a(1,1)*b(1,2) + a(1,2)*b(2,2),
			a(1,0)*b(0,3) + a(1,1)*b(1,3) + a(1,2)*b(2,3) + a(1,3),

			a(2,0)*b(0,0) + a(2,1)*b(1,0) + a(2,2)*b(2,0),
			a(2,0)*b(0,1) + a(2,1)*b(1,1) + a(2,2)*b(2,1),
			a(2,0)*b(0,2) + a(2,1)*b(1,2) + a(2,2)*b(2,2),
			a(2,0)*b(0,3) + a(2,1)*b(1,3) + a(2,2)*b(2,3) + a(2,3)
	);
#else
	AffineTransformationT<T> res;
	for(typename AffineTransformationT<T>::size_type i = 0; i < 3; i++) {
		for(typename AffineTransformationT<T>::size_type j = 0; j < 4; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) += a(i,3);
	}
	return res;
#endif
}

/// Multiplies a 3x4 matrix with a scalar.
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR AffineTransformationT<T> operator*(const AffineTransformationT<T>& a, T s)
{
	return { a.column(0)*s, a.column(1)*s, a.column(2)*s, a.column(3)*s };
}

/// Multiplies a 3x4 matrix with a scalar.
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR AffineTransformationT<T> operator*(T s, const AffineTransformationT<T>& a)
{
	return a * s;
}

/// Computes the product of a 3x3 matrix and a 3x4 matrix.
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR AffineTransformationT<T> operator*(const Matrix_3<T>& a, const AffineTransformationT<T>& b)
{
#if 1
	return AffineTransformationT<T>(
			a(0,0)*b(0,0) + a(0,1)*b(1,0) + a(0,2)*b(2,0),
			a(0,0)*b(0,1) + a(0,1)*b(1,1) + a(0,2)*b(2,1),
			a(0,0)*b(0,2) + a(0,1)*b(1,2) + a(0,2)*b(2,2),
			a(0,0)*b(0,3) + a(0,1)*b(1,3) + a(0,2)*b(2,3),

			a(1,0)*b(0,0) + a(1,1)*b(1,0) + a(1,2)*b(2,0),
			a(1,0)*b(0,1) + a(1,1)*b(1,1) + a(1,2)*b(2,1),
			a(1,0)*b(0,2) + a(1,1)*b(1,2) + a(1,2)*b(2,2),
			a(1,0)*b(0,3) + a(1,1)*b(1,3) + a(1,2)*b(2,3),

			a(2,0)*b(0,0) + a(2,1)*b(1,0) + a(2,2)*b(2,0),
			a(2,0)*b(0,1) + a(2,1)*b(1,1) + a(2,2)*b(2,1),
			a(2,0)*b(0,2) + a(2,1)*b(1,2) + a(2,2)*b(2,2),
			a(2,0)*b(0,3) + a(2,1)*b(1,3) + a(2,2)*b(2,3)
	);
#else
	AffineTransformationT<T> res;
	for(typename AffineTransformationT<T>::size_type i = 0; i < 3; i++)
		for(typename AffineTransformationT<T>::size_type j = 0; j < 4; j++)
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
	return res;
#endif
}

/// Computes the product of a 3x4 matrix and a 3x3 matrix.
/// \relates AffineTransformationT
template<typename T>
inline Q_DECL_CONSTEXPR AffineTransformationT<T> operator*(const AffineTransformationT<T>& a, const Matrix_3<T>& b)
{
#if 1
	return AffineTransformationT<T>(
			a(0,0)*b(0,0) + a(0,1)*b(1,0) + a(0,2)*b(2,0),
			a(0,0)*b(0,1) + a(0,1)*b(1,1) + a(0,2)*b(2,1),
			a(0,0)*b(0,2) + a(0,1)*b(1,2) + a(0,2)*b(2,2),
			a(0,3),

			a(1,0)*b(0,0) + a(1,1)*b(1,0) + a(1,2)*b(2,0),
			a(1,0)*b(0,1) + a(1,1)*b(1,1) + a(1,2)*b(2,1),
			a(1,0)*b(0,2) + a(1,1)*b(1,2) + a(1,2)*b(2,2),
			a(1,3),

			a(2,0)*b(0,0) + a(2,1)*b(1,0) + a(2,2)*b(2,0),
			a(2,0)*b(0,1) + a(2,1)*b(1,1) + a(2,2)*b(2,1),
			a(2,0)*b(0,2) + a(2,1)*b(1,2) + a(2,2)*b(2,2),
			a(2,3)
	);
#else
	AffineTransformationT<T> res;
	for(typename AffineTransformationT<T>::size_type i = 0; i < 3; i++) {
		for(typename AffineTransformationT<T>::size_type j = 0; j < 3; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) = a(i,3);
	}
	return res;
#endif
}

// Generates a pure rotation matrix around the given axis.
template<typename T>
inline AffineTransformationT<T> AffineTransformationT<T>::rotation(const RotationT<T>& rot)
{
	T c = cos(rot.angle());
	T s = sin(rot.angle());
	T t = T(1) - c;
    const auto& a = rot.axis();
	OVITO_ASSERT_MSG(std::abs(a.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON), "AffineTransformation::rotation", "Rotation axis vector must be normalized.");

	// Make sure the result is a pure rotation matrix.
#ifdef OVITO_DEBUG
	AffineTransformationT<T> tm(	t * a.x() * a.x() + c,       t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(), 0.0,
						t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c,       t * a.y() * a.z() - s * a.x(), 0.0,
						t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c      , 0.0);
    OVITO_ASSERT_MSG(tm.isRotationMatrix(), "AffineTransformation::rotation(const Rotation&)" , "Result is not a pure rotation matrix.");
#endif

	return AffineTransformationT<T>(	t * a.x() * a.x() + c,       t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(), 0.0,
					t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c,       t * a.y() * a.z() - s * a.x(), 0.0,
					t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c      , 0.0);
}

// Generates a pure rotation matrix from a quaternion.
template<typename T>
inline AffineTransformationT<T> AffineTransformationT<T>::rotation(const QuaternionT<T>& q)
{
#ifdef OVITO_DEBUG
	if(std::abs(q.dot(q) - T(1)) > T(FLOATTYPE_EPSILON))
		OVITO_ASSERT_MSG(false, "AffineTransformation::rotation(const Quaternion&)", "Quaternion must be normalized.");

	// Make sure the result is a pure rotation matrix.
	AffineTransformationT<T> tm(T(1) - T(2)*(q.y()*q.y() + q.z()*q.z()),       T(2)*(q.x()*q.y() - q.w()*q.z()),       T(2)*(q.x()*q.z() + q.w()*q.y()), T(0),
			T(2)*(q.x()*q.y() + q.w()*q.z()), T(1) - T(2)*(q.x()*q.x() + q.z()*q.z()),       T(2)*(q.y()*q.z() - q.w()*q.x()), T(0),
			T(2)*(q.x()*q.z() - q.w()*q.y()),       T(2)*(q.y()*q.z() + q.w()*q.x()), T(1) - T(2)*(q.x()*q.x() + q.y()*q.y()), T(0));
    OVITO_ASSERT_MSG(tm.isRotationMatrix(), "AffineTransformation::rotation(const Quaternion&)" , "Result is not a pure rotation matrix.");
#endif
	return AffineTransformationT<T>(T(1) - T(2)*(q.y()*q.y() + q.z()*q.z()),       T(2)*(q.x()*q.y() - q.w()*q.z()),       T(2)*(q.x()*q.z() + q.w()*q.y()), T(0),
						T(2)*(q.x()*q.y() + q.w()*q.z()), T(1) - T(2)*(q.x()*q.x() + q.z()*q.z()),       T(2)*(q.y()*q.z() - q.w()*q.x()), T(0),
						T(2)*(q.x()*q.z() - q.w()*q.y()),       T(2)*(q.y()*q.z() + q.w()*q.x()), T(1) - T(2)*(q.x()*q.x() + q.y()*q.y()), T(0));
}

// Generates a pure scaling matrix.
template<typename T>
inline AffineTransformationT<T> AffineTransformationT<T>::scaling(const ScalingT<T>& scaling)
{
	Matrix_3<T> U = Matrix_3<T>::rotation(scaling.Q);
	Matrix_3<T> K = Matrix_3<T>(scaling.S.x(), T(0), T(0),
								T(0), scaling.S.y(), T(0),
								T(0), T(0), scaling.S.z());
	return AffineTransformationT<T>(U * K * U.transposed());
}

/// Prints a matrix to an output stream.
/// \relates AffineTransformationT
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const AffineTransformationT<T>& m) {
	for(typename AffineTransformationT<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Prints a matrix to a Qt debug stream.
/// \relates AffineTransformationT
template<typename T>
inline QDebug operator<<(QDebug dbg, const AffineTransformationT<T>& m) {
	for(typename AffineTransformationT<T>::size_type row = 0; row < m.row_count(); row++)
		dbg.nospace() << m(row,0) << " " << m(row,1) << " " << m(row,2) << " " << m(row,3) << "\n";
    return dbg.space();
}

/// \brief Writes a matrix to a binary output stream.
/// \relates AffineTransformationT
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const AffineTransformationT<T>& m)
{
	for(typename AffineTransformationT<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.column(col);
	return stream;
}

/// \brief Reads a matrix from a binary input stream.
/// \relates AffineTransformationT
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, AffineTransformationT<T>& m)
{
	for(typename AffineTransformationT<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.column(col);
	return stream;
}

/// \brief Writes a matrix to a Qt data stream.
/// \relates AffineTransformationT
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const AffineTransformationT<T>& m) {
	for(typename AffineTransformationT<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.column(col);
	return stream;
}

/// \brief Reads a matrix from a Qt data stream.
/// \relates AffineTransformationT
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, AffineTransformationT<T>& m) {
	for(typename AffineTransformationT<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.column(col);
	return stream;
}

/**
 * \brief Instantiation of the AffineTransformationT class template with the default floating-point type.
 * \relates AffineTransformationT
 */
typedef AffineTransformationT<FloatType>		AffineTransformation;

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::AffineTransformation);
Q_DECLARE_METATYPE(Ovito::AffineTransformation*);
Q_DECLARE_TYPEINFO(Ovito::AffineTransformation, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::AffineTransformation*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_AFFINE_TRANSFORMATION_H
