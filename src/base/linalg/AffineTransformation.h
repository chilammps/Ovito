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
 * \file AffineTransformation.h 
 * \brief Contains the definition of the Ovito::Matrix_34 class template.
 */

#ifndef __OVITO_AFFINE_TRANSFORMATION_H
#define __OVITO_AFFINE_TRANSFORMATION_H

#include <base/Base.h>
#include "Vector3.h"
#include "Vector4.h"
#include "Point3.h"
#include "Matrix3.h"

namespace Ovito {

/**
 * \brief A 3x4 matrix class that describes an affine transformations in 3d space.
 *
 * This class stores a 3x4 floating-point array.
 */
template<typename T>
class Matrix_34
{
private:

	/// \brief The four columns of the matrix.
	/// The fourth column contains the translation vector.
	Vector_3<T> _m[4];

public:

	struct Zero {};
	struct Identity {};

	typedef T value_type;
	typedef Vector_3<T> column_type;
	typedef std::size_t size_type;

public:

	/// \brief Constructs a matrix without initializing its elements.
	/// \note All elements are left uninitialized by this constructor and have therefore an undefined value!
	Matrix_34() {}

	/// \brief Constructor that initializes 9 elements of the matrix to the given values. Translation is set to zero.
	/// \note Values are given in row-major order, i.e. row by row.
	Q_DECL_CONSTEXPR Matrix_34(T m11, T m12, T m13,
					    T m21, T m22, T m23,
					    T m31, T m32, T m33) :
		_m{Vector_3<T>(m11,m21,m31),
			Vector_3<T>(m12,m22,m32),
			Vector_3<T>(m13,m23,m33),
			typename Vector_3<T>::Zero()} {}

	/// \brief Constructor that initializes all elements of the matrix to the given values.
	/// \note Values are given in row-major order, i.e. row by row.
	Q_DECL_CONSTEXPR Matrix_34(T m11, T m12, T m13, T m14,
					    T m21, T m22, T m23, T m24,
					    T m31, T m32, T m33, T m34) :
		_m{Vector_3<T>(m11,m21,m31),
			Vector_3<T>(m12,m22,m32),
			Vector_3<T>(m13,m23,m33),
			Vector_3<T>(m14,m24,m34)} {}

	/// \brief Constructor that initializes the matrix from four column vectors.
	Q_DECL_CONSTEXPR Matrix_34(const column_type& c1, const column_type& c2, const column_type& c3, const column_type& c4) :
			_m{c1, c2, c3, c4} {}

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
	Q_DECL_CONSTEXPR Matrix_34(Zero) : _m{
		typename Vector_3<T>::Zero(),
		typename Vector_3<T>::Zero(),
		typename Vector_3<T>::Zero(),
		typename Vector_3<T>::Zero()} {}

	/// \brief Initializes the matrix to the identity matrix.
	/// All diagonal elements are set to one and all off-diagonal elements are set to zero.
	Q_DECL_CONSTEXPR Matrix_34(Identity) : _m{
		Vector_3<T>(T(1),T(0),T(0)),
		Vector_3<T>(T(0),T(1),T(0)),
		Vector_3<T>(T(0),T(0),T(1)),
		Vector_3<T>(T(0),T(0),T(0))} {}

	/// \brief Initializes the 3x4 matrix from a 3x3 matrix.
	/// The translation component of the affine transformation is set to the null vector.
	explicit Q_DECL_CONSTEXPR Matrix_34(const Matrix_3<T>& tm) : _m{tm.column(0), tm.column(1), tm.column(2), typename Vector_3<T>::Zero()} {}

	/// \brief Returns the number of rows in this matrix.
	static Q_DECL_CONSTEXPR size_type row_count() { return 3; }

	/// \brief Returns the columns of rows in this matrix.
	static Q_DECL_CONSTEXPR size_type col_count() { return 4; }

	/// \brief Returns the value of a matrix element.
	/// \param row The row of the element to return.
	/// \param col The column of the element to return.
	/// \return The value of the matrix element.
	inline Q_DECL_CONSTEXPR T operator()(size_type row, size_type col) const {
		return _m[col][row];
	}

	/// \brief Returns a reference to a matrix element.
	/// \param row The row of the element to return.
	/// \param col The column of the element to return.
	inline T& operator()(size_type row, size_type col) {
		return _m[col][row];
	}

	/// \brief Returns a column vector in the matrix.
	/// \param col The index of the column to return.
	/// \return The i-th column of the matrix as a vector.
	inline Q_DECL_CONSTEXPR const column_type& column(size_type col) const {
		return _m[col];
	}

