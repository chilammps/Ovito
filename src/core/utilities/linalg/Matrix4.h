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
 * \brief Contains the definition of the Ovito::Matrix_4 class template.
 */

#ifndef __OVITO_MATRIX4_H
#define __OVITO_MATRIX4_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include "Vector3.h"
#include "Point3.h"
#include "Matrix3.h"
#include "Vector4.h"
#include "AffineTransformation.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief A 4x4 matrix.
 *
 * In contrast to the AffineTransformation matrix class this Matrix4 class
 * can describe perspective projections.
 */
template<typename T>
class Matrix_4 : public std::array<Vector_4<T>,4>
{
public:

	/// An empty type that denotes a 4x4 matrix with all elements equal to zero.
	struct Zero {};

	/// An empty type that denotes the 4x4 identity matrix.
	struct Identity {};

	/// The type of a single element of the matrix.
	typedef T element_type;

	/// The type of a single column of the matrix.
	typedef Vector_4<T> column_type;

	using typename std::array<Vector_4<T>, 4>::size_type;
	using typename std::array<Vector_4<T>, 4>::difference_type;
	using typename std::array<Vector_4<T>, 4>::value_type;
	using typename std::array<Vector_4<T>, 4>::iterator;
	using typename std::array<Vector_4<T>, 4>::const_iterator;

public:

	/// \brief Empty default constructor that does not initialize the matrix elements (for performance reasons).
	///        The matrix elements will have an undefined value and need to be initialized later.
	Matrix_4() {}

	/// \brief Constructor that initializes 9 elements of the matrix to the given values. All other elements are set to zero.
	/// \note Values are given in row-major order, i.e. row by row.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(T m11, T m12, T m13,
					   T m21, T m22, T m23,
					   T m31, T m32, T m33)
		: std::array<Vector_4<T>,4>{{{m11,m21,m31,T(0)},{m12,m22,m32,T(0)},{m13,m23,m33,T(0)}, typename Vector_4<T>::Zero()}} {}
#else
	Matrix_4(T m11, T m12, T m13,
		   T m21, T m22, T m23,
		   T m31, T m32, T m33)
		{ (*this)[0] = Vector_4<T>(m11,m21,m31,T(0)); 
		  (*this)[1] = Vector_4<T>(m12,m22,m32,T(0));
		  (*this)[2] = Vector_4<T>(m13,m23,m33,T(0));
		  (*this)[3] = typename Vector_4<T>::Zero(); }
#endif

	/// \brief Constructor that initializes the 12 elements of the 3x4 submatrix to the given values.
	///        All other elements are set to zero.
	/// \note Elements need to be specified in row-major order, i.e. row by row.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(
						T m11, T m12, T m13, T m14,
						T m21, T m22, T m23, T m24,
						T m31, T m32, T m33, T m34)
		: std::array<Vector_4<T>,4>{{
			Vector_4<T>(m11,m21,m31,T(0)),
			Vector_4<T>(m12,m22,m32,T(0)),
			Vector_4<T>(m13,m23,m33,T(0)),
			Vector_4<T>(m14,m24,m34,T(0))}} {}
#else
	Matrix_4(
			T m11, T m12, T m13, T m14,
			T m21, T m22, T m23, T m24,
			T m31, T m32, T m33, T m34)
		{ (*this)[0] = Vector_4<T>(m11,m21,m31,T(0)); 
		  (*this)[1] = Vector_4<T>(m12,m22,m32,T(0));
		  (*this)[2] = Vector_4<T>(m13,m23,m33,T(0));
		  (*this)[3] = Vector_4<T>(m14,m24,m34,T(0)); }
#endif

	/// \brief Constructor that initializes 16 elements of the matrix to the given values.
	/// \note Elements need to be specified in row-major order, i.e. row by row.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(
						T m11, T m12, T m13, T m14,
						T m21, T m22, T m23, T m24,
						T m31, T m32, T m33, T m34,
						T m41, T m42, T m43, T m44)
		: std::array<Vector_4<T>,4>{{
			Vector_4<T>(m11,m21,m31,m41),
			Vector_4<T>(m12,m22,m32,m42),
			Vector_4<T>(m13,m23,m33,m43),
			Vector_4<T>(m14,m24,m34,m44)}} {}
