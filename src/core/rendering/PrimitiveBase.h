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

#ifndef __OVITO_PRIMITIVE_BASE_H
#define __OVITO_PRIMITIVE_BASE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Abstract base class for drawing primitives.
 */
class OVITO_CORE_EXPORT PrimitiveBase
{
public:

	/// \brief Virtual base destructor.
	virtual ~PrimitiveBase() {}

	/// \brief Returns true if the primitive buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the primitive using the given renderer.
	virtual void render(SceneRenderer* renderer) = 0;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PRIMITIVE_BASE_H