	/// \brief Returns a reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return The i-th column of the matrix as a vector reference. Modifying the vector modifies the matrix.
	inline column_type& column(size_type col) {
		return _m[col];
	}
	
	/// \brief Returns a row from the matrix.
	/// \param row The row to return.
	/// \return The i-th row of the matrix as a vector.
	Q_DECL_CONSTEXPR Vector_4<T> row(size_type row) const {
		return { _m[0][row], _m[1][row], _m[2][row], _m[3][row] };
	}

	/// \brief Returns the translational part of this transformation matrix.
	/// \return A vector that specifies the translation.
	Q_DECL_CONSTEXPR const column_type& translation() const { return column(3); }

	/// \brief Returns a reference to the translational part of this transformation matrix.
	column_type& translation() { return column(3); }

	/// \brief Sets all components of the matrix to zero.
	Matrix_34& setZero() {
		for(size_type i = 0; i < col_count(); i++)
			_m[i].setZero();
		return *this;
	}

	/// \brief Sets all components of the matrix to zero.
	Matrix_34& operator=(Zero) {
		return setZero();
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_34& setIdentity() {
		_m[0] = Vector_3<T>(T(1),T(0),T(0));
		_m[1] = Vector_3<T>(T(0),T(1),T(0));
		_m[2] = Vector_3<T>(T(0),T(0),T(1));
		_m[3].setZero();
		return *this;
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_34& operator=(Identity) {
		return setIdentity();
	}

	/// \brief Returns a pointer to the element data of the matrix.
	/// \sa constData()
	T* data() {
		return reinterpret_cast<T*>(&_m);
	}

	/// \brief Returns a pointer to the element data of the matrix.
	/// \sa data()
	const T* constData() const {
		return reinterpret_cast<const T*>(&_m);
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Compares two matrices for exact equality.
	/// \return true if all elements are equal; false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Matrix_34& b) const {
		return (b._m[0] == _m[0]) && (b._m[1] == _m[1]) && (b._m[2] == _m[2]) && (b._m[3] == _m[3]);
	}

	/// \brief Compares two matrices for inequality.
	/// \return true if not all elements are equal; false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Matrix_34& b) const {
		return (b._m[0] != _m[0]) || (b._m[1] != _m[1]) || (b._m[2] != _m[2]) || (b._m[3] != _m[3]);
	}

	////////////////////////////////// Computations ///////////////////////////////////

	/// \brief Computes the determinant of the matrix.
	Q_DECL_CONSTEXPR inline T determinant() const {
		return((_m[0][0]*_m[1][1] - _m[0][1]*_m[1][0])*(_m[2][2])
			  -(_m[0][0]*_m[1][2] - _m[0][2]*_m[1][0])*(_m[2][1])
			  +(_m[0][1]*_m[1][2] - _m[0][2]*_m[1][1])*(_m[2][0]));
	}

	/// \brief Computes the inverse of the matrix. 
	/// \throw Exception if matrix is not invertible because it is singular.
	Matrix_34 inverse() const {
		// Compute inverse of 3x3 sub-matrix.
		// Then multiply with inverse translation.
		T det = determinant();
		OVITO_ASSERT_MSG(det != T(0), "AffineTransformation::inverse()", "Singular matrix cannot be inverted: Determinant is zero.");
		if(det == T(0)) throw Exception("Affine transformation cannot be inverted: Determinant is zero.");

		Matrix_34 inv((_m[1][1]*_m[2][2] - _m[1][2]*_m[2][1])/det,
						(_m[2][0]*_m[1][2] - _m[1][0]*_m[2][2])/det,
						(_m[1][0]*_m[2][1] - _m[1][1]*_m[2][0])/det,
						T(0),
						(_m[2][1]*_m[0][2] - _m[0][1]*_m[2][2])/det,
						(_m[0][0]*_m[2][2] - _m[2][0]*_m[0][2])/det,
						(_m[0][1]*_m[2][0] - _m[0][0]*_m[2][1])/det,
						T(0),
						(_m[0][1]*_m[1][2] - _m[1][1]*_m[0][2])/det,
						(_m[0][2]*_m[1][0] - _m[0][0]*_m[1][2])/det,
						(_m[0][0]*_m[1][1] - _m[1][0]*_m[0][1])/det,
						T(0));
		inv.translation() = inv * (-translation());
		return inv;
	}

	/// \brief Computes the inverse of the matrix.
	/// \return False if matrix is not invertible because it is singular; true if the inverse has been calculated
	///         and stored in the output parameter.
	bool inverse(Matrix_34& result, FloatType epsilon = FLOATTYPE_EPSILON) const {
		T det = determinant();
		if(std::abs(det) <= epsilon) return false;
		result = Matrix_34((_m[1][1]*_m[2][2] - _m[1][2]*_m[2][1])/det,
						(_m[2][0]*_m[1][2] - _m[1][0]*_m[2][2])/det,
						(_m[1][0]*_m[2][1] - _m[1][1]*_m[2][0])/det,
						T(0),
						(_m[2][1]*_m[0][2] - _m[0][1]*_m[2][2])/det,
						(_m[0][0]*_m[2][2] - _m[2][0]*_m[0][2])/det,
						(_m[0][1]*_m[2][0] - _m[0][0]*_m[2][1])/det,
						T(0),
						(_m[0][1]*_m[1][2] - _m[1][1]*_m[0][2])/det,
						(_m[0][2]*_m[1][0] - _m[0][0]*_m[1][2])/det,
						(_m[0][0]*_m[1][1] - _m[1][0]*_m[0][1])/det,
						T(0));
		result.translation() = result * (-translation());
		return true;
	}

    // Algorithm uses Gram-Schmidt orthogonalization.  If 'this' matrix is
    // M = [m0|m1|m2], then orthonormal output matrix is Q = [q0|q1|q2],
    //
    //   q0 = m0/|m0|
    //   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
    //   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
    //
    // where |V| indicates length of vector V and A*B indicates dot
    // product of vectors A and B.
	void orthonormalize() {

		// Compute q0.
		_m[0].normalize();

	    // Compute q1.
		T dot0 = _m[0].dot(_m[1]);
		_m[1][0] -= dot0 * _m[0][0];
		_m[1][1] -= dot0 * _m[0][1];
		_m[1][2] -= dot0 * _m[0][2];
		_m[1].normalize();

	    // compute q2
	    dot0 = _m[0].dot(_m[2]);
	    T dot1 = _m[1].dot(_m[2]);
	    _m[2][0] -= dot0*_m[0][0] + dot1*_m[1][0];
	    _m[2][1] -= dot0*_m[0][1] + dot1*_m[1][1];
	    _m[2][2] -= dot0*_m[0][2] + dot1*_m[1][2];
	    _m[2].normalize();
	}

	/// Multiplies a 3x4 matrix with a Point3 (which is extended to a 4-vector with the last
	/// element being 1) and returns one component of the resulting point.
	inline Q_DECL_CONSTEXPR T prodrow(const Point_3<T>& p, typename Point_3<T>::size_type index) const {
		return _m[0][index] * p[0] + _m[1][index] * p[1] + _m[2][index] * p[2] + _m[3][index];
	}

	/// Multiplies a 3x4 matrix with a Vector3 (which is automatically extended to a 4-vector with the last
	/// element being 0) and returns one component of the resulting vector.
	inline Q_DECL_CONSTEXPR T prodrow(const Vector_3<T>& v, typename Vector_3<T>::size_type index) const {
		return _m[0][index] * v[0] + _m[1][index] * v[1] + _m[2][index] * v[2];
	}

	/// Returns the upper left 3x3 submatrix of this 3x4 matrix containing only the rotation/scale/shear transformation but not the translations.
	inline Q_DECL_CONSTEXPR Matrix_3<T> linear() const {
		return Matrix_3<T>(_m[0], _m[1], _m[2]);
	}

	////////////////////////////////// Generation ///////////////////////////////////
	
	/// \brief Generates a matrix describing a rotation around the X axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_34 rotationX(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {T(1), T(0), T(0), T(0),
				T(0),    c,   -s, T(0),
				T(0),    s,    c, T(0)};
	}

