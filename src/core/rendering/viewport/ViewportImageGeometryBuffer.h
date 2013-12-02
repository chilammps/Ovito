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
 * \file ViewportImageGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportImageGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_IMAGE_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_IMAGE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ImageGeometryBuffer.h>
#include <core/utilities/opengl/SharedOpenGLResource.h>

namespace Ovito {

/**
 * \brief Buffer object that stores an image to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT ViewportImageGeometryBuffer : public ImageGeometryBuffer, private SharedOpenGLResource
{
public:

	/// Constructor.
	ViewportImageGeometryBuffer(ViewportSceneRenderer* renderer);

	/// Destructor.
	virtual ~ViewportImageGeometryBuffer();

	/// \brief Sets the image to be rendered.
	virtual void setImage(const QImage& image) override {
		_needTextureUpdate = true;
		ImageGeometryBuffer::setImage(image);
	}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the image in a rectangle given in pixel coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, const Vector2& size) override;

	/// \brief Renders the image in a rectangle given in viewport coordinates.
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, const Vector2& size) override;

protected:

    /// This method that takes care of freeing the shared OpenGL resources owned by this class.
    virtual void freeOpenGLResources() override;

private:

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the image.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL vertex buffer that stores the vertex positions.
	QOpenGLBuffer _vertexBuffer;

	/// Resource identifier of the OpenGL texture that is used for rendering the image.
	GLuint _texture;

	/// Indicates that the texture needs to be updated.
	bool _needTextureUpdate;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_VIEWPORT_IMAGE_GEOMETRY_BUFFER_H
