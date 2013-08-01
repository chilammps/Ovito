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
 * \file Matrix4.h 
 * \brief Contains definition of the Ovito::Matrix4 class and operators.
 */

#ifndef __OVITO_MATRIX4_H
#define __OVITO_MATRIX4_H

#include <base/Base.h>
#include "Vector3.h"
#include "Point3.h"
#include "Matrix3.h"
#include "Vector4.h"
#include "AffineTransformation.h"

namespace Ovito {

/**
 * \brief A 4x4 matrix class.
 *
 * In contrast to the AffineTransformation matrix class this Matrix4 class
 * can describe perspective projections.
 */
template<typename T>
class Matrix_4
{
private:

	/// \brief The four columns of the matrix.
	Vector_4<T> _m[4];

public:

	struct Zero {};
	struct Identity {};

	typedef T value_type;
	typedef std::size_t size_type;

public:

	/// \brief Constructs a matrix without initializing its elements.
	/// \note All elements are left uninitialized by this constructor and have therefore an undefined value!
	Matrix_4() {}

	/// \brief Constructor that initializes 9 elements of the matrix to the given values. All other elements are set to zero.
	/// \note Values are given in row-major order, i.e. row by row.
	Q_DECL_CONSTEXPR Matrix_4(T m11, T m12, T m13,
					   T m21, T m22, T m23,
					   T m31, T m32, T m33) :
		_m{{m11,m21,m31,T(0)},{m12,m22,m32,T(0)},{m13,m23,m33,T(0)},
			typename Vector_4<T>::Zero()} {}

	/// \brief Constructor that initializes 12 elements of the matrix to the given values. All other elements are set to zero.
	/// \note Values are given in row-major order, i.e. row by row.
	Q_DECL_CONSTEXPR Matrix_4(T m11, T m12, T m13, T m14,
					    T m21, T m22, T m23, T m24,
					    T m31, T m32, T m33, T m34) : _m{
		Vector_4<T>(m11,m21,m31,T(0)),
		Vector_4<T>(m12,m22,m32,T(0)),
		Vector_4<T>(m13,m23,m33,T(0)),
		Vector_4<T>(m14,m24,m34,T(0))} {}

	/// \brief Constructor that initializes 16 elements of the matrix to the given values.
	/// \note Values are given in row-major order, i.e. row by row.
	Q_DECL_CONSTEXPR Matrix_4(T m11, T m12, T m13, T m14,
						T m21, T m22, T m23, T m24,
						T m31, T m32, T m33, T m34,
						T m41, T m42, T m43, T m44) : _m{
		Vector_4<T>(m11,m21,m31,m41),
		Vector_4<T>(m12,m22,m32,m42),
		Vector_4<T>(m13,m23,m33,m43),
		Vector_4<T>(m14,m24,m34,m44)} {}

	/// \brief Constructor that initializes the matrix from four column vectors.
	Q_DECL_CONSTEXPR Matrix_4(const Vector_4<T>& c1, const Vector_4<T>& c2, const Vector_4<T>& c3, const Vector_4<T>& c4) : _m{c1, c2, c3, c4} {}

	/// \brief Initializes the 4x4 matrix from a 3x4 matrix.
	explicit Q_DECL_CONSTEXPR Matrix_4(const Matrix_34<T>& tm) : _m{
		Vector_4<T>(tm(0,0),tm(1,0),tm(2,0),T(0)),
		Vector_4<T>(tm(0,1),tm(1,1),tm(2,1),T(0)),
		Vector_4<T>(tm(0,2),tm(1,2),tm(2,2),T(0)),
		Vector_4<T>(tm(0,3),tm(1,3),tm(2,3),T(1))} {}

	/// \brief Initializes the 4x4 matrix from 4 column vectors.
	Q_DECL_CONSTEXPR Matrix_4(const Vector_3<T>& c1, const Vector_3<T>& c2, const Vector_3<T>& c3, const Vector_3<T>& c4) : _m{
		Vector_4<T>(c1[0],c1[1],c1[2],T(0)),
		Vector_4<T>(c2[0],c2[1],c2[2],T(0)),
		Vector_4<T>(c3[0],c3[1],c3[2],T(0)),
		Vector_4<T>(c4[0],c4[1],c4[2],T(1))} {}

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
	Q_DECL_CONSTEXPR Matrix_4(Zero) : _m{
		typename Vector_4<T>::Zero(),
		typename Vector_4<T>::Zero(),
		typename Vector_4<T>::Zero(),
		typename Vector_4<T>::Zero()} {}

