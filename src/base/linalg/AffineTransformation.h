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
 * \brief Contains definition of Ovito::AffineTransformation class.
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
	typedef std::size_t size_type;

public:

	/// \brief Constructs a matrix without initializing its elements.
	/// \note All elements are left uninitialized by this constructor and have therefore an undefined value!
	Matrix_34() {}

	/// \brief Constructor that initializes 9 elements of the matrix to the given values. Translation is set to zero.
	/// \note Values are given in row-major order, i.e. row by row.
	constexpr Matrix_34(T m11, T m12, T m13,
					    T m21, T m22, T m23,
					    T m31, T m32, T m33) :
		_m{{m11,m21,m31},{m12,m22,m32},{m13,m23,m33},Vector_3<T>::Zero()} {}

	/// \brief Constructor that initializes all elements of the matrix to the given values.
	/// \note Values are given in row-major order, i.e. row by row.
	constexpr Matrix_34(T m11, T m12, T m13, T m14,
					    T m21, T m22, T m23, T m24,
					    T m31, T m32, T m33, T m34) :
		_m{{m11,m21,m31},{m12,m22,m32},{m13,m23,m33},{m14,m24,m34}} {}

	/// \brief Constructor that initializes the matrix from four column vectors.
	constexpr Matrix_34(const Vector_3<T>& c1, const Vector_3<T>& c2, const Vector_3<T>& c3, const Vector_3<T>& c4) : _m{c1, c2, c3, c4} {}

	/// \brief Initializes the matrix to the null matrix.
	/// All matrix elements are set to zero by this constructor.
	explicit constexpr Matrix_34(Zero) : _m{Vector_3<T>::Zero(), Vector_3<T>::Zero(), Vector_3<T>::Zero(), Vector_3<T>::Zero()} {}

	/// \brief Initializes the matrix to the identity matrix.
	/// All diagonal elements are set to one and all off-diagonal elements are set to zero.
	explicit constexpr Matrix_34(Identity) : _m{
		{T(1),T(0),T(0)},
		{T(0),T(1),T(0)},
		{T(0),T(0),T(1)},
		{T(0),T(0),T(0)}} {}

	/// \brief Initializes the 3x4 matrix from a 3x3 matrix.
	/// The translation component of the affine transformation is set to the null vector.
	explicit constexpr Matrix_34(const Matrix_3<T>& tm) : _m{tm.column(0), tm.column(1), tm.column(2), Vector_3<T>::Zero()} {}

	/// \brief Returns the number of rows in this matrix.
	constexpr size_type row_count() const { return 3; }

	/// \brief Returns the columns of rows in this matrix.
	constexpr size_type col_count() const { return 4; }

	/// \brief Returns the value of a matrix element.
	/// \param row The row of the element to return.
	/// \param col The column of the element to return.
	/// \return The value of the matrix element.
	inline constexpr const T& operator()(size_type row, size_type col) const {
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
	inline constexpr const Vector_3<T>& column(size_type col) const {
		return _m[col];
	}

	/// \brief Returns a reference to a column vector of the matrix.
	/// \param col The column to return.
	/// \return The i-th column of the matrix as a vector reference. Modifying the vector modifies the matrix.
	inline Vector_3<T>& column(size_type col) {
		return _m[col];
	}
	
	/// \brief Returns a row from the matrix.
	/// \param row The row to return.
	/// \return The i-th row of the matrix as a vector.
	constexpr Vector_4<T> row(size_type row) const {
		return { _m[0][row], _m[1][row], _m[2][row], _m[3][row] };
	}

	/// \brief Returns the translational part of this transformation matrix.
	/// \return A vector that specifies the translation.
	constexpr const Vector_3<T>& translation() const { return column(3); }

	/// \brief Returns a reference to the translational part of this transformation matrix.
	Vector_3<T>& translation() { return column(3); }

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
	Matrix_34& setIdentity(Identity) {
		_m[0][0] = T(1); _m[0][1] = T(0); _m[0][2] = T(0);
		_m[1][0] = T(0); _m[1][1] = T(1); _m[1][2] = T(0);
		_m[2][0] = T(0); _m[2][1] = T(0); _m[2][2] = T(1);
		_m[3][0] = T(0); _m[3][1] = T(0); _m[3][2] = T(0);
		return *this;
	}

	/// \brief Sets the matrix to the identity matrix.
	Matrix_34& operator=(Identity) {
		return setIdentity();
	}

	////////////////////////////////// Comparison ///////////////////////////////////

	/// \brief Computes the determinant of the matrix.
	constexpr inline T determinant() const {
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

	/// \brief Returns a pointer to the first element of the matrix.
	/// \return A pointer to 12 matrix elements in column-major order.
	/// \sa constData() 
	T* data() {
        OVITO_STATIC_ASSERT(sizeof(_m) == sizeof(T) * col_count() * row_count());
		return reinterpret_cast<T*>(&_m);
	}

	/// \brief Returns a pointer to the first element of the matrix.
	/// \return A pointer to 12 matrix elements in column-major order.
	/// \sa data() 
	const T* constData() const {
        OVITO_STATIC_ASSERT(sizeof(_m) == sizeof(T) * col_count() * row_count());
		return reinterpret_cast<const T*>(&_m);
	}
	
	/// \brief Generates a matrix describing a rotation around the X axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_34 rotationX(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {1, 0, 0, 0,
				0, c,-s, 0,
				0, s, c, 0};
	}

	/// \brief Generates a matrix describing a rotation around the Y axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_34 rotationY(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return { c, 0, s, 0,
				 0, 1, 0, 0,
				-s, 0, c, 0};
	}

	/// \brief Generates a matrix describing a rotation around the Z axis.
	/// \param angle The rotation angle in radians.
	static inline Matrix_34 rotationZ(T angle) {
		const T c = cos(angle);
		const T s = sin(angle);
		return {c, -s, 0, 0,
				s,  c, 0, 0,
				0,  0, 1, 0};
	}

	/// Generates a pure rotation matrix around the given axis.
	static Matrix_34 rotation(const RotationT<T>& rot);

	/// Generates a pure rotation matrix from a quaternion.
	static Matrix_34 rotation(const QuaternionT<T>& q);

	/// Generates a pure translation matrix.
	static Matrix_34 translation(const Vector_3<T>& translation);

	/// Generates a pure diagonal scaling matrix.
	static Matrix_34 scaling(T scalingFactor);

	/// Generates a pure scaling matrix.
	static Matrix_34 scaling(const ScalingT<T>& scaling);

	/// Generates a matrix with pure shearing transformation normal to the z-axis in the x- and y-direction.
	static Matrix_34 shear(T gammaX, T gammaY);

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
		auto zaxis = (camera - target).normalized();
		auto xaxis = upVector.cross(zaxis);
		if(xaxis == Vector_3<T>::Zero()) {
			xaxis = Vector_3<T>(0,1,0).cross(zaxis);
			if(xaxis == Vector_3<T>::Zero()) {
				xaxis = Vector_3<T>(0,0,1).cross(zaxis);
				OVITO_ASSERT(xaxis != Vector_3<T>::Zero());
			}
		}
		xaxis.normalize();
		auto yaxis = zaxis.cross(xaxis);

		return { xaxis.x(), xaxis.y(), xaxis.z(), -xaxis.dot(camera - Point_3<T>::Origin()),
				 yaxis.x(), yaxis.y(), yaxis.z(), -yaxis.dot(camera - Point_3<T>::Origin()),
				 zaxis.x(), zaxis.y(), zaxis.z(), -zaxis.dot(camera - Point_3<T>::Origin()) };
	}
	
	///////////////////////////////// Information ////////////////////////////////

	/// \brief Tests whether the matrix is a pure rotation matrix.
	/// \return \c If the matrix is a pure rotation matrix; \c false otherwise.
	///
	/// The matrix A is a pure rotation matrix if:
	///   (1) det(A) = 1  and
	///   (2) A * A^T = I
	constexpr bool isRotationMatrix(T epsilon = T(FLOATTYPE_EPSILON)) const {
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
};

