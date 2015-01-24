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
 * \brief Contains the definition of the Ovito::Matrix_3 class template.
 */
 
#ifndef __OVITO_MATRIX3_H
#define __OVITO_MATRIX3_H

#include <core/Core.h>
#include <core/utilities/io/SaveStream.h>
#include <core/utilities/io/LoadStream.h>
#include <core/utilities/Exception.h>
#include "Vector3.h"
#include "Point3.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

template<typename T> class RotationT;
template<typename T> class QuaternionT;
template<typename T> class ScalingT;

/**
 * \brief A 3x3 matrix.
 *
 * The matrix is stored in column-major order. Matrix_3 is derived from std::array< Vector_3<T>, 3 >. Thus, it is an array of three column vectors.
 *
 * The template parameter \a T specifies the data type of the matrix elements.
 * The typedef \c Matrix3 for matrices with floating-point elements is predefined:
 *
 * \code
 *      typedef Matrix_3<FloatType>  Matrix3;
 * \endcode
 *
 * \sa Vector_3, Point_3
 * \sa AffineTransformationT, Matrix_4
 */
template<typename T>
class Matrix_3 : public std::array<Vector_3<T>,3>
{
public:

	/// An empty type that denotes a 3x3 matrix with all elements equal to zero.
	struct Zero {};

	/// An empty type that denotes the 3x3 identity matrix.
	struct Identity {};

	/// The type of a single element of the matrix.
	typedef T element_type;

	/// The type of a single column of the matrix.
	typedef Vector_3<T> column_type;

	using typename std::array<Vector_3<T>, 3>::size_type;
	using typename std::array<Vector_3<T>, 3>::difference_type;
	using typename std::array<Vector_3<T>, 3>::value_type;
	using typename std::array<Vector_3<T>, 3>::iterator;
	using typename std::array<Vector_3<T>, 3>::const_iterator;

	enum EulerAxisSequence {
		sxyz, sxyx, sxzy, sxzx, syzx, syzy, syxz, syxy, szxy, szxz, szyx, szyz,
		rzyx, rxyx, ryzx, rxzx, rxzy, ryzy, rzxy, ryxy, ryxz, rzxz, rxyz, rzyz
	};

public:

	/// \brief Empty default constructor that does not initialize the matrix elements (for performance reasons).
	///        The matrix elements will have an undefined value and need to be initialized later.
	Matrix_3() {}

	/// \brief Constructor that initializes all 9 elements of the matrix to the given values.
	/// \note Values are given in row-major order, i.e. row by row.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_3(T m11, T m12, T m13,
					   T m21, T m22, T m23,
					   T m31, T m32, T m33)
		: std::array<Vector_3<T>,3>{{Vector_3<T>(m11,m21,m31),
									 Vector_3<T>(m12,m22,m32),
									 Vector_3<T>(m13,m23,m33)}} {}
#else
	Matrix_3(T m11, T m12, T m13,
		   T m21, T m22, T m23,
		   T m31, T m32, T m33)
		{ (*this)[0] = Vector_3<T>(m11,m21,m31); 
		  (*this)[1] = Vector_3<T>(m12,m22,m32);
		  (*this)[2] = Vector_3<T>(m13,m23,m33); }
#endif

	/// \brief Constructor that initializes the matrix from three column vectors.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_3(const column_type& c1, const column_type& c2, const column_type& c3)
		: std::array<Vector_3<T>,3>{{c1, c2, c3}} {}
#else
	Matrix_3(const column_type& c1, const column_type& c2, const column_type& c3)
		{ (*this)[0] = c1; (*this)[1] = c2; (*this)[2] = c3; } 
#endif

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_3(Zero)
		: std::array<Vector_3<T>,3>{{typename Vector_3<T>::Zero(), typename Vector_3<T>::Zero(), typename Vector_3<T>::Zero()}} {}
#else
	Matrix_3(Zero)
		{ this->fill(typename Vector_3<T>::Zero()); }
#endif

	/// \brief Initializes the matrix to the identity matrix.
	/// All diagonal elements are set to one, and all off-diagonal elements are set to zero.