	/// \brief Initializes the matrix to the identity matrix.
	/// All diagonal elements are set to one and all off-diagonal elements are set to zero.
	Q_DECL_CONSTEXPR Matrix_4(Identity) : _m{
		Vector_4<T>(T(1),T(0),T(0),T(0)),
		Vector_4<T>(T(0),T(1),T(0),T(0)),
		Vector_4<T>(T(0),T(0),T(1),T(0)),
		Vector_4<T>(T(0),T(0),T(0),T(1))} {}

	/// \brief Returns the number of rows in this matrix.
	static Q_DECL_CONSTEXPR size_type row_count() { return 4; }

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
	inline Q_DECL_CONSTEXPR const Vector_4<T>& column(size_type col) const {
		return _m[col];
	}

	/// \brief Returns a reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return The i-th column of the matrix as a vector reference. Modifying the vector modifies the matrix.
	inline Vector_4<T>& column(size_type col) {
		return _m[col];
	}
	
	/// \brief Returns a row from the matrix.
	/// \param row The row to return.
	/// \return The i-th row of the matrix as a vector.
	Q_DECL_CONSTEXPR Vector_4<T> row(size_type row) const {
		return { _m[0][row], _m[1][row], _m[2][row], _m[3][row] };
	}

	/// \brief Replaces a row of the matrix.
	/// \param row The row to replace.
	/// \param v The new row vector.
	void setRow(size_type row, const Vector_4<T>& v) {
		_m[0][row] = v[0];
		_m[1][row] = v[1];
		_m[2][row] = v[2];
		_m[3][row] = v[3];
	}

	/// \brief Sets all components of the matrix to zero.
	Matrix_4& setZero() {
		for(size_type i = 0; i < col_count(); i++)
			_m[i].setZero();
		return *this;
	}