/// Generates a pure rotation matrix around the given axis.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::rotation(const RotationT<T>& rot)
{
	T c = cos(rot.angle());
	T s = sin(rot.angle());
	T t = 1.0 - c;
    const auto& a = rot.axis();
	OVITO_ASSERT_MSG(std::abs(a.squaredLength() - T(1)) <= T(FLOATTYPE_EPSILON), "AffineTransformation::rotation", "Rotation axis vector must be normalized.");

	// Make sure the result is a pure rotation matrix.
#ifdef _DEBUG
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
#ifdef _DEBUG
	if(std::abs(q.dot(q) - T(1)) > T(FLOATTYPE_EPSILON))
		OVITO_ASSERT_MSG(false, "AffineTransformation::rotation(const Quaternion&)", "Quaternion must be normalized.");

	// Make sure the result is a pure rotation matrix.
	Matrix_34<T> tm(1.0 - 2.0*(q.y()*q.y() + q.z()*q.z()),       2.0*(q.x()*q.y() - q.w()*q.z()),       2.0*(q.x()*q.z() + q.w()*q.y()), 0.0,
	        2.0*(q.x()*q.y() + q.w()*q.z()), 1.0 - 2.0*(q.x()*q.x() + q.z()*q.z()),       2.0*(q.y()*q.z() - q.w()*q.x()), 0.0,
            2.0*(q.x()*q.z() - q.w()*q.y()),       2.0*(q.y()*q.z() + q.w()*q.x()), 1.0 - 2.0*(q.x()*q.x() + q.y()*q.y()), 0.0);
    OVITO_ASSERT_MSG(tm.isRotationMatrix(), "AffineTransformation::rotation(const Quaternion&)" , "Result is not a pure rotation matrix.");
#endif
	return Matrix_34<T>(1.0 - 2.0*(q.y()*q.y() + q.z()*q.z()),       2.0*(q.x()*q.y() - q.w()*q.z()),       2.0*(q.x()*q.z() + q.w()*q.y()), 0.0,
				        2.0*(q.x()*q.y() + q.w()*q.z()), 1.0 - 2.0*(q.x()*q.x() + q.z()*q.z()),       2.0*(q.y()*q.z() - q.w()*q.x()), 0.0,
			            2.0*(q.x()*q.z() - q.w()*q.y()),       2.0*(q.y()*q.z() + q.w()*q.x()), 1.0 - 2.0*(q.x()*q.x() + q.y()*q.y()), 0.0);
}

