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
 * \file
 * \brief This header file defines the default floating-point type and numeric constants used throughout the program.
 */

#ifndef __OVITO_FLOATTYPE_H
#define __OVITO_FLOATTYPE_H

namespace Ovito {

/// The default floating-point type used by OVITO.
///
typedef float FloatType;

#define FLOATTYPE_FLOAT		// This tells the program that we're using 32-bit floating-point numbers.

/// A small epsilon, which is used in OVITO to test if a number is (almost) zero.
#define FLOATTYPE_EPSILON	Ovito::FloatType(1e-6)

/// The maximum value for floating-point variables of type Ovito::FloatType.
#define FLOATTYPE_MAX	(std::numeric_limits<FloatType>::max())

/// The lowest value for floating-point variables of type Ovito::FloatType.
#define FLOATTYPE_MIN	(std::numeric_limits<FloatType>::lowest())

/// The constant PI.
#define FLOATTYPE_PI	Ovito::FloatType(3.14159265358979323846)

/// The format specifier to be passed to the sscanf() function to parse floating-point numbers of type Ovito::FloatType.
#define FLOATTYPE_SCANF_STRING 		"%g"

// Type-specific OpenGL functions:
inline void glVertex3(FloatType x, FloatType y, FloatType z) { glVertex3f(x,y,z); }
inline void glColor3(FloatType r, FloatType g, FloatType b) { glColor3f(r,g,b); }

}	// End of namespace

#endif // __OVITO_FLOATTYPE_H