	/// \brief Sets all components of the matrix to zero.
	Matrix_4& operator=(Zero) {
		return setZero();
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_4& setIdentity() {
		_m[0][0] = T(1); _m[0][1] = T(0); _m[0][2] = T(0); _m[0][3] = T(0);
		_m[1][0] = T(0); _m[1][1] = T(1); _m[1][2] = T(0); _m[1][3] = T(0);
		_m[2][0] = T(0); _m[2][1] = T(0); _m[2][2] = T(1); _m[2][3] = T(0);
		_m[3][0] = T(0); _m[3][1] = T(0); _m[3][2] = T(0); _m[3][3] = T(1);
		return *this;
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_4& operator=(Identity) {
		return setIdentity();
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Computes the determinant of the matrix.
	Q_DECL_CONSTEXPR inline T determinant() const {
		return (_m[0][3] * _m[1][2] * _m[2][1] * _m[3][0]-_m[0][2] * _m[1][3] * _m[2][1] * _m[3][0]-_m[0][3] * _m[1][1] * _m[2][2] * _m[3][0]+_m[0][1] * _m[1][3] * _m[2][2] * _m[3][0]+
				_m[0][2] * _m[1][1] * _m[2][3] * _m[3][0]-_m[0][1] * _m[1][2] * _m[2][3] * _m[3][0]-_m[0][3] * _m[1][2] * _m[2][0] * _m[3][1]+_m[0][2] * _m[1][3] * _m[2][0] * _m[3][1]+
				_m[0][3] * _m[1][0] * _m[2][2] * _m[3][1]-_m[0][0] * _m[1][3] * _m[2][2] * _m[3][1]-_m[0][2] * _m[1][0] * _m[2][3] * _m[3][1]+_m[0][0] * _m[1][2] * _m[2][3] * _m[3][1]+
				_m[0][3] * _m[1][1] * _m[2][0] * _m[3][2]-_m[0][1] * _m[1][3] * _m[2][0] * _m[3][2]-_m[0][3] * _m[1][0] * _m[2][1] * _m[3][2]+_m[0][0] * _m[1][3] * _m[2][1] * _m[3][2]+
				_m[0][1] * _m[1][0] * _m[2][3] * _m[3][2]-_m[0][0] * _m[1][1] * _m[2][3] * _m[3][2]-_m[0][2] * _m[1][1] * _m[2][0] * _m[3][3]+_m[0][1] * _m[1][2] * _m[2][0] * _m[3][3]+
				_m[0][2] * _m[1][0] * _m[2][1] * _m[3][3]-_m[0][0] * _m[1][2] * _m[2][1] * _m[3][3]-_m[0][1] * _m[1][0] * _m[2][2] * _m[3][3]+_m[0][0] * _m[1][1] * _m[2][2] * _m[3][3]);
	}

	/// \brief Computes the inverse of the matrix. 
	/// \throw Exception if matrix is not invertible because it is singular.
	Matrix_4 inverse() const {

		T det = determinant();
		OVITO_ASSERT_MSG(det != T(0), "Matrix4::inverse()", "Singular matrix cannot be inverted: Determinant is zero.");
		if(det == T(0))
			throw Exception("Matrix4 cannot be inverted: Determinant is zero.");

		// Assign to individual variable names to aid
		// selecting correct values.
		const T a1 = _m[0][0]; const T b1 = _m[0][1];
		const T c1 = _m[0][2]; const T d1 = _m[0][3];
		const T a2 = _m[1][0]; const T b2 = _m[1][1];
		const T c2 = _m[1][2]; const T d2 = _m[1][3];
		const T a3 = _m[2][0]; const T b3 = _m[2][1];
		const T c3 = _m[2][2]; const T d3 = _m[2][3];
		const T a4 = _m[3][0]; const T b4 = _m[3][1];
		const T c4 = _m[3][2]; const T d4 = _m[3][3];

	    return Matrix_4(
				det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4) / det,
			  - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4) / det,
				det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4) / det,
			  - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4) / det,

			  - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4) / det,
				det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4) / det,
			  - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4) / det,
				det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4) / det,

				det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4) / det,
			  - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4) / det,
				det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4) / det,
			  - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4) / det,

			  - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3) / det,
				det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3) / det,
			  - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3) / det,
				det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3) / det);
	}

	/// \brief Returns a pointer to the first element of the matrix.
	/// \return A pointer to 16 matrix elements in column-major order.
	/// \sa constData() 
	T* data() {
        OVITO_STATIC_ASSERT(sizeof(_m) == sizeof(T) * col_count() * row_count());
		return reinterpret_cast<T*>(&_m);
	}

	/// \brief Returns a pointer to the first element of the matrix.
	/// \return A pointer to 16 matrix elements in column-major order.
	/// \sa data() 
	const T* constData() const {
        OVITO_STATIC_ASSERT(sizeof(_m) == sizeof(T) * col_count() * row_count());
		return reinterpret_cast<const T*>(&_m);
	}

	/// \brief Converts this matrix to a Qt 4x4 matrix object.
	operator QMatrix4x4() const {
		return QMatrix4x4(
				(*this)(0,0), (*this)(0,1), (*this)(0,2), (*this)(0,3),
				(*this)(1,0), (*this)(1,1), (*this)(1,2), (*this)(1,3),
				(*this)(2,0), (*this)(2,1), (*this)(2,2), (*this)(2,3),
				(*this)(3,0), (*this)(3,1), (*this)(3,2), (*this)(3,3));
	}


	///////////////////////////// Generation //////////////////////////////////

	/// \brief Generates a translation matrix.
	static Matrix_4<T> translation(const Vector_3<T>& t) {
		return { T(1), T(0), T(0), t.x(),
				 T(0), T(1), T(0), t.y(),
				 T(0), T(0), T(1), t.z(),
				 T(0), T(0), T(0), T(1) };
	}

	/// \brief Generates a perspective projection matrix.
	static Matrix_4<T> perspective(T fovy, T aspect, T znear, T zfar) {
		T f = tan(fovy * T(0.5));
		OVITO_ASSERT(f != T(0));
		OVITO_ASSERT(zfar > znear);
		return { T(1)/(aspect*f), T(0), T(0), T(0),
				 T(0), T(1)/f, T(0), T(0),
				 T(0), T(0), -(zfar+znear)/(zfar-znear), -(T(2)*zfar*znear)/(zfar-znear),
				 T(0), T(0), T(-1), T(0) };
	}

	/// \brief Generates an orthogonal projection matrix.
	static Matrix_4<T> ortho(T left, T right, T bottom, T top, T znear, T zfar) {
		OVITO_ASSERT(znear < zfar);
		return { T(2)/(right-left), T(0),  T(0), -(right+left)/(right-left),
				 T(0), T(2)/(top-bottom), T(0), -(top+bottom)/(top-bottom),
				 T(0), T(0), T(-2)/(zfar-znear), -(zfar+znear)/(zfar-znear),
				 T(0), T(0), T(0), T(1) };
	}

private:

	// Computes the determinant of a 2x2 matrix. This is for internal use only.
	static Q_DECL_CONSTEXPR inline T det2x2(T a, T b, T c, T d) { return (a * d - b * c); }

	// Computes the determinant of a 3x3 matrix. This is for internal use only.
	static Q_DECL_CONSTEXPR inline T det3x3(T a1, T a2, T a3, T b1, T b2, T b3, T c1, T c2, T c3) {
		return (a1 * det2x2( b2, b3, c2, c3 )
			- b1 * det2x2( a2, a3, c2, c3 )
	        + c1 * det2x2( a2, a3, b2, b3 ));
	}

};

