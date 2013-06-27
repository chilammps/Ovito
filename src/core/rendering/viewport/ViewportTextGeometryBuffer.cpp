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

#include <core/Core.h>
#include "ViewportTextGeometryBuffer.h"
#include "ViewportSceneRenderer.h"

#include <QGLWidget>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ViewportTextGeometryBuffer, TextGeometryBuffer);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportTextGeometryBuffer::ViewportTextGeometryBuffer(ViewportSceneRenderer* renderer) :
	_renderer(renderer),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_texture(0),
	_needTextureUpdate(true)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shader.
	_shader = renderer->loadShaderProgram("text", ":/core/glsl/text.vertex.glsl", ":/core/glsl/text.fragment.glsl");

	// Create OpenGL texture.
	glGenTextures(1, &_texture);

	// Make sure texture gets deleted again when this object is destroyed.
	attachOpenGLResources();
}

/******************************************************************************
* Destructor.
******************************************************************************/
ViewportTextGeometryBuffer::~ViewportTextGeometryBuffer()
{
	destroyOpenGLResources();
}

/******************************************************************************
* This method that takes care of freeing the shared OpenGL resources owned
* by this class.
******************************************************************************/
void ViewportTextGeometryBuffer::freeOpenGLResources()
{
	OVITO_CHECK_OPENGL(glDeleteTextures(1, &_texture));
	_texture = 0;
}

/******************************************************************************
* Returns true if the buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportTextGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return (_contextGroup == vpRenderer->glcontext()->shareGroup()) && (_texture != 0);
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportTextGeometryBuffer::render(const Point2& pos)
{
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_texture != 0);

	if(text().isEmpty())
		return;

	// Compute text area. This is all in pixel coordinates.
	QFontMetricsF metrics(font());
	QRectF rect(metrics.boundingRect(text()));

	// Transform rectangle to normalized device coordinates.
	QRectF rect2 = rect;
	rect2.translate(pos.x(), pos.y());
	GLint vc[4];
	glGetIntegerv(GL_VIEWPORT, vc);
	QVector2D corners[4] = {
			QVector2D(rect2.left() / vc[2] * 2 - 1, 1 - rect2.bottom() / vc[3] * 2),
			QVector2D(rect2.right() / vc[2] * 2 - 1, 1 - rect2.bottom() / vc[3] * 2),
			QVector2D(rect2.left() / vc[2] * 2 - 1, 1 - rect2.top() / vc[3] * 2),
			QVector2D(rect2.right() / vc[2] * 2 - 1, 1 - rect2.top() / vc[3] * 2)
	};

	// Prepare texture.
	OVITO_CHECK_OPENGL(glBindTexture(GL_TEXTURE_2D, _texture));
	renderer()->glfuncs()->glActiveTexture(GL_TEXTURE0);

	if(_needTextureUpdate) {
		_needTextureUpdate = false;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		// Generate texture image.
		QImage image(rect.width(), rect.height(), QImage::Format_RGB32);
		image.fill(0);
		{
			QPainter painter(&image);
			painter.setFont(font());
			painter.setPen(Qt::white);
			painter.drawText(-rect.left(), -rect.top(), text());
		}

		// Upload texture data.
		QImage textureImage = QGLWidget::convertToGLFormat(image);
		OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.width(), textureImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage.constBits()));
	}

	bool wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	bool wasBlendEnabled = glIsEnabled(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!_shader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	_shader->setUniformValue("text_color", color().r(), color().g(), color().b(), color().a());
	_shader->setUniformValueArray("corners", corners, 4);

	OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	_shader->release();

	// Restore old state.
	if(wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
	if(!wasBlendEnabled) glDisable(GL_BLEND);
}

};
