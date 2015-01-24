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

#ifndef __OVITO_OPENGL_IMAGE_PRIMITIVE_H
#define __OVITO_OPENGL_IMAGE_PRIMITIVE_H

#include <core/Core.h>
#include <core/rendering/ImagePrimitive.h>
#include "OpenGLTexture.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Buffer object that stores an image to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT OpenGLImagePrimitive : public ImagePrimitive
{
public:

	/// Constructor.
	OpenGLImagePrimitive(ViewportSceneRenderer* renderer);

	/// \brief Sets the image to be rendered.
	virtual void setImage(const QImage& image) override {
		_needTextureUpdate = true;
		ImagePrimitive::setImage(image);
	}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the image in a rectangle given in pixel coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, const Vector2& size) override;

	/// \brief Renders the image in a rectangle given in viewport coordinates.
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, const Vector2& size) override;

private:

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the image.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL vertex buffer that stores the vertex positions.
	QOpenGLBuffer _vertexBuffer;

	/// The OpenGL texture that is used for rendering the image.
	OpenGLTexture _texture;

	/// Indicates that the texture needs to be updated.
	bool _needTextureUpdate;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_IMAGE_PRIMITIVE_H