#if !defined(Q_CC_MSVC) && !defined(ONLY_FOR_DOXYGEN) // The MSVC compiler and the Doxygen parser do not like C++11 array aggregate initializers.
	Q_DECL_CONSTEXPR Matrix_3(Identity)
		: std::array<Vector_3<T>,3>{{Vector_3<T>(T(1),T(0),T(0)),
									 Vector_3<T>(T(0),T(1),T(0)),
									 Vector_3<T>(T(0),T(0),T(1))}} {}
#else
	Matrix_3(Identity)
		{ (*this)[0] = Vector_3<T>(T(1),T(0),T(0)); 
		  (*this)[1] = Vector_3<T>(T(0),T(1),T(0));
		  (*this)[2] = Vector_3<T>(T(0),T(0),T(1)); }
#endif

	/// \brief Casts the matrix to a matrix with another data type.
	template<typename U>
	Q_DECL_CONSTEXPR explicit operator Matrix_3<U>() const {
		return Matrix_3<U>(
				static_cast<U>((*this)(0,0)), static_cast<U>((*this)(0,1)), static_cast<U>((*this)(0,2)),
				static_cast<U>((*this)(1,0)), static_cast<U>((*this)(1,1)), static_cast<U>((*this)(1,2)),
				static_cast<U>((*this)(2,0)), static_cast<U>((*this)(2,1)), static_cast<U>((*this)(2,2)));
	}

	/// \brief Returns the number of rows of this matrix.
	static Q_DECL_CONSTEXPR size_type row_count() { return 3; }

	/// \brief Returns the number of columns of this matrix.
	static Q_DECL_CONSTEXPR size_type col_count() { return 3; }

	/// \brief Returns the value of a matrix element.
	/// \param row The matrix row of the element.
	/// \param col The matrix column of the element.
	/// \return The value of the matrix element.
	Q_DECL_CONSTEXPR inline T operator()(size_type row, size_type col) const {
		return (*this)[col][row];
	}

	/// \brief Returns a reference to a matrix element.
	/// \param row The row of the element.
	/// \param col The column of the element.
	/// \return A non-const reference to the matrix element, which may be written to.
	inline T& operator()(size_type row, size_type col) {
		return (*this)[col][row];
	}

	/// \brief Returns a column vector of the matrix.
	/// \param col The index of the column to return.
	/// \return A vector containing the matrix elements of the column \a col.
	Q_DECL_CONSTEXPR const column_type& column(size_type col) const {
		return (*this)[col];
	}

	/// \brief Returns a reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return A reference to the vector containing the matrix elements of the column \a col.
	/// \note Modifying the elements of the returned vector will modify the matrix elements.
	/// \sa row()
	column_type& column(size_type col) {
		return (*this)[col];
	}
	
	/// \brief Returns a row from the matrix.
	/// \param row The row to return.
	/// \return The requested row of the matrix as a vector.
	/// \note The returned vector is a copy of the matrix row elements.
	/// \sa column()
	Q_DECL_CONSTEXPR Vector_3<T> row(size_type row) const {
		return { (*this)[0][row], (*this)[1][row], (*this)[2][row] };
	}

	/// Returns a pointer to the 9 elements of the matrix (stored in column-major order).
	const element_type* elements() const {
		OVITO_STATIC_ASSERT(sizeof(*this) == sizeof(element_type)*9);
		return column(0).data();
	}

	/// Returns a pointer to the 9 elements of the matrix (stored in column-major order).
	element_type* elements() {
		OVITO_STATIC_ASSERT(sizeof(*this) == sizeof(element_type)*9);
		return column(0).data();
	}

	/// Sets all elements of the matrix to zero.
	void setZero() {
		(*this)[0].setZero();
		(*this)[1].setZero();
		(*this)[2].setZero();
	}

	/// Sets all elements of the matrix to zero.
	Matrix_3& operator=(Zero) {
		setZero();
		return *this;
	}

	/// Sets the matrix to the identity matrix.
	void setIdentity() {
		(*this)[0] = Vector_3<T>(1,0,0);
		(*this)[1] = Vector_3<T>(0,1,0);
		(*this)[2] = Vector_3<T>(0,0,1);
	}

	/// Sets the matrix to the identity matrix.
	Matrix_3& operator=(Identity) {
		setIdentity();
		return *this;
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Compares two matrices for exact equality.
	/// \return \c true if all elements are equal; \c false otherwise.
	Q_DECL_CONSTEXPR bool operator==(const Matrix_3& b) const {
		return (b[0] == (*this)[0]) && (b[1] == (*this)[1]) && (b[2] == (*this)[2]);
	}

	/// \brief Compares two matrices for inequality.
	/// \return \c true if not all elements are equal; \c false if all are equal.
	Q_DECL_CONSTEXPR bool operator!=(const Matrix_3& b) const {
		return !(*this == b);
	}

	////////////////////////////////// Computations ///////////////////////////////////

	/// Computes the inverse of the matrix.
	/// \throw Exception if matrix is not invertible because it is singular.
	/// \sa determinant()
	Matrix_3 inverse() const {
		T det = determinant();
		OVITO_ASSERT_MSG(det != T(0), "Matrix3::inverse()", "Singular matrix cannot be inverted: Determinant is zero.");
		if(det == 0) throw Exception("Matrix3 cannot be inverted: determinant is zero.");
		return Matrix_3(((*this)[1][1]*(*this)[2][2] - (*this)[1][2]*(*this)[2][1])/det,
						((*this)[2][0]*(*this)[1][2] - (*this)[1][0]*(*this)[2][2])/det,
						((*this)[1][0]*(*this)[2][1] - (*this)[1][1]*(*this)[2][0])/det,
						((*this)[2][1]*(*this)[0][2] - (*this)[0][1]*(*this)[2][2])/det,
						((*this)[0][0]*(*this)[2][2] - (*this)[2][0]*(*this)[0][2])/det,
						((*this)[0][1]*(*this)[2][0] - (*this)[0][0]*(*this)[2][1])/det,
						((*this)[0][1]*(*this)[1][2] - (*this)[1][1]*(*this)[0][2])/det,
						((*this)[0][2]*(*this)[1][0] - (*this)[0][0]*(*this)[1][2])/det,
						((*this)[0][0]*(*this)[1][1] - (*this)[1][0]*(*this)[0][1])/det);
	}

	/// \brief Computes the inverse of the matrix.
	/// \param result A reference to an output matrix that will receive the computed inverse.
	/// \param epsilon A threshold that is used to determine if the matrix is invertible. The matrix is considered singular if |det|<=epsilon.
	/// \return \c false if the matrix is not invertible because it is singular; \c true if the inverse has been calculated
	///         and was stored in \a result.
	/// \sa determinant()
	bool inverse(Matrix_3& result, T epsilon = T(FLOATTYPE_EPSILON)) const {
		T det = determinant();
		if(std::abs(det) <= epsilon) return false;
		result = Matrix_3(((*this)[1][1]*(*this)[2][2] - (*this)[1][2]*(*this)[2][1])/det,
						((*this)[2][0]*(*this)[1][2] - (*this)[1][0]*(*this)[2][2])/det,
						((*this)[1][0]*(*this)[2][1] - (*this)[1][1]*(*this)[2][0])/det,
						((*this)[2][1]*(*this)[0][2] - (*this)[0][1]*(*this)[2][2])/det,
						((*this)[0][0]*(*this)[2][2] - (*this)[2][0]*(*this)[0][2])/det,
						((*this)[0][1]*(*this)[2][0] - (*this)[0][0]*(*this)[2][1])/det,
						((*this)[0][1]*(*this)[1][2] - (*this)[1][1]*(*this)[0][2])/det,
						((*this)[0][2]*(*this)[1][0] - (*this)[0][0]*(*this)[1][2])/det,
						((*this)[0][0]*(*this)[1][1] - (*this)[1][0]*(*this)[0][1])/det);
		return true;
	}

	/// Calculates the determinant of the matrix.
	Q_DECL_CONSTEXPR inline T determinant() const {
		return(((*this)[0][0]*(*this)[1][1] - (*this)[0][1]*(*this)[1][0])*((*this)[2][2])
			  -((*this)[0][0]*(*this)[1][2] - (*this)[0][2]*(*this)[1][0])*((*this)[2][1])
			  +((*this)[0][1]*(*this)[1][2] - (*this)[0][2]*(*this)[1][1])*((*this)[2][0]));
	}

	/// Returns the transpose of the matrix.
	Q_DECL_CONSTEXPR Matrix_3 transposed() const {
		return Matrix_3((*this)[0][0], (*this)[0][1], (*this)[0][2],
						(*this)[1][0], (*this)[1][1], (*this)[1][2],
						(*this)[2][0], (*this)[2][1], (*this)[2][2]);
	}

	/// Computes the product of the matrix and a point and returns one coordinate of the resulting point.
	/// \param p The point to transform with the matrix.
	/// \param index The component (0-2) of the transformed point to return.
	/// \return ((*this)*p)[index]
	inline Q_DECL_CONSTEXPR T prodrow(const Point_3<T>& p, typename Point_3<T>::size_type index) const {
		return (*this)[0][index] * p[0] + (*this)[1][index] * p[1] + (*this)[2][index] * p[2];
	}

	/// Computes the product of the matrix and a vector and returns one component of the resulting vector.
	/// \param v The vector to transform with the matrix.
	/// \param index The component (0-2) of the transformed vector to return.
	/// \return ((*this)*v)[index]
	inline Q_DECL_CONSTEXPR T prodrow(const Vector_3<T>& v, typename Vector_3<T>::size_type index) const {
		return (*this)[0][index] * v[0] + (*this)[1][index] * v[1] + (*this)[2][index] * v[2];
	}

	/// \brief Tests whether the matrix is a pure rotation matrix.
	/// \return \c true if the matrix is a pure rotation matrix; \c false otherwise.
	///
	/// The matrix A is a pure rotation matrix if:
	///   1. det(A) = 1  and
	///   2. A * A^T = I
	Q_DECL_CONSTEXPR bool isRotationMatrix(T epsilon = T(FLOATTYPE_EPSILON)) const {
		return
			(std::abs((*this)[0][0]*(*this)[1][0] + (*this)[0][1]*(*this)[1][1] + (*this)[0][2]*(*this)[1][2]) <= epsilon) &&
			(std::abs((*this)[0][0]*(*this)[2][0] + (*this)[0][1]*(*this)[2][1] + (*this)[0][2]*(*this)[2][2]) <= epsilon) &&
			(std::abs((*this)[1][0]*(*this)[2][0] + (*this)[1][1]*(*this)[2][1] + (*this)[1][2]*(*this)[2][2]) <= epsilon) &&
			(std::abs((*this)[0][0]*(*this)[0][0] + (*this)[0][1]*(*this)[0][1] + (*this)[0][2]*(*this)[0][2] - T(1)) <= epsilon) &&
			(std::abs((*this)[1][0]*(*this)[1][0] + (*this)[1][1]*(*this)[1][1] + (*this)[1][2]*(*this)[1][2] - T(1)) <= epsilon) &&
			(std::abs((*this)[2][0]*(*this)[2][0] + (*this)[2][1]*(*this)[2][1] + (*this)[2][2]*(*this)[2][2] - T(1)) <= epsilon) &&
			(std::abs(determinant() - T(1)) <= epsilon);
	}

	/// \brief Converts this matrix to a Qt matrix.
	operator QMatrix3x3() const {
		QMatrix3x3 qtm;
		for(size_type row = 0; row < 3; row++)
			for(size_type col = 0; col < 3; col++)
				qtm(row,col) = (*this)(row,col);
		return qtm;
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

	/// \brief Calculates the Euler angles from a rotation matrix.
	/// \param axisSequence Selects the order of rotations around the X, Y, and Z axes.
	/// \return A 3-vector containing the three Euler angles.
	Vector_3<T> toEuler(EulerAxisSequence axisSequence) const;

	////////////////////////////////// Generation ///////////////////////////////////

	/// \brief Generates a matrix describing a rotation around the X axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_3 rotationX(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {T(1), T(0), T(0),
				T(0), c,   -s,
				T(0), s,    c};
	}

	/// \brief Generates a matrix describing a rotation around the Y axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_3 rotationY(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return { c,    T(0), s,
				 T(0), T(1), T(0),
			    -s,    T(0), c};
	}

	/// \brief Generates a matrix describing a rotation around the Z axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_3 rotationZ(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {c,    -s,    T(0),
				s,     c,    T(0),
				T(0),  T(0), T(1)};
	}

	/// \brief Generates a rotation matrix from an axis-angle representation.
	static Matrix_3 rotation(const RotationT<T>& rot);

	/// \brief Generates a rotation matrix from a quaternion.
	static Matrix_3 rotation(const QuaternionT<T>& q);

	/// \brief Generates a rotation matrix from Euler angles.
	/// \param ai The first Euler angle.
	/// \param aj The second Euler angle.
	/// \param ak The third Euler angle.
	/// \param axisSequence Determines the order in which the rotations about the three axes are performed.
	static Matrix_3 rotation(T ai, T aj, T ak, EulerAxisSequence axisSequence);

	/// \brief Generates a scaling matrix.
	static Matrix_3 scaling(const ScalingT<T>& scaling);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#include "Quaternion.h"
