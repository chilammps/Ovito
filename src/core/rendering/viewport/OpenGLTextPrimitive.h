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
 * \file OpenGLTextPrimitive.h
 * \brief Contains the definition of the Ovito::OpenGLTextPrimitive class.
 */

#ifndef __OVITO_OPENGL_TEXT_PRIMITIVE_H
#define __OVITO_OPENGL_TEXT_PRIMITIVE_H

#include <core/Core.h>
#include <core/rendering/TextPrimitive.h>
#include "OpenGLTexture.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Buffer object that stores a text string to be rendered in the viewports.
 */
class OVITO_CORE_EXPORT OpenGLTextPrimitive : public TextPrimitive
{
public:

	/// Constructor.
	OpenGLTextPrimitive(ViewportSceneRenderer* renderer);

	/// \brief Sets the text to be rendered.
	virtual void setText(const QString& text) override {
		if(text != this->text())
			_needTextureUpdate = true;
		TextPrimitive::setText(text);
	}

	/// Sets the text font.
	virtual void setFont(const QFont& font) override {
		if(font != this->font())
			_needTextureUpdate = true;
		TextPrimitive::setFont(font);
	}

	/// Sets the text color.
	virtual void setColor(const ColorA& color) override {
		if(color != this->color())
			_needTextureUpdate = true;
		TextPrimitive::setColor(color);
	}

	/// Sets the text background color.
	virtual void setBackgroundColor(const ColorA& color) override {
		if(color != this->backgroundColor())
			_needTextureUpdate = true;
		TextPrimitive::setBackgroundColor(color);
	}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the text string at the given 2D window (pixel) coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, int alignment = Qt::AlignLeft | Qt::AlignTop) override;

	/// \brief Renders the text string at the given 2D normalized viewport coordinates ([-1,+1] range).
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, int alignment = Qt::AlignLeft | Qt::AlignTop) override;

private:

	/// The GL context group under which the GL vertex buffer has been created.
	QOpenGLContextGroup* _contextGroup;

	/// The OpenGL shader program used to render the text.
	QOpenGLShaderProgram* _shader;

	/// The OpenGL vertex buffer that stores the vertex positions.
	QOpenGLBuffer _vertexBuffer;

	/// The OpenGL texture that is used for rendering the text image.
	OpenGLTexture _texture;

	/// The texture image.
	QImage _textureImage;

	/// The position of the text inside the texture images.
	QPoint _textOffset;

	/// Indicates that the texture needs to be updated.
	bool _needTextureUpdate;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_TEXT_PRIMITIVE_H