/// Generates a pure translation matrix.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::translation(const Vector_3<T>& t)
{
	return Matrix_34<T>(1.0, 0.0, 0.0, t.x(),
				  0.0, 1.0, 0.0, t.y(),
				  0.0, 0.0, 1.0, t.z());
}

/// Generates a pure diagonal scaling matrix.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::scaling(T s)
{
	return Matrix_34<T>(  s, 0.0, 0.0, 0.0,
				  	  	  0.0,   s, 0.0, 0.0,
				  	  	  0.0, 0.0,   s, 0.0);
}

/// Generates a pure scaling matrix.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::scaling(const ScalingT<T>& scaling)
{
	Matrix_3<T> U = Matrix_3<T>::rotation(scaling.Q);
	Matrix_3<T> K = Matrix_3<T>(scaling.S.x(), 0.0, 0.0,
								0.0, scaling.S.y(), 0.0,
								0.0, 0.0, scaling.S.z());
	return (U * K * U.transposed());
}

/// Generates a matrix with pure shearing transformation normal to the z-axis in the x- and y-direction.
template<typename T>
inline Matrix_34<T> Matrix_34<T>::shear(T gammaX, T gammaY)
{
	return AffineTransformation(1.0, 0.0, gammaX, 0.0,
				  0.0, 1.0, gammaY, 0.0,
				  0.0, 0.0, 1.0, 0.0);
}

/// Multiplies a 3x4 matrix with a Vector3 (which is automatically extended to a 4-vector with the last
/// element being 0).
template<typename T>
inline constexpr Vector_3<T> operator*(const Matrix_34<T>& m, const Vector_3<T>& v)
{
	return { m(0,0) * v[0] + m(0,1) * v[1] + m(0,2) * v[2],
			 m(1,0) * v[0] + m(1,1) * v[1] + m(1,2) * v[2],
			 m(2,0) * v[0] + m(2,1) * v[1] + m(2,2) * v[2] };
}