#include "Scaling.h"
#include "Rotation.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

// Generates a rotation matrix from an axis and an angle.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::rotation(const RotationT<T>& rot)
{
	if(rot.angle() == T(0))
		return Matrix_3<T>::Identity();
	T c = cos(rot.angle());
	T s = sin(rot.angle());
	T t = T(1) - c;
	const auto& a = rot.axis();
	OVITO_ASSERT_MSG(std::abs(a.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON), "Matrix3::rotation", "Rotation axis vector must be normalized.");
	return Matrix_3<T>(	t * a.x() * a.x() + c,       t * a.x() * a.y() - s * a.z(), t * a.x() * a.z() + s * a.y(),
					t * a.x() * a.y() + s * a.z(), t * a.y() * a.y() + c,       t * a.y() * a.z() - s * a.x(),
					t * a.x() * a.z() - s * a.y(), t * a.y() * a.z() + s * a.x(), t * a.z() * a.z() + c       );
}

// Generates a rotation matrix from a quaternion.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::rotation(const QuaternionT<T>& q)
{
#ifdef OVITO_DEBUG
	if(std::abs(q.dot(q) - T(1)) > T(FLOATTYPE_EPSILON)) {
		OVITO_ASSERT_MSG(false, "Matrix3::rotation", "Quaternion must be normalized.");
	}
#endif
	if(std::abs(q.w()) >= T(1))
		return Matrix_3<T>::Identity();
	return Matrix_3<T>(T(1) - T(2)*(q.y()*q.y() + q.z()*q.z()),       T(2)*(q.x()*q.y() - q.w()*q.z()),       T(2)*(q.x()*q.z() + q.w()*q.y()),
						T(2)*(q.x()*q.y() + q.w()*q.z()), T(1) - T(2)*(q.x()*q.x() + q.z()*q.z()),       T(2)*(q.y()*q.z() - q.w()*q.x()),
						T(2)*(q.x()*q.z() - q.w()*q.y()),       T(2)*(q.y()*q.z() + q.w()*q.x()), T(1) - T(2)*(q.x()*q.x() + q.y()*q.y()));
}