	/// \brief Generates a matrix describing a rotation around the Y axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_34 rotationY(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {   c, T(0),    s, T(0),
				T(0), T(1), T(0), T(0),
				  -s, T(0),    c, T(0)};
	}

	/// \brief Generates a matrix describing a rotation around the Z axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_34 rotationZ(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {   c,   -s, T(0), T(0),
				   s,    c, T(0), T(0),
				T(0), T(0), T(1), T(0)};
	}

	/// Generates a pure rotation matrix around the given axis.
	static Matrix_34 rotation(const RotationT<T>& rot);

	/// Generates a pure rotation matrix from a quaternion.
	static Matrix_34 rotation(const QuaternionT<T>& q);

	/// Generates a pure translation matrix.
	static Q_DECL_CONSTEXPR Matrix_34 translation(const Vector_3<T>& t) {
		return Matrix_34(T(1), T(0), T(0), t.x(),
						 T(0), T(1), T(0), t.y(),
						 T(0), T(0), T(1), t.z());
	}

	/// Generates a pure diagonal scaling matrix.
	static Q_DECL_CONSTEXPR Matrix_34 scaling(T s) {
		return Matrix_34(   s, T(0), T(0), T(0),
						 T(0),    s, T(0), T(0),
						 T(0), T(0),    s, T(0));
	}

	/// Generates a pure scaling matrix.
	static Matrix_34 scaling(const ScalingT<T>& scaling);

	/// Generates a matrix with pure shearing transformation normal to the z-axis in the x- and y-direction.
	static Q_DECL_CONSTEXPR Matrix_34 shear(T gammaX, T gammaY) {
		return Matrix_34(T(1), T(0), gammaX, T(0),
						 T(0), T(1), gammaY, T(0),
						 T(0), T(0), T(1),   T(0));
	}

	/// Generates a matrix from an OpenGL transformation matrix stored in the given array.
	static Matrix_34 fromOpenGL(const T tm[16]) {
		OVITO_ASSERT(tm[3] == 0 && tm[7] == 0 && tm[11] == 0 && tm[15] == 1);
		return {tm[0], tm[4], tm[8], tm[12],
				tm[1], tm[5], tm[9], tm[13],
				tm[2], tm[6], tm[10], tm[14]};
	}

	/// \brief Generates a look-at-matrix. 
	/// \param camera The position of the camera in space.
	/// \param target The position in space where to camera should point to.
	/// \param upVector A vector pointing to the upward direction (the sky) that defines the rotation of the camera
	///                 around the viewing axis.
	/// \return The transformation from world space to view space.
	static Matrix_34 lookAt(const Point_3<T>& camera, const Point_3<T>& target, const Vector_3<T>& upVector) {
		return lookAlong(camera, target - camera, upVector);
	}

	/// \brief Generates a look-along-matrix.
	/// \param camera The position of the camera in space.
	/// \param direction The viewing direction.
	/// \param upVector A vector pointing to the upward direction (the sky) that defines the rotation of the camera
	///                 around the viewing axis.
	/// \return The transformation from world space to view space.
	static Matrix_34 lookAlong(const Point_3<T>& camera, const Vector_3<T>& direction, const Vector_3<T>& upVector) {
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

		return { xaxis.x(), xaxis.y(), xaxis.z(), -xaxis.dot(camera - typename Point_3<T>::Origin()),
				 yaxis.x(), yaxis.y(), yaxis.z(), -yaxis.dot(camera - typename Point_3<T>::Origin()),
				 zaxis.x(), zaxis.y(), zaxis.z(), -zaxis.dot(camera - typename Point_3<T>::Origin()) };
	}
	
	///////////////////////////////// Information ////////////////////////////////

	/// \brief Tests whether the matrix is a pure rotation matrix.
	/// \return \c If the matrix is a pure rotation matrix; \c false otherwise.
	///
	/// The matrix A is a pure rotation matrix if:
	///   (1) det(A) = 1  and
	///   (2) A * A^T = I
	Q_DECL_CONSTEXPR bool isRotationMatrix(T epsilon = T(FLOATTYPE_EPSILON)) const {
		return
			translation().isZero(epsilon) &&
			(std::abs(_m[0][0]*_m[1][0] + _m[0][1]*_m[1][1] + _m[0][2]*_m[1][2]) <= epsilon) &&
			(std::abs(_m[0][0]*_m[2][0] + _m[0][1]*_m[2][1] + _m[0][2]*_m[2][2]) <= epsilon) &&
			(std::abs(_m[1][0]*_m[2][0] + _m[1][1]*_m[2][1] + _m[1][2]*_m[2][2]) <= epsilon) &&
			(std::abs(_m[0][0]*_m[0][0] + _m[0][1]*_m[0][1] + _m[0][2]*_m[0][2] - T(1)) <= epsilon) &&
			(std::abs(_m[1][0]*_m[1][0] + _m[1][1]*_m[1][1] + _m[1][2]*_m[1][2] - T(1)) <= epsilon) &&
			(std::abs(_m[2][0]*_m[2][0] + _m[2][1]*_m[2][1] + _m[2][2]*_m[2][2] - T(1)) <= epsilon) &&
			(std::abs(determinant() - T(1)) <= epsilon);
	}

	/// \brief Converts this matrix to a Qt 4x4 matrix object.
	operator QMatrix4x4() const {
		return QMatrix4x4(
				(*this)(0,0), (*this)(0,1), (*this)(0,2), (*this)(0,3),
				(*this)(1,0), (*this)(1,1), (*this)(1,2), (*this)(1,3),
				(*this)(2,0), (*this)(2,1), (*this)(2,2), (*this)(2,3),
				0, 0, 0, 1);
	}
};

