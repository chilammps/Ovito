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
 * \file Matrix3.h 
 * \brief Contains definition of the Ovito::Matrix3 class and operators.
 */
 
#ifndef __OVITO_MATRIX3_H
#define __OVITO_MATRIX3_H

#include <base/Base.h>
#include <base/utilities/Exception.h>
#include "Vector3.h"
#include "Point3.h"

namespace Ovito {

template<typename T> class RotationT;
template<typename T> class QuaternionT;
template<typename T> class ScalingT;

/**
 * \brief A 3x3 matrix class.
 */
template<typename T>
class Matrix_3
{
private:

	/// The 3 x 3 elements of the matrix.
	/// Elements are stored in column-major order, i.e. the first 
	/// array index specifies the column and the second the row.
	Vector_3<T> _m[3];

public:

	struct Zero {};
	struct Identity {};

	typedef T value_type;
	typedef std::size_t size_type;

public:

	/// \brief Constructs a matrix without initializing its elements.
	/// \note All elements are left uninitialized by this constructor and have therefore an undefined value!
	Matrix_3() {}

	/// \brief Constructor that initializes all 9 elements of the matrix to the given values.
	/// \note Values are given in row-major order, i.e. row by row.
	constexpr Matrix_3(T m11, T m12, T m13,
					   T m21, T m22, T m23,
					   T m31, T m32, T m33) :
		_m{Vector_3<T>(m11,m21,m31),Vector_3<T>(m12,m22,m32),Vector_3<T>(m13,m23,m33)} {}

	/// \brief Constructor that initializes the matrix from three column vectors.
	constexpr Matrix_3(const Vector_3<T>& c1, const Vector_3<T>& c2, const Vector_3<T>& c3) : _m{c1, c2, c3} {}

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
	constexpr Matrix_3(Zero) : _m{Vector_3<T>::Zero(),Vector_3<T>::Zero(),Vector_3<T>::Zero()} {}

	/// \brief Initializes the matrix to the identity matrix.
	/// All diagonal elements are set to one and all off-diagonal elements are set to zero.
	constexpr Matrix_3(Identity) : _m{
				Vector_3<T>(T(1),T(0),T(0)),
				Vector_3<T>(T(0),T(1),T(0)),
				Vector_3<T>(T(0),T(0),T(2))} {}

	/// \brief Returns the number of rows in this matrix.
	constexpr size_type row_count() const { return 3; }

	/// \brief Returns the columns of rows in this matrix.
	constexpr size_type col_count() const { return 3; }