// Generates a rotation matrix from Euler angles and an axis sequence.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::rotation(T ai, T aj, T ak, EulerAxisSequence axisSequence)
{
	OVITO_ASSERT(axisSequence == Matrix_3<T>::szyx);
	int firstaxis = 2;
	int parity = 1;
	bool repetition = false;
	bool frame = false;

	int i = firstaxis;
	int j = (i + parity + 1) % 3;
	int k = (i - parity + 2) % 3;

	if(frame)
		std::swap(ai, ak);
	if(parity) {
		ai = -ai;
		aj = -aj;
		ak = -ak;
	}

	T si = std::sin(ai), sj = std::sin(aj), sk = std::sin(ak);
	T ci = std::cos(ai), cj = std::cos(aj), ck = std::cos(ak);
	T cc = ci*ck, cs = ci*sk;
	T sc = si*ck, ss = si*sk;

	Matrix_3<T> M;
	if(repetition) {
		M(i, i) = cj;
		M(i, j) = sj*si;
		M(i, k) = sj*ci;
		M(j, i) = sj*sk;
		M(j, j) = -cj*ss+cc;
		M(j, k) = -cj*cs-sc;
		M(k, i) = -sj*ck;
		M(k, j) = cj*sc+cs;
		M(k, k) = cj*cc-ss;
	}
	else {
		M(i, i) = cj*ck;
		M(i, j) = sj*sc-cs;
		M(i, k) = sj*cc+ss;
		M(j, i) = cj*sk;
		M(j, j) = sj*ss+cc;
		M(j, k) = sj*cs-sc;
		M(k, i) = -sj;
		M(k, j) = cj*si;
		M(k, k) = cj*ci;
	}
	return M;
}