/// \brief Multiplies a 4x4 matrix with a Vector4.
template<typename T>
Q_DECL_CONSTEXPR inline Vector_4<T> operator*(const Matrix_4<T>& a, const Vector_4<T>& v)
{
	return {
		a(0,0)*v[0] + a(0,1)*v[1] + a(0,2)*v[2] + a(0,3)*v[3],
		a(1,0)*v[0] + a(1,1)*v[1] + a(1,2)*v[2] + a(1,3)*v[3],
		a(2,0)*v[0] + a(2,1)*v[1] + a(2,2)*v[2] + a(2,3)*v[3],
		a(3,0)*v[0] + a(3,1)*v[1] + a(3,2)*v[2] + a(3,3)*v[3]
	};
}

/// \brief Multiplies a 4x4 matrix with a Vector3.
template<typename T>
inline Vector_3<T> operator*(const Matrix_4<T>& a, const Vector_3<T>& v)
{
	T s = a(3,0)*v[0] + a(3,1)*v[1] + a(3,2)*v[2] + a(3,3);
	return {
		(a(0,0)*v[0] + a(0,1)*v[1] + a(0,2)*v[2]) / s,
		(a(1,0)*v[0] + a(1,1)*v[1] + a(1,2)*v[2]) / s,
		(a(2,0)*v[0] + a(2,1)*v[1] + a(2,2)*v[2]) / s
	};
}

/// \brief Multiplies a 4x4 matrix with a Point3.
template<typename T>
inline Point_3<T> operator*(const Matrix_4<T>& a, const Point_3<T>& v)
{
	T s = a(3,0)*v[0] + a(3,1)*v[1] + a(3,2)*v[2] + a(3,3);
	return {
		(a(0,0)*v[0] + a(0,1)*v[1] + a(0,2)*v[2] + a(0,3)) / s,
		(a(1,0)*v[0] + a(1,1)*v[1] + a(1,2)*v[2] + a(1,3)) / s,
		(a(2,0)*v[0] + a(2,1)*v[1] + a(2,2)*v[2] + a(2,3)) / s
	};
}

/// Computes the product of a 4x4 matrix with another 4x4 Matrix.
template<typename T>
inline Matrix_4<T> operator*(const Matrix_4<T>& a, const Matrix_4<T>& b)
{
	Matrix_4<T> res;
	for(typename Matrix_4<T>::size_type i = 0; i < 4; i++) {
		for(typename Matrix_4<T>::size_type j = 0; j < 4; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j) + a(i,3)*b(3,j);
		}
	}
	return res;
}

/// Computes the product of a 4x4 matrix with a 3x4 Matrix.
template<typename T>
inline Matrix_4<T> operator*(const Matrix_4<T>& a, const Matrix_34<T>& b)
{
	Matrix_4<T> res;
	for(typename Matrix_4<T>::size_type i = 0; i < 4; i++) {
		for(typename Matrix_4<T>::size_type j = 0; j < 3; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) = a(i,0)*b(0,3) + a(i,1)*b(1,3) + a(i,2)*b(2,3) + a(i,3);
	}
	return res;
}

/// Multiplies a 4x4 matrix with a scalar.
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_4<T> operator*(const Matrix_4<T>& a, T s)
{
	return { a.column(0)*s, a.column(1)*s, a.column(2)*s, a.column(3)*s };
}

/// Multiplies a 4x4 matrix with a scalar.
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_4<T> operator*(T s, const Matrix_4<T>& a)
{
	return a * s;
}

/// Writes the matrix to an output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Matrix_4<T>& m) {
	for(typename Matrix_4<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Writes the matrix to the Qt debug stream.
template<typename T>
inline QDebug operator<<(QDebug dbg, const Matrix_4<T>& m) {
	for(typename Matrix_4<T>::size_type row = 0; row < m.row_count(); row++)
		dbg.nospace() << m(row,0) << " " << m(row,1) << " " << m(row,2) << " " << m(row,3) << "\n";
    return dbg.space();
}

/// \brief Writes a matrix to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Matrix_4<T>& m)
{
	for(typename Matrix_4<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.col(col);
	return stream;
}

/// \brief Reads a matrix from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Matrix_4<T>& m)
{
	for(typename Matrix_4<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.col(col);
	return stream;
}

/**
 * \fn typedef Matrix4
 * \brief Template class instance of the Matrix_4 class.
 */
typedef Matrix_4<FloatType>		Matrix4;

// Type-specific OpenGL functions:
inline void glLoadMatrix(const Matrix_4<GLdouble>& tm) { glLoadMatrixd(tm.constData()); }
inline void glLoadMatrix(const Matrix_4<GLfloat>& tm) { glLoadMatrixf(tm.constData()); }

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::Matrix4)
Q_DECLARE_TYPEINFO(Ovito::Matrix4, Q_PRIMITIVE_TYPE);

#endif // __OVITO_MATRIX4_H