/// Multiplies a 3x4 matrix with a Point3 (which is extended to a 4-vector with the last
/// element being 1).
template<typename T>
inline constexpr Point_3<T> operator*(const Matrix_34<T>& m, const Point_3<T>& p)
{
	return { m(0,0) * p[0] + m(0,1) * p[1] + m(0,2) * p[2] + m(0,3),
			 m(1,0) * p[0] + m(1,1) * p[1] + m(1,2) * p[2] + m(1,3),
			 m(2,0) * p[0] + m(2,1) * p[1] + m(2,2) * p[2] + m(2,3) };
}

/// Computes the product of a 3x4 matrix with another 3x4 Matrix.
template<typename T>
inline Matrix_34<T> operator*(const Matrix_34<T>& a, const Matrix_34<T>& b)
{
	Matrix_34<T> res;
	for(typename Matrix_34<T>::size_type i = 0; i < 3; i++) {
		for(typename Matrix_34<T>::size_type j = 0; j < 4; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) += a(i,3);
	}
	return res;
}

/// Multiplies a 3x4 matrix with a scalar.
template<typename T>
constexpr inline Matrix_34<T> operator*(const Matrix_34<T>& a, T s)
{
	return { a.column(0)*s, a.column(1)*s, a.column(2)*s, a.column(3)*s };
}

/// Multiplies a 3x4 matrix with a scalar.
template<typename T>
constexpr inline Matrix_34<T> operator*(T s, const Matrix_34<T>& a)
{
	return a * s;
}

/// Computes the product of a 3x3 matrix and a 3x4 Matrix.
template<typename T>
inline Matrix_34<T> operator*(const Matrix_3<T>& a, const Matrix_34<T>& b)
{
	Matrix_34<T> res;
	for(typename Matrix_34<T>::size_type i = 0; i < 3; i++)
		for(typename Matrix_34<T>::size_type j = 0; j < 4; j++)
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
	return res;
}

/// Computes the product of a 3x4 matrix and a 3x3 matrix.
template<typename T>
inline Matrix_34<T> operator*(const Matrix_34<T>& a, const Matrix_3<T>& b)
{
	Matrix_34<T> res;
	for(typename Matrix_34<T>::size_type i = 0; i < 3; i++) {
		for(typename Matrix_34<T>::size_type j = 0; j < 3; j++) {
			res(i,j) = a(i,0)*b(0,j) + a(i,1)*b(1,j) + a(i,2)*b(2,j);
		}
		res(i,3) = a(i,3);
	}
	return res;
}

/// Writes the matrix to an output stream.
template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Matrix_34<T>& m) {
	for(typename Matrix_34<T>::size_type row = 0; row < m.row_count(); row++)
		os << m.row(row) << std::endl;
	return os;
}

/// \brief Writes a matrix to a binary output stream.
template<typename T>
inline SaveStream& operator<<(SaveStream& stream, const Matrix_34<T>& m)
{
	for(typename Matrix_34<T>::size_type col = 0; col < m.col_count(); col++)
		stream << m.col(col);
	return stream;
}

/// \brief Reads a matrix from a binary input stream.
template<typename T>
inline LoadStream& operator>>(LoadStream& stream, Matrix_34<T>& m)
{
	for(typename Matrix_34<T>::size_type col = 0; col < m.col_count(); col++)
		stream >> m.col(col);
	return stream;
}

/**
 * \fn typedef AffineTransformation
 * \brief Template class instance of the Matrix_34 class.
 */
typedef Matrix_34<FloatType>		AffineTransformation;

};	// End of namespace

Q_DECLARE_METATYPE(Ovito::AffineTransformation)
Q_DECLARE_TYPEINFO(Ovito::AffineTransformation, Q_PRIMITIVE_TYPE);

#endif // __OVITO_AFFINE_TRANSFORMATION_H