// Returns the Euler angles from a rotation matrix.
template<typename T>
inline Vector_3<T> Matrix_3<T>::toEuler(EulerAxisSequence axisSequence) const
{
	OVITO_ASSERT(axisSequence == Matrix_3<T>::szyx);
	int firstaxis = 2;
	int parity = 1;
	bool repetition = false;
	bool frame = false;

	int i = firstaxis;
	int j = (i + parity + 1) % 3;
	int k = (i - parity + 2) % 3;

	T ax, ay, az;
	const Matrix_3<T>& M = *this;
    if(repetition) {
        T sy = std::sqrt(M(i, j)*M(i, j) + M(i, k)*M(i, k));
        if(sy > T(FLOATTYPE_EPSILON)) {
            ax = std::atan2( M(i, j),  M(i, k));
            ay = std::atan2( sy,       M(i, i));
            az = std::atan2( M(j, i), -M(k, i));
        }
        else {
            ax = std::atan2(-M(j, k),  M(j, j));
            ay = std::atan2( sy,       M(i, i));
            az = 0;
        }
    }
    else {
        T cy = std::sqrt(M(i, i)*M(i, i) + M(j, i)*M(j, i));
        if(cy > T(FLOATTYPE_EPSILON)) {
            ax = std::atan2( M(k, j),  M(k, k));
            ay = std::atan2(-M(k, i),  cy);
            az = std::atan2( M(j, i),  M(i, i));
        }
        else {
            ax = std::atan2(-M(j, k),  M(j, j));
            ay = std::atan2(-M(k, i),  cy);
            az = 0;
        }
    }

    if(parity) {
    	ax = -ax;
    	ay = -ay;
    	az = -az;
    }
    if(frame)
    	std::swap(ax, az);
    return Vector_3<T>(ax, ay, az);
}

