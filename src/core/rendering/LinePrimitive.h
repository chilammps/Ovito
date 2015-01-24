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

#ifndef __OVITO_LINE_DISPLAY_PRIMITIVE_H
#define __OVITO_LINE_DISPLAY_PRIMITIVE_H

#include <core/Core.h>
#include "PrimitiveBase.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Abstract base class for line drawing primitives.
 */
class OVITO_CORE_EXPORT LinePrimitive : public PrimitiveBase
{
public:

	/// \brief Allocates a geometry buffer with the given number of vertices.
	virtual void setVertexCount(int vertexCount, FloatType lineWidth = FloatType(1)) = 0;

	/// \brief Returns the number of vertices stored in the buffer.
	virtual int vertexCount() const = 0;

	/// \brief Sets the coordinates of the vertices.
	virtual void setVertexPositions(const Point3* coordinates) = 0;

	/// \brief Sets the colors of the vertices.
	virtual void setVertexColors(const ColorA* colors) = 0;

	/// \brief Sets the color of all vertices to the given value.
	virtual void setLineColor(const ColorA color) = 0;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_LINE_DISPLAY_PRIMITIVE_H