	/// \brief Returns the value of a matrix element.
	/// \param row The row of the element to return.
	/// \param col The column of the element to return.
	/// \return The value of the matrix element.
	constexpr inline const T& operator()(size_type row, size_type col) const {
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
	constexpr const Vector_3<T>& column(size_type col) const {
		return _m[col];
	}

	/// \brief Returns a reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return The i-th column of the matrix as a vector reference. Modifying the vector modifies the matrix.
	Vector_3<T>& column(size_type col) {
		return _m[col];
	}
	
	/// \brief Returns a row from the matrix.
	/// \param row The row to return.
	/// \return The i-th row of the matrix as a vector.
	constexpr Vector_3<T> row(size_type row) const {
		return { _m[0][row], _m[1][row], _m[2][row] };
	}

	/// \brief Sets all components of the matrix to zero.
	Matrix_3& setZero() {
		_m[0].setZero();
		_m[1].setZero();
		_m[2].setZero();
		return *this;
	}

	/// \brief Sets all components of the matrix to zero.
	Matrix_3& operator=(Zero) {
		return setZero();
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_3& setIdentity(Identity) {
		_m[0] = Vector_3<T>(1,0,0);
		_m[1] = Vector_3<T>(0,1,0);
		_m[2] = Vector_3<T>(0,0,1);
		return *this;
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_3& operator=(Identity) {
		return setIdentity();
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Compares two matrices for exact equality.
	/// \return true if all elements are equal; false otherwise.
	constexpr bool operator==(const Matrix_3& b) const {
		return (b._m[0] == _m[0]) && (b._m[1] == _m[1]) && (b._m[2] == _m[2]);
	}

	/// \brief Compares two matrices for inequality.
	/// \return true if not all elements are equal; false if all are equal.
	constexpr bool operator!=(const Matrix_3& b) const {
		return (b._m[0] != _m[0]) || (b._m[1] != _m[1]) || (b._m[2] != _m[2]);
	}

	////////////////////////////////// Computations ///////////////////////////////////

	/// \brief Computes the inverse of the matrix. 
	/// \throw Exception if matrix is not invertible because it is singular.
	Matrix_3 inverse() const {
		T det = determinant();
		OVITO_ASSERT_MSG(det != T(0), "Matrix3::inverse()", "Singular matrix cannot be inverted: Determinant is zero.");
		if(det == 0) throw Exception("AffineTransformation cannot be inverted: determinant is zero.");
		return Matrix_3((_m[1][1]*_m[2][2] - _m[1][2]*_m[2][1])/det,
						(_m[2][0]*_m[1][2] - _m[1][0]*_m[2][2])/det,
						(_m[1][0]*_m[2][1] - _m[1][1]*_m[2][0])/det,
						(_m[2][1]*_m[0][2] - _m[0][1]*_m[2][2])/det,
						(_m[0][0]*_m[2][2] - _m[2][0]*_m[0][2])/det,
						(_m[0][1]*_m[2][0] - _m[0][0]*_m[2][1])/det,
						(_m[0][1]*_m[1][2] - _m[1][1]*_m[0][2])/det,
						(_m[0][2]*_m[1][0] - _m[0][0]*_m[1][2])/det,
						(_m[0][0]*_m[1][1] - _m[1][0]*_m[0][1])/det);
	}

	/// \brief Computes the determinant of the matrix.
	constexpr inline T determinant() const {
		return((_m[0][0]*_m[1][1] - _m[0][1]*_m[1][0])*(_m[2][2])
			  -(_m[0][0]*_m[1][2] - _m[0][2]*_m[1][0])*(_m[2][1])
			  +(_m[0][1]*_m[1][2] - _m[0][2]*_m[1][1])*(_m[2][0]));
	}

	/// \brief Returns the transpose of this matrix.
	/// \return A new matrix with columns and rows swapped.
	constexpr Matrix_3 transposed() const {
		return Matrix_3(_m[0][0], _m[0][1], _m[0][2],
						_m[1][0], _m[1][1], _m[1][2],
						_m[2][0], _m[2][1], _m[2][2]);
	}

	/// \brief Tests whether the matrix is a pure rotation matrix.
	/// \return \c If the matrix is a pure rotation matrix; \c false otherwise.
	///
	/// The matrix A is a pure rotation matrix if:
	///   (1) det(A) = 1  and 
	///   (2) A * A^T = I
	constexpr bool isRotationMatrix(T epsilon = T(FLOATTYPE_EPSILON)) const {
		return
			(std::abs(_m[0][0]*_m[1][0] + _m[0][1]*_m[1][1] + _m[0][2]*_m[1][2]) <= epsilon) &&
			(std::abs(_m[0][0]*_m[2][0] + _m[0][1]*_m[2][1] + _m[0][2]*_m[2][2]) <= epsilon) &&
			(std::abs(_m[1][0]*_m[2][0] + _m[1][1]*_m[2][1] + _m[1][2]*_m[2][2]) <= epsilon) &&
			(std::abs(_m[0][0]*_m[0][0] + _m[0][1]*_m[0][1] + _m[0][2]*_m[0][2] - T(1)) <= epsilon) &&
			(std::abs(_m[1][0]*_m[1][0] + _m[1][1]*_m[1][1] + _m[1][2]*_m[1][2] - T(1)) <= epsilon) &&
			(std::abs(_m[2][0]*_m[2][0] + _m[2][1]*_m[2][1] + _m[2][2]*_m[2][2] - T(1)) <= epsilon) &&
			(std::abs(determinant() - T(1)) <= epsilon);
	}

	////////////////////////////////// Generation ///////////////////////////////////

	/// \brief Generates a matrix describing a rotation around the X axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_3 rotationX(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {1, 0, 0,
				0, c,-s,
				0, s, c};
	}

	/// \brief Generates a matrix describing a rotation around the Y axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_3 rotationY(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return { c, 0, s,
			     0, 1, 0,
			    -s, 0, c};
	}

	/// \brief Generates a matrix describing a rotation around the Z axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_3 rotationZ(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {c, -s, 0,
				s,  c, 0,
				0,  0, 1};
	}

	/// \brief Generates a rotation matrix from an axis and an angle.
	/// \param rot The rotation specified in the Rotation structure.
	static Matrix_3 rotation(const RotationT<T>& rot);

	/// \brief Generates a rotation matrix from a quaternion.
	/// \param q The Quaternion describing the 3d rotation.
	static Matrix_3 rotation(const QuaternionT<T>& q);

	/// \brief Generates a scaling matrix.
	static Matrix_3 scaling(const ScalingT<T>& scaling);
};

};

#include "Quaternion.h"
#include "Scaling.h"
#include "Rotation.h"