// Creates a transformation matrix that describes a scaling of coordinates.
template<typename T>
inline Matrix_3<T> Matrix_3<T>::scaling(const ScalingT<T>& scaling)
{
	Matrix_3<T> K(scaling.S.x(), T(0), T(0),
				  T(0), scaling.S.y(), T(0),
				  T(0), T(0), scaling.S.z());
	if(std::abs(scaling.Q.w()) >= T(1))
		return K;
	Matrix_3<T> U = Matrix_3<T>::rotation(scaling.Q);
	return U * K * U.transposed();
}

/// \brief Computes the product of a matrix and a vector.
/// \relates Matrix_3
template<typename T>
Q_DECL_CONSTEXPR inline Vector_3<T> operator*(const Matrix_3<T>& m, const Vector_3<T>& v)
{
	return { m(0,0)*v[0] + m(0,1)*v[1] + m(0,2)*v[2],
			 m(1,0)*v[0] + m(1,1)*v[1] + m(1,2)*v[2],
			 m(2,0)*v[0] + m(2,1)*v[1] + m(2,2)*v[2] };
}

/// \brief Computes the product of a matrix and a point. This is the same as a matrix-vector product.
/// \relates Matrix_3
template<typename T>
Q_DECL_CONSTEXPR inline Point_3<T> operator*(const Matrix_3<T>& m, const Point_3<T>& p)
{
	return { m(0,0)*p[0] + m(0,1)*p[1] + m(0,2)*p[2],
			 m(1,0)*p[0] + m(1,1)*p[1] + m(1,2)*p[2],
			 m(2,0)*p[0] + m(2,1)*p[1] + m(2,2)*p[2] };
}

