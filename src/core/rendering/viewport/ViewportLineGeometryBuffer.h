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
 * \file ViewportLineGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportLineGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_LINE_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_LINE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/LineGeometryBuffer.h>

namespace Ovito {

class ViewportSceneRenderer;

/**
 * \brief Buffer object that stores line geometry to be rendered in the viewports.
 */
class ViewportLineGeometryBuffer : public LineGeometryBuffer
{
protected:

	/// Constructor.
	ViewportLineGeometryBuffer(ViewportSceneRenderer* renderer) :
		_contextGroup(QOpenGLContextGroup::currentContextGroup()), _vertexBuffer(nullptr), _vertexCount(0) {}

public:

	/// \brief Allocates a geometry buffer with the given number of vertices.
	/// \param vertexCount The number of vertices. Must be an even number.
	virtual void beginCreate(int vertexCount) override;

	/// \brief Returns a pointer to the internal vertex array allocated by beginCreate().
	virtual Vertex* vertexBuffer() override { return _vertexBuffer; }

	/// \brief This finalizes the buffer after it has has been filled with data.
	virtual void endCreate() override;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render() override;

private:

	/// The internal OpenGL vertex buffer.
	QOpenGLBuffer _glbuffer;

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The number of vertices stored in the buffer.
	int _vertexCount;

	/// Pointer to the memory-mapped vertex buffer.
	Vertex* _vertexBuffer;

	Q_OBJECT
	OVITO_OBJECT

	friend class ViewportSceneRenderer;
};

};

#endif // __OVITO_VIEWPORT_LINE_GEOMETRY_BUFFER_H
