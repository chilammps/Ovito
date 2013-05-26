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
 * \file FloatType.h
 * \brief This header file defines the floating-point data type used throughout the program.
 */

#ifndef __OVITO_FLOATTYPE_H
#define __OVITO_FLOATTYPE_H

namespace Ovito {

// Use single precision (32-bit) float as default floating point type.
typedef float FloatType;

#define FLOATTYPE_FLOAT		// This tells the program that we're using 32-bit floating point.

/// A small epsilon value for the FloatType.
#define FLOATTYPE_EPSILON	1e-6f

/// The maximum value for floating point variables.
#define FLOATTYPE_MAX	(std::numeric_limits<FloatType>::max())
#define FLOATTYPE_PI	Ovito::FloatType(3.14159265358979323846)

};

#endif // __OVITO_FLOATTYPE_H