#else
	Matrix_4(
			T m11, T m12, T m13, T m14,
			T m21, T m22, T m23, T m24,
			T m31, T m32, T m33, T m34,
			T m41, T m42, T m43, T m44)
		{ (*this)[0] = Vector_4<T>(m11,m21,m31,m41); 
		  (*this)[1] = Vector_4<T>(m12,m22,m32,m42);
		  (*this)[2] = Vector_4<T>(m13,m23,m33,m43);
		  (*this)[3] = Vector_4<T>(m14,m24,m34,m44); }
#endif

	/// \brief Constructor that initializes the matrix from four column vectors.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(const Vector_4<T>& c1, const Vector_4<T>& c2, const Vector_4<T>& c3, const Vector_4<T>& c4)
		: std::array<Vector_4<T>,4>{{c1, c2, c3, c4}} {}
#else
	Matrix_4(const Vector_4<T>& c1, const Vector_4<T>& c2, const Vector_4<T>& c3, const Vector_4<T>& c4)
		{ (*this)[0] = c1; (*this)[1] = c2; (*this)[2] = c3; (*this)[3] = c4; } 
#endif

	/// \brief Initializes the 4x4 matrix from a 3x4 matrix.
	/// The lower matrix row is initialized to (0,0,0,1).
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	explicit Q_DECL_CONSTEXPR Matrix_4(const AffineTransformationT<T>& tm)
		: std::array<Vector_4<T>,4>{{
			Vector_4<T>(tm(0,0),tm(1,0),tm(2,0),T(0)),
			Vector_4<T>(tm(0,1),tm(1,1),tm(2,1),T(0)),
			Vector_4<T>(tm(0,2),tm(1,2),tm(2,2),T(0)),
			Vector_4<T>(tm(0,3),tm(1,3),tm(2,3),T(1))}} {}
#else
	explicit Matrix_4(const AffineTransformationT<T>& tm)
		{ (*this)[0] = Vector_4<T>(tm(0,0),tm(1,0),tm(2,0),T(0)); 
		  (*this)[1] = Vector_4<T>(tm(0,1),tm(1,1),tm(2,1),T(0));
		  (*this)[2] = Vector_4<T>(tm(0,2),tm(1,2),tm(2,2),T(0));
		  (*this)[3] = Vector_4<T>(tm(0,3),tm(1,3),tm(2,3),T(1)); }
#endif

	/// \brief Constructor that initializes the top 3x4 submatrix from four column 3-vectors.
	/// The lower matrix row is initialized to (0,0,0,1).
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(const Vector_3<T>& c1, const Vector_3<T>& c2, const Vector_3<T>& c3, const Vector_3<T>& c4)
		: std::array<Vector_4<T>,4>{{
			Vector_4<T>(c1[0],c1[1],c1[2],T(0)),
			Vector_4<T>(c2[0],c2[1],c2[2],T(0)),
			Vector_4<T>(c3[0],c3[1],c3[2],T(0)),
			Vector_4<T>(c4[0],c4[1],c4[2],T(1))}} {}
#else
	Matrix_4(const Vector_3<T>& c1, const Vector_3<T>& c2, const Vector_3<T>& c3, const Vector_3<T>& c4)
		{ (*this)[0] = Vector_4<T>(c1[0],c1[1],c1[2],T(0)); 
		  (*this)[1] = Vector_4<T>(c2[0],c2[1],c2[2],T(0));
		  (*this)[2] = Vector_4<T>(c3[0],c3[1],c3[2],T(0));
		  (*this)[3] = Vector_4<T>(c4[0],c4[1],c4[2],T(1)); }
#endif

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(Zero)
		: std::array<Vector_4<T>,4>{{
			typename Vector_4<T>::Zero(),
			typename Vector_4<T>::Zero(),
			typename Vector_4<T>::Zero(),
			typename Vector_4<T>::Zero()}} {}
#else
	Matrix_4(Zero)
		{ this->fill(typename Vector_4<T>::Zero()); }
