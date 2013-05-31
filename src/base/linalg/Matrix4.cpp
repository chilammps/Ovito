///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2008) Alexander Stukowski
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

#include <base/Base.h>
#include <base/linalg/Matrix4.h>
#include <base/utilities/Exception.h>

namespace Base {

// Computes the determinant of a 2x2 matrix. This is for internal use only.
#define det2x2(a, b, c, d) (a * d - b * c)

// Computes the determinant of a 3x3 matrix. This is for internal use only.
#define det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3)	\
		(a1 * det2x2( b2, b3, c2, c3 )				\
        - b1 * det2x2( a2, a3, c2, c3 )				\
        + c1 * det2x2( a2, a3, b2, b3 ))

/******************************************************************************
* Computes the inverse of the matrix. Throws an exception if matrix is not invertible.
******************************************************************************/
Matrix4 Matrix4::inverse() const 
{
	// Assign to individual variable names to aid
	// selecting correct values.
	const FloatType& a1 = m[0][0]; const FloatType& b1 = m[0][1]; 
	const FloatType& c1 = m[0][2]; const FloatType& d1 = m[0][3];
	const FloatType& a2 = m[1][0]; const FloatType& b2 = m[1][1]; 
	const FloatType& c2 = m[1][2]; const FloatType& d2 = m[1][3];
	const FloatType& a3 = m[2][0]; const FloatType& b3 = m[2][1];
	const FloatType& c3 = m[2][2]; const FloatType& d3 = m[2][3];
	const FloatType& a4 = m[3][0]; const FloatType& b4 = m[3][1]; 
	const FloatType& c4 = m[3][2]; const FloatType& d4 = m[3][3];

    Matrix4 mat(
			det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4),
		  - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4),
			det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4),
		  - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4),        

		  - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4),
			det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4),
		  - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4),
			det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4),
    
			det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4),
		  - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4),
			det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4),
		  - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4),
    
		  - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3),
			det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3),
		  - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3),
			det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3));

	FloatType det = determinant();
	if(det == 0.0) throw Exception("AffineTransformation cannot be inverted: Determinant is zero.");
	FloatType invdet = 1.0 / det;
    for(int i=0; i<4; i++)
		mat.m[i] *= invdet;
	return mat;
}

/******************************************************************************
* Generates a matrix that does perspective projection from a viewing frustum.
******************************************************************************/
Matrix4 Matrix4::frustum(FloatType left, FloatType right, FloatType bottom, FloatType top, FloatType znear, FloatType zfar)
{
	OVITO_ASSERT(zfar != znear);
	return Matrix4(2.0*znear/(right-left), 0.0, (right+left)/(right-left), 0.0,
					0.0, 2.0*znear/(top-bottom), (top+bottom)/(top-bottom), 0.0,
					0.0, 0.0, -(zfar+znear)/(zfar-znear), -(2.0*zfar*znear)/(zfar-znear),
					0.0, 0.0, -1.0, 0.0); 
}

/******************************************************************************
* Generates a matrix that does perspective projection.
******************************************************************************/
Matrix4 Matrix4::perspective(FloatType fovy, FloatType aspect, FloatType znear, FloatType zfar)
{
	FloatType f = tan(fovy*0.5);
	OVITO_ASSERT(zfar != znear);
	OVITO_ASSERT(f != 0.0);
	return Matrix4(1.0/(aspect*f), 0.0, 0.0, 0.0,
					0.0, 1.0/f, 0.0, 0.0,
					0.0, 0.0, -(zfar+znear)/(zfar-znear), -(2.0*zfar*znear)/(zfar-znear),
					0.0, 0.0, -1.0, 0.0); 
}

/******************************************************************************
* Generates a projection matrix for orthogonal projection.
******************************************************************************/
Matrix4 Matrix4::ortho(FloatType left, FloatType right, FloatType bottom, FloatType top, FloatType znear, FloatType zfar)
{
	OVITO_ASSERT(znear != zfar);
	return Matrix4(2.0/(right-left), 0.0,  0.0,  -(right+left)/(right-left),
					0.0, 2.0/(top-bottom), 0.0,  -(top+bottom)/(top-bottom),
					0.0, 0.0, -2.0/(zfar-znear), -(zfar+znear)/(zfar-znear),
					0.0, 0.0, 0.0, 1.0); 
}

/******************************************************************************
* Generates a rotation matrix around the X axis.
******************************************************************************/
Matrix4 Matrix4::rotationX(const FloatType angle)
{
	FloatType c = cos(angle);
	FloatType s = sin(angle);
	return Matrix4(1.0, 0.0, 0.0, 0.0,
				   0.0,   c,  -s, 0.0,
				   0.0,   s,   c, 0.0,
				   0.0, 0.0, 0.0, 1.0);
}

/******************************************************************************
* Generates a rotation matrix around the Y axis.
******************************************************************************/
Matrix4 Matrix4::rotationY(const FloatType angle)
{
	FloatType c = cos(angle);
	FloatType s = sin(angle);
	return Matrix4(  c, 0.0,   s, 0.0,
				   0.0, 1.0, 0.0, 0.0,
				    -s, 0.0,   c, 0.0,
				   0.0, 0.0, 0.0, 1.0);
}

/******************************************************************************
* Generates a rotation matrix around the Z axis.
******************************************************************************/
Matrix4 Matrix4::rotationZ(const FloatType angle)
{
	FloatType c = cos(angle);
	FloatType s = sin(angle);
	return Matrix4(  c,  -s, 0.0, 0.0,
				     s,   c, 0.0, 0.0,
				   0.0, 0.0, 1.0, 0.0,
				   0.0, 0.0, 0.0, 1.0);
}

/******************************************************************************
* Generates a rotation matrix around the given axis.
******************************************************************************/
Matrix4 Matrix4::rotation(const Rotation& rot)
{
	FloatType c = cos(rot.angle);
	FloatType s = sin(rot.angle);
	FloatType t = (FloatType)1 - c;
    const Vector3& a = rot.axis;
	return Matrix4(	t * a.X * a.X + c,       t * a.X * a.Y - s * a.Z, t * a.X * a.Z + s * a.Y, 0.0, 
					t * a.X * a.Y + s * a.Z, t * a.Y * a.Y + c,       t * a.Y * a.Z - s * a.X, 0.0,
					t * a.X * a.Z - s * a.Y, t * a.Y * a.Z + s * a.X, t * a.Z * a.Z + c      , 0.0,
					0.0                    , 0.0                    , 0.0                    , 1.0);
}

/******************************************************************************
* Generates a translation matrix.
******************************************************************************/
Matrix4 Matrix4::translation(const Vector3& t)
{
	return Matrix4(1.0, 0.0, 0.0, t.X,
				   0.0, 1.0, 0.0, t.Y,
				   0.0, 0.0, 1.0, t.Z,
				   0.0, 0.0, 0.0, 1.0);
}

/******************************************************************************
* Generates a matrix from an OpenGL transformation matrix stored in the given array.
******************************************************************************/
Matrix4 Matrix4::fromOpenGL(const FloatType tm[16])
{
	return Matrix4(
			tm[0], tm[4], tm[8], tm[12],
			tm[1], tm[5], tm[9], tm[13],
			tm[2], tm[6], tm[10], tm[14],
			tm[3], tm[7], tm[11], tm[15]);
}

};	// End of namespace Base
