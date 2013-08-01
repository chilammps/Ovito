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
 * \file ViewportTextGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ViewportTextGeometryBuffer class.
 */

#ifndef __OVITO_VIEWPORT_TEXT_GEOMETRY_BUFFER_H
#define __OVITO_VIEWPORT_TEXT_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/TextGeometryBuffer.h>
#include <core/utilities/opengl/SharedOpenGLResource.h>

namespace Ovito {

class ViewportSceneRenderer;

/**
 * \brief Buffer object that stores a text string to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT ViewportTextGeometryBuffer : public TextGeometryBuffer, private SharedOpenGLResource
{
public:

	/// Constructor.
	ViewportTextGeometryBuffer(ViewportSceneRenderer* renderer);

	/// Destructor.
	virtual ~ViewportTextGeometryBuffer();

	/// \brief Sets the text to be rendered.
	virtual void setText(const QString& text) override {
		if(text != this->text())
			_needTextureUpdate = true;
		TextGeometryBuffer::setText(text);
	}

	/// Sets the text font.
	virtual void setFont(const QFont& font) override {
		if(font != this->font())
			_needTextureUpdate = true;
		TextGeometryBuffer::setFont(font);
	}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the text string at the given 2D window (pixel) coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, int alignment = Qt::AlignLeft | Qt::AlignTop) override;

	/// \brief Renders the text string at the given 2D normalized viewport coordinates ([-1,+1] range).
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, int alignment = Qt::AlignLeft | Qt::AlignTop) override;

protected:

    /// This method that takes care of freeing the shared OpenGL resources owned by this class.
    virtual void freeOpenGLResources() override;

private:

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the text.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL vertex buffer that stores the vertex positions.
	QOpenGLBuffer _vertexBuffer;

	/// Resource identifier of the OpenGL texture that is used for rendering the text image.
	GLuint _texture;

	/// The texture image.
	QImage _textureImage;

	/// The position of the text inside the texture images.
	QPoint _textOffset;

	/// Indicates that the texture needs to be updated.
	bool _needTextureUpdate;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_VIEWPORT_TEXT_GEOMETRY_BUFFER_H