/// \brief Computes the product of two matrices.
/// \relates Matrix_3
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_3<T> operator*(const Matrix_3<T>& a, const Matrix_3<T>& b)
{
#if 1
	return Matrix_3<T>(
			a(0,0)*b(0,0) + a(0,1)*b(1,0) + a(0,2)*b(2,0),
			a(0,0)*b(0,1) + a(0,1)*b(1,1) + a(0,2)*b(2,1),
			a(0,0)*b(0,2) + a(0,1)*b(1,2) + a(0,2)*b(2,2),

			a(1,0)*b(0,0) + a(1,1)*b(1,0) + a(1,2)*b(2,0),
			a(1,0)*b(0,1) + a(1,1)*b(1,1) + a(1,2)*b(2,1),
			a(1,0)*b(0,2) + a(1,1)*b(1,2) + a(1,2)*b(2,2),

			a(2,0)*b(0,0) + a(2,1)*b(1,0) + a(2,2)*b(2,0),
			a(2,0)*b(0,1) + a(2,1)*b(1,1) + a(2,2)*b(2,1),
			a(2,0)*b(0,2) + a(2,1)*b(1,2) + a(2,2)*b(2,2)
	);
#else
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
#endif
}

/// \brief Multiplies a matrix with a scalar value.
/// \relates Matrix_3
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_3<T> operator*(const Matrix_3<T>& a, T s)
{
#if 1
	return Matrix_3<T>(
			a(0,0)*s, a(0,1)*s, a(0,2)*s,
			a(1,0)*s, a(1,1)*s, a(1,2)*s,
			a(2,0)*s, a(2,1)*s, a(2,2)*s
	);
#else
	Matrix_3<T> m;
	for(typename Matrix_3<T>::size_type i = 0; i < 3; i++) {
		for(typename Matrix_3<T>::size_type j = 0; j < 3; j++) {
			m(i, j) = a(i, j) * s;
		}
	}
	return m;
#endif
}

/// \brief Multiplies a matrix with a scalar value.
/// \relates Matrix_3
template<typename T>
Q_DECL_CONSTEXPR inline Matrix_3<T> operator*(T s, const Matrix_3<T>& a) {
	return a * s;
}

/// \brief Prints a matrix to a text output stream.
/// \relates Matrix_3
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Matrix_3<T>& m) {
	for(typename Matrix_3<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Prints a matrix to a Qt debug stream.
/// \relates Matrix_3
template<typename T>
inline QDebug operator<<(QDebug dbg, const Matrix_3<T>& m) {
	for(typename Matrix_3<T>::size_type row = 0; row < m.row_count(); row++)
		dbg.nospace() << m(row,0) << " " << m(row,1) << " " << m(row,2) << "\n";
    return dbg.space();
}

/// \brief Writes a matrix to a Qt data stream.
/// \relates Matrix_3
template<typename T>
inline QDataStream& operator<<(QDataStream& stream, const Matrix_3<T>& m) {
	for(typename Matrix_3<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.column(col);
	return stream;
}

/// \brief Reads a matrix from a Qt data stream.
/// \relates Matrix_3
template<typename T>
inline QDataStream& operator>>(QDataStream& stream, Matrix_3<T>& m) {
	for(typename Matrix_3<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.column(col);
	return stream;
}

/**
 * \brief Instantiation of the Matrix_3 class template with the default floating-point type.
 * \relates Matrix_3
 */
typedef Matrix_3<FloatType>		Matrix3;

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Matrix3);
Q_DECLARE_METATYPE(Ovito::Matrix3*);
Q_DECLARE_TYPEINFO(Ovito::Matrix3, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::Matrix3*, Q_PRIMITIVE_TYPE);

#endif // __OVITO_MATRIX3_H