#endif

		/// \brief Initializes the matrix to the identity matrix.
		/// All diagonal elements are set to one, and all off-diagonal elements are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_4(Identity)
		: std::array<Vector_4<T>,4>{{
			Vector_4<T>(T(1),T(0),T(0),T(0)),
			Vector_4<T>(T(0),T(1),T(0),T(0)),
			Vector_4<T>(T(0),T(0),T(1),T(0)),
			Vector_4<T>(T(0),T(0),T(0),T(1))}} {}
#else
	Matrix_4(Identity)
		{ (*this)[0] = Vector_4<T>(T(1),T(0),T(0),T(0)); 
		  (*this)[1] = Vector_4<T>(T(0),T(1),T(0),T(0));
		  (*this)[2] = Vector_4<T>(T(0),T(0),T(1),T(0));
		  (*this)[3] = Vector_4<T>(T(0),T(0),T(0),T(1)); }
#endif

	/// \brief Returns the number of rows of this matrix.
	static Q_DECL_CONSTEXPR size_type row_count() { return 4; }

	/// \brief Returns the number of columns of this matrix.
	static Q_DECL_CONSTEXPR size_type col_count() { return 4; }

	/// \brief Returns the value of a matrix element.
	/// \param row The row of the element to return.
	/// \param col The column of the element to return.
	/// \return The value of the matrix element.
	inline Q_DECL_CONSTEXPR T operator()(size_type row, size_type col) const {
		return (*this)[col][row];
	}

	/// \brief Returns a reference to a matrix element.
	/// \param row The row of the element to return.
	/// \param col The column of the element to return.
	inline T& operator()(size_type row, size_type col) {
		return (*this)[col][row];
	}

	/// \brief Returns a column vector in the matrix.
	/// \param col The index of the column to return.
	/// \return The i-th column of the matrix as a vector.
	inline Q_DECL_CONSTEXPR const column_type& column(size_type col) const {
		return (*this)[col];
	}

	/// \brief Returns a reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return The i-th column of the matrix as a vector reference. Modifying the vector modifies the matrix.
	inline column_type& column(size_type col) {
		return (*this)[col];
	}
	
	/// \brief Returns a row from the matrix.
	/// \param row The row to return.
	/// \return The i-th row of the matrix as a vector.
	Q_DECL_CONSTEXPR Vector_4<T> row(size_type row) const {
		return { (*this)[0][row], (*this)[1][row], (*this)[2][row], (*this)[3][row] };
	}

	/// \brief Replaces a row of the matrix.
	/// \param row The row to replace.
	/// \param v The new row vector.
	void setRow(size_type row, const Vector_4<T>& v) {
		(*this)[0][row] = v[0];
		(*this)[1][row] = v[1];
		(*this)[2][row] = v[2];
		(*this)[3][row] = v[3];
	}

	/// Returns a pointer to the 16 elements of the matrix (stored in column-major order).
	const element_type* elements() const {
		OVITO_STATIC_ASSERT(sizeof(*this) == sizeof(element_type)*16);
		return column(0).data();
	}

	/// Returns a pointer to the 16 elements of the matrix (stored in column-major order).
	element_type* elements() {
		OVITO_STATIC_ASSERT(sizeof(*this) == sizeof(element_type)*16);
		return column(0).data();
	}

	/// \brief Sets all components of the matrix to zero.
	void setZero() {
		for(size_type i = 0; i < col_count(); i++)
			column(i).setZero();
	}

	/// \brief Sets all components of the matrix to zero.
	Matrix_4& operator=(Zero) {
		setZero();
		return *this;
	}

	/// \brief Sets the matrix to the identity matrix.
	void setIdentity() {
		(*this)[0][0] = T(1); (*this)[0][1] = T(0); (*this)[0][2] = T(0); (*this)[0][3] = T(0);
		(*this)[1][0] = T(0); (*this)[1][1] = T(1); (*this)[1][2] = T(0); (*this)[1][3] = T(0);
		(*this)[2][0] = T(0); (*this)[2][1] = T(0); (*this)[2][2] = T(1); (*this)[2][3] = T(0);
		(*this)[3][0] = T(0); (*this)[3][1] = T(0); (*this)[3][2] = T(0); (*this)[3][3] = T(1);
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_4& operator=(Identity) {
		setIdentity();
		return *this;
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Computes the determinant of the matrix.
	Q_DECL_CONSTEXPR inline T determinant() const {
		return ((*this)[0][3] * (*this)[1][2] * (*this)[2][1] * (*this)[3][0]-(*this)[0][2] * (*this)[1][3] * (*this)[2][1] * (*this)[3][0]-(*this)[0][3] * (*this)[1][1] * (*this)[2][2] * (*this)[3][0]+(*this)[0][1] * (*this)[1][3] * (*this)[2][2] * (*this)[3][0]+
				(*this)[0][2] * (*this)[1][1] * (*this)[2][3] * (*this)[3][0]-(*this)[0][1] * (*this)[1][2] * (*this)[2][3] * (*this)[3][0]-(*this)[0][3] * (*this)[1][2] * (*this)[2][0] * (*this)[3][1]+(*this)[0][2] * (*this)[1][3] * (*this)[2][0] * (*this)[3][1]+
				(*this)[0][3] * (*this)[1][0] * (*this)[2][2] * (*this)[3][1]-(*this)[0][0] * (*this)[1][3] * (*this)[2][2] * (*this)[3][1]-(*this)[0][2] * (*this)[1][0] * (*this)[2][3] * (*this)[3][1]+(*this)[0][0] * (*this)[1][2] * (*this)[2][3] * (*this)[3][1]+
				(*this)[0][3] * (*this)[1][1] * (*this)[2][0] * (*this)[3][2]-(*this)[0][1] * (*this)[1][3] * (*this)[2][0] * (*this)[3][2]-(*this)[0][3] * (*this)[1][0] * (*this)[2][1] * (*this)[3][2]+(*this)[0][0] * (*this)[1][3] * (*this)[2][1] * (*this)[3][2]+
				(*this)[0][1] * (*this)[1][0] * (*this)[2][3] * (*this)[3][2]-(*this)[0][0] * (*this)[1][1] * (*this)[2][3] * (*this)[3][2]-(*this)[0][2] * (*this)[1][1] * (*this)[2][0] * (*this)[3][3]+(*this)[0][1] * (*this)[1][2] * (*this)[2][0] * (*this)[3][3]+
				(*this)[0][2] * (*this)[1][0] * (*this)[2][1] * (*this)[3][3]-(*this)[0][0] * (*this)[1][2] * (*this)[2][1] * (*this)[3][3]-(*this)[0][1] * (*this)[1][0] * (*this)[2][2] * (*this)[3][3]+(*this)[0][0] * (*this)[1][1] * (*this)[2][2] * (*this)[3][3]);
	}

	/// \brief Computes the inverse of the matrix. 
	/// \throw Exception if matrix is not invertible because it is singular.
	Matrix_4 inverse() const {

		T det = determinant();
		OVITO_ASSERT_MSG(det != T(0), "Matrix4::inverse()", "Singular matrix cannot be inverted: Determinant is zero.");
		if(det == T(0))
			throw Exception("Matrix4 cannot be inverted: determinant is zero.");

		// Assign to individual variable names to aid
		// selecting correct values.
		const T a1 = (*this)[0][0]; const T b1 = (*this)[0][1];
		const T c1 = (*this)[0][2]; const T d1 = (*this)[0][3];
		const T a2 = (*this)[1][0]; const T b2 = (*this)[1][1];
		const T c2 = (*this)[1][2]; const T d2 = (*this)[1][3];
		const T a3 = (*this)[2][0]; const T b3 = (*this)[2][1];
		const T c3 = (*this)[2][2]; const T d3 = (*this)[2][3];
		const T a4 = (*this)[3][0]; const T b4 = (*this)[3][1];
		const T c4 = (*this)[3][2]; const T d4 = (*this)[3][3];

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

	/// \brief Generates a perspective projection matrix.
	static Matrix_4<T> frustum(T left, T right, T bottom, T top, T znear, T zfar) {
		OVITO_ASSERT(znear < zfar);
		return { T(2)*znear/(right-left), T(0),  (right + left) / (right - left), T(0),
				 T(0), T(2)*znear/(top-bottom), (top + bottom) / (top - bottom), T(0),
				 T(0), T(0), -(zfar + znear) / (zfar - znear), -(T(2)*zfar*znear)/(zfar - znear),
				 T(0), T(0), T(-1), T(0) };
	}


private:

	// Computes the determinant of a 2x2 sub-matrix. This is for internal use only.
	static Q_DECL_CONSTEXPR inline T det2x2(T a, T b, T c, T d) { return (a * d - b * c); }

	// Computes the determinant of a 3x3 sub-matrix. This is for internal use only.
	static Q_DECL_CONSTEXPR inline T det3x3(T a1, T a2, T a3, T b1, T b2, T b3, T c1, T c2, T c3) {
		return (a1 * det2x2( b2, b3, c2, c3 )
			- b1 * det2x2( a2, a3, c2, c3 )
	        + c1 * det2x2( a2, a3, b2, b3 ));
	}

};

/// \brief Computes the product of a 4x4 matrix and a Vector4.
/// \relates Matrix_4
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

/// \brief Computes the product of a 4x4 matrix and a Vector3 (which is assumed to be a 4-vector with the last element equal to 0).
/// \relates Matrix_4
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

/// \brief Computes the product of a 4x4 matrix and a Point3 (which is assumed to be a 4-vector with the last element equal to 1).
/// \relates Matrix_4
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

/// Computes the product of two 4x4 matrices.
/// \relates Matrix_4
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

/// Computes the product of a 4x4 matrix and a 3x4 Matrix.
/// \relates Matrix_4
template<typename T>
inline Matrix_4<T> operator*(const Matrix_4<T>& a, const AffineTransformationT<T>& b)
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
/// \relates Matrix_4
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_4<T> operator*(const Matrix_4<T>& a, T s)
{
	return { a.column(0)*s, a.column(1)*s, a.column(2)*s, a.column(3)*s };
}

/// Multiplies a 4x4 matrix with a scalar.
/// \relates Matrix_4
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_4<T> operator*(T s, const Matrix_4<T>& a)
{
	return a * s;
}

/// Prints a matrix to an output stream.
/// \relates Matrix_4
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Matrix_4<T>& m) {
	for(typename Matrix_4<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Prints a matrix to a Qt debug stream.
/// \relates Matrix_4
template<typename T>
inline QDebug operator<<(QDebug dbg, const Matrix_4<T>& m) {
	for(typename Matrix_4<T>::size_type row = 0; row < m.row_count(); row++)
		dbg.nospace() << m(row,0) << " " << m(row,1) << " " << m(row,2) << " " << m(row,3) << "\n";
    return dbg.space();
}

/// \brief Writes a matrix to a binary output stream.
/// \relates Matrix_4
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Matrix_4<T>& m)
{
	for(typename Matrix_4<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.col(col);
	return stream;
}

/// \brief Reads a matrix from a binary input stream.
/// \relates Matrix_4
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Matrix_4<T>& m)
{
	for(typename Matrix_4<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.col(col);
	return stream;
}

/// \brief Writes a matrix to a Qt data stream.
/// \relates Matrix_4
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Matrix_4<T>& m) {
	for(typename Matrix_4<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.column(col);
	return stream;
}

/// \brief Reads a matrix from a Qt data stream.
/// \relates Matrix_4
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Matrix_4<T>& m) {
	for(typename Matrix_4<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.column(col);
	return stream;
}

/**
 * \brief Instantiation of the Matrix_4 class template with the default floating-point type.
 * \relates Matrix_4
 */
typedef Matrix_4<FloatType>		Matrix4;

// Type-specific OpenGL functions:
inline void glLoadMatrix(const Matrix_4<GLdouble>& tm) { glLoadMatrixd(tm.elements()); }
inline void glLoadMatrix(const Matrix_4<GLfloat>& tm) { glLoadMatrixf(tm.elements()); }

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Matrix4);
Q_DECLARE_METATYPE(Ovito::Matrix4*);
Q_DECLARE_TYPEINFO(Ovito::Matrix4, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Matrix4*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_MATRIX4_H