/// Multiplies a 3x4 matrix with a Vector3 (which is automatically extended to a 4-vector with the last
/// element being 0).
template<typename T>
inline Q_DECL_CONSTEXPR Vector_3<T> operator*(const Matrix_34<T>& m, const Vector_3<T>& v)
{
	return { m(0,0) * v[0] + m(0,1) * v[1] + m(0,2) * v[2],
			 m(1,0) * v[0] + m(1,1) * v[1] + m(1,2) * v[2],
			 m(2,0) * v[0] + m(2,1) * v[1] + m(2,2) * v[2] };
}

/// Multiplies a 3x4 matrix with a Point3 (which is extended to a 4-vector with the last
/// element being 1).
template<typename T>
inline Q_DECL_CONSTEXPR Point_3<T> operator*(const Matrix_34<T>& m, const Point_3<T>& p)
{
	return { m(0,0) * p[0] + m(0,1) * p[1] + m(0,2) * p[2] + m(0,3),
			 m(1,0) * p[0] + m(1,1) * p[1] + m(1,2) * p[2] + m(1,3),
			 m(2,0) * p[0] + m(2,1) * p[1] + m(2,2) * p[2] + m(2,3) };
}

/// Computes the product of a 3x4 matrix with another 3x4 Matrix.
template<typename T>
inline Q_DECL_CONSTEXPR Matrix_34<T> operator*(const Matrix_34<T>& a, const Matrix_34<T>& b)
{
#if 1
	return Matrix_34<T>(
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
	Matrix_34<T> res;
	for(typename Matrix_34<T>::size_type i = 0; i < 3; i++) {
		for(typename Matrix_34<T>::size_type j = 0; j < 4; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) += a(i,3);
	}
	return res;
#endif
}

/// Multiplies a 3x4 matrix with a scalar.
template<typename T>
inline Q_DECL_CONSTEXPR Matrix_34<T> operator*(const Matrix_34<T>& a, T s)
{
	return { a.column(0)*s, a.column(1)*s, a.column(2)*s, a.column(3)*s };
}

/// Multiplies a 3x4 matrix with a scalar.
template<typename T>
inline Q_DECL_CONSTEXPR Matrix_34<T> operator*(T s, const Matrix_34<T>& a)
{
	return a * s;
}

/// Computes the product of a 3x3 matrix and a 3x4 Matrix.
template<typename T>
inline Q_DECL_CONSTEXPR Matrix_34<T> operator*(const Matrix_3<T>& a, const Matrix_34<T>& b)
{
#if 1
	return Matrix_34<T>(
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
	Matrix_34<T> res;
	for(typename Matrix_34<T>::size_type i = 0; i < 3; i++)
		for(typename Matrix_34<T>::size_type j = 0; j < 4; j++)
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
	return res;
#endif
}

/// Computes the product of a 3x4 matrix and a 3x3 matrix.
template<typename T>
inline Q_DECL_CONSTEXPR Matrix_34<T> operator*(const Matrix_34<T>& a, const Matrix_3<T>& b)
{
#if 1
	return Matrix_34<T>(
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
	Matrix_34<T> res;
	for(typename Matrix_34<T>::size_type i = 0; i < 3; i++) {
		for(typename Matrix_34<T>::size_type j = 0; j < 3; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) = a(i,3);
	}
	return res;
#endif
}

/// Generates a pure rotation matrix around the given axis.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::rotation(const RotationT<T>& rot)
{
	T c = cos(rot.angle());
	T s = sin(rot.angle());
	T t = T(1) - c;
    const auto& a = rot.axis();
	OVITO_ASSERT_MSG(std::abs(a.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON), "AffineTransformation::rotation", "Rotation axis vector must be normalized.");

	// Make sure the result is a pure rotation matrix.
#ifdef OVITO_DEBUG
	Matrix_34<T> tm(	t * a.x() * a.x() + c,       t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(), 0.0,
						t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c,       t * a.y() * a.z() - s * a.x(), 0.0,
						t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c      , 0.0);
    OVITO_ASSERT_MSG(tm.isRotationMatrix(), "AffineTransformation::rotation(const Rotation&)" , "Result is not a pure rotation matrix.");
#endif

	return Matrix_34<T>(	t * a.x() * a.x() + c,       t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(), 0.0,
					t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c,       t * a.y() * a.z() - s * a.x(), 0.0,
					t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c      , 0.0);
}

/// Generates a pure rotation matrix from a quaternion.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::rotation(const QuaternionT<T>& q)
{
#ifdef OVITO_DEBUG
	if(std::abs(q.dot(q) - T(1)) > T(FLOATTYPE_EPSILON))
		OVITO_ASSERT_MSG(false, "AffineTransformation::rotation(const Quaternion&)", "Quaternion must be normalized.");

	// Make sure the result is a pure rotation matrix.
	Matrix_34<T> tm(T(1) - T(2)*(q.y()*q.y() + q.z()*q.z()),       T(2)*(q.x()*q.y() - q.w()*q.z()),       T(2)*(q.x()*q.z() + q.w()*q.y()), T(0),
			T(2)*(q.x()*q.y() + q.w()*q.z()), T(1) - T(2)*(q.x()*q.x() + q.z()*q.z()),       T(2)*(q.y()*q.z() - q.w()*q.x()), T(0),
			T(2)*(q.x()*q.z() - q.w()*q.y()),       T(2)*(q.y()*q.z() + q.w()*q.x()), T(1) - T(2)*(q.x()*q.x() + q.y()*q.y()), T(0));
    OVITO_ASSERT_MSG(tm.isRotationMatrix(), "AffineTransformation::rotation(const Quaternion&)" , "Result is not a pure rotation matrix.");
#endif
	return Matrix_34<T>(T(1) - T(2)*(q.y()*q.y() + q.z()*q.z()),       T(2)*(q.x()*q.y() - q.w()*q.z()),       T(2)*(q.x()*q.z() + q.w()*q.y()), T(0),
						T(2)*(q.x()*q.y() + q.w()*q.z()), T(1) - T(2)*(q.x()*q.x() + q.z()*q.z()),       T(2)*(q.y()*q.z() - q.w()*q.x()), T(0),
						T(2)*(q.x()*q.z() - q.w()*q.y()),       T(2)*(q.y()*q.z() + q.w()*q.x()), T(1) - T(2)*(q.x()*q.x() + q.y()*q.y()), T(0));
}

/// Generates a pure scaling matrix.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::scaling(const ScalingT<T>& scaling)
{
	Matrix_3<T> U = Matrix_3<T>::rotation(scaling.Q);
	Matrix_3<T> K = Matrix_3<T>(scaling.S.x(), T(0), T(0),
								T(0), scaling.S.y(), T(0),
								T(0), T(0), scaling.S.z());
	return Matrix_34<T>(U * K * U.transposed());
}

/// Writes the matrix to an output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Matrix_34<T>& m) {
	for(typename Matrix_34<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Writes the matrix to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Matrix_34<T>& m) {
	for(typename Matrix_34<T>::size_type row = 0; row < m.row_count(); row++)
		dbg.nospace() << m(row,0) << " " << m(row,1) << " " << m(row,2) << " " << m(row,3) << "\n";
    return dbg.space();
}

/// \brief Writes a matrix to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Matrix_34<T>& m)
{
	for(typename Matrix_34<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.column(col);
	return stream;
}

/// \brief Reads a matrix from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Matrix_34<T>& m)
{
	for(typename Matrix_34<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.column(col);
	return stream;
}

/// \brief Writes a matrix to a Qt data stream.
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Matrix_34<T>& m) {
	for(typename Matrix_34<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.column(col);
	return stream;
}

/// \brief Reads a matrix from a Qt data stream.
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Matrix_34<T>& m) {
	for(typename Matrix_34<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.column(col);
	return stream;
}

/**
 * \fn typedef AffineTransformation
 * \brief Template class instance of the Matrix_34 class.
 */
typedef Matrix_34<FloatType>		AffineTransformation;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::AffineTransformation);
Q_DECLARE_METATYPE(Ovito::AffineTransformation*);
Q_DECLARE_TYPEINFO(Ovito::AffineTransformation, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::AffineTransformation*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_AFFINE_TRANSFORMATION_H
