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
 * \file LineGeometryBuffer.h
 * \brief Contains the definition of the Ovito::LineGeometryBuffer class.
 */

#ifndef __OVITO_LINE_GEOMETRY_BUFFER_H
#define __OVITO_LINE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito {

class SceneRenderer;			// defined in SceneRenderer.h

/**
 * \brief Abstract base class for buffer objects that store line geometry.
 */
class LineGeometryBuffer : public OvitoObject
{
public:

	/// A structure that stores the data of a single vertex.
	struct Vertex {
		/// The XYZ coordinates of the vertex.
		Point3 position;
		/// The color of the vertex.
		ColorA color;
	};

public:

	/// \brief Allocates a geometry buffer with the given number of vertices.
	/// \param vertexCount The number of vertices. Must be an even number.
	virtual void beginCreate(int vertexCount) = 0;

	/// \brief Returns a pointer to the internal vertex array allocated by beginCreate().
	virtual Vertex* vertexBuffer() = 0;

	/// \brief This finalizes the buffer after it has has been filled with data.
	virtual void endCreate() = 0;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the geometry.
	virtual void render() = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_LINE_GEOMETRY_BUFFER_H
