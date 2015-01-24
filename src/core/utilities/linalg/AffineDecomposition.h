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
//  The matrix decomposition code has been taken from the book
//  Graphics Gems IV - Ken Shoemake, Polar AffineTransformation Decomposition. 
//
///////////////////////////////////////////////////////////////////////////////

/** 
 * \file
 * \brief Contains the definition of the Ovito::AffineDecomposition class.
 */

#ifndef __OVITO_AFFINE_DECOMP_H
#define __OVITO_AFFINE_DECOMP_H

#include <core/Core.h>
#include "Vector3.h"
#include "Rotation.h"
#include "Scaling.h"
#include "AffineTransformation.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Math)

/**
 * \brief Decomposes an affine transformation matrix into translation, rotation and scaling parts.
 * 
 * A AffineTransformation matrix is decomposed in the following way:
 * 
 * M = T * F * R * S
 * 
 * with 
 * \li T - Translation
 * \li F - Sign of determinant
 * \li R - Rotation
 * \li S - Scaling  
 * 
 * The scaling matrix is spectrally decomposed into S = U * K * U.transposed().
 * 
 * \note Decomposing a matrix into its affine parts is a slow operation and should only be done when really necessary.
 */
class OVITO_CORE_EXPORT AffineDecomposition
{
public:

	/// Translation part.
	Vector3 translation;

	/// Rotation part.
	Quaternion rotation;

	/// Scaling part.
	Scaling scaling;

	/// Sign of determinant (either -1.0 or +1.0).
	FloatType sign;	

	/// \brief Constructor that decomposes the given matrix into its affine parts.
	///
	/// After the constructor has been called the components of the decomposed
	/// transformation can be accessed through the #translation, #rotation,
	/// #scaling and #sign member variables.
	AffineDecomposition(const AffineTransformation& tm);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_AFFINE_DECOMP_H