namespace Ovito {

/// \brief Generates a rotation matrix from an axis and an angle.
/// \param rot The rotation specified in the Rotation structure.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::rotation(const RotationT<T>& rot)
{
	T c = cos(rot.angle());
	T s = sin(rot.angle());
	T t = T(1) - c;
	const auto& a = rot.axis();
	OVITO_ASSERT_MSG(std::abs(a.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON), "Matrix3::rotation", "Rotation axis vector must be normalized.");
	return Matrix_3<T>(	t * a.x() * a.x() + c,       t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(),
					t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c,       t * a.y() * a.z() - s * a.x(),
					t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c       );
}

/// Generates a rotation matrix from a quaternion.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::rotation(const QuaternionT<T>& q)
{
#ifdef OVITO_DEBUG
	if(std::abs(q.dot(q) - T(1)) > T(FLOATTYPE_EPSILON)) {
		OVITO_ASSERT_MSG(false, "Matrix3::rotation", "Quaternion must be normalized.");
	}
#endif
	return Matrix_3<T>(1.0 - 2.0*(q.y()*q.y() + q.z()*q.z()),       2.0*(q.x()*q.y() - q.w()*q.z()),       2.0*(q.x()*q.z() + q.w()*q.y()),
				         2.0*(q.x()*q.y() + q.w()*q.z()), 1.0 - 2.0*(q.x()*q.x() + q.z()*q.z()),       2.0*(q.y()*q.z() - q.w()*q.x()),
			             2.0*(q.x()*q.z() - q.w()*q.y()),       2.0*(q.y()*q.z() + q.w()*q.x()), 1.0 - 2.0*(q.x()*q.x() + q.y()*q.y()));
}


/// Generates a scaling matrix.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::scaling(const ScalingT<T>& scaling)
{
	Matrix_3<T> U = Matrix_3<T>::rotation(scaling.Q);
	Matrix_3<T> K = Matrix_3<T>(scaling.S.x(), 0.0, 0.0,
						0.0, scaling.S.y(), 0.0,
						0.0, 0.0, scaling.S.z());
	return U * K * U.transposed();
}

/// \brief Multiplies a 3x3 matrix with a Vector3.
template<typename T>
constexpr inline Vector_3<T> operator*(const Matrix_3<T>& m, const Vector_3<T>& v)
{
	return { m(0,0)*v[0] + m(0,1)*v[1] + m(0,2)*v[2],
			 m(1,0)*v[0] + m(1,1)*v[1] + m(1,2)*v[2],
			 m(2,0)*v[0] + m(2,1)*v[1] + m(2,2)*v[2] };
}

/// \brief Multiplies a 3x3 matrix with a Point3.
template<typename T>
constexpr inline Point_3<T> operator*(const Matrix_3<T>& m, const Point_3<T>& p)
{
	return { m(0,0)*p[0] + m(0,1)*p[1] + m(0,2)*p[2],
			 m(1,0)*p[0] + m(1,1)*p[1] + m(1,2)*p[2],
			 m(2,0)*p[0] + m(2,1)*p[1] + m(2,2)*p[2] };
}

/// \brief Multiplies a 3x3 matrix with a 3x3 Matrix.
template<typename T>
inline Matrix_3<T> operator*(const Matrix_3<T>& a, const Matrix_3<T>& b)
{
	Matrix_3<T> m;
	for(typename Matrix_3<T>::size_type col = 0; col < 3; col++) {
		for(typename Matrix_3<T>::size_type row = 0; row < 3; row++) {
			T v{0};
			for(typename Matrix_3<T>::size_type k = 0; k < 3; k++)
				v += a(row, k) * b(k, col);
			m(row, col) = v;
		}
	}
	return m;
}

/// \brief Multiplies a 3x3 matrix with a scalar value. 
/// Each element of the matrix is multiplied by the scalar value.
template<typename T>
inline Matrix_3<T> operator*(const Matrix_3<T>& a, T s)
{
	Matrix_3<T> m;
	for(typename Matrix_3<T>::size_type i = 0; i < 3; i++) {
		for(typename Matrix_3<T>::size_type j = 0; j < 3; j++) {
			m(i, j) = a(i, j) * s;
		}
	}
	return m;
}

/// \brief Multiplies a 3x3 matrix with a scalar value. 
/// Each element of the matrix is multiplied by the scalar value.
template<typename T>
inline Matrix_3<T> operator*(T s, const Matrix_3<T>& a) {
	return a * s;
}

/// \brief Writes the matrix to a text output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Matrix_3<T>& m) {
	for(typename Matrix_3<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Writes a matrix to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Matrix_3<T>& m)
{
	for(typename Matrix_3<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.col(col);
	return stream;
}

/// \brief Reads a matrix from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Matrix_3<T>& m)
{
	for(typename Matrix_3<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.col(col);
	return stream;
}

/**
 * \fn typedef Matrix3
 * \brief Template class instance of the Matrix_3 class.
 */
typedef Matrix_3<FloatType>		Matrix3;


};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Matrix3)
Q_DECLARE_TYPEINFO(Ovito::Matrix3, Q_PRIMITIVE_TYPE);

#endif // __OVITO_MATRIX3_H
