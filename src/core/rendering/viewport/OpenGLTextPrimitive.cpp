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
#include "OpenGLTextPrimitive.h"
#include "ViewportSceneRenderer.h"

#include <QGLWidget>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructor.
******************************************************************************/
OpenGLTextPrimitive::OpenGLTextPrimitive(ViewportSceneRenderer* renderer) :
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_needTextureUpdate(true),
	_textureImage(1, 1, QImage::Format_RGB32)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shader.
	_shader = renderer->loadShaderProgram("text", ":/core/glsl/text/text.vs", ":/core/glsl/text/text.fs");

	// Create vertex buffer
	if(!_vertexBuffer.create())
		throw Exception(QStringLiteral("Failed to create OpenGL vertex buffer."));
	_vertexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
	if(!_vertexBuffer.bind())
			throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(_vertexBuffer.allocate(4 * sizeof(Point2)));
	_vertexBuffer.release();

	// Create OpenGL texture.
	_texture.create();
}

/******************************************************************************
* Returns true if the buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool OpenGLTextPrimitive::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return (_contextGroup == vpRenderer->glcontext()->shareGroup()) && _texture.isCreated() && _vertexBuffer.isCreated();
}

/******************************************************************************
* Renders the text string at the given location given in normalized
* viewport coordinates ([-1,+1] range).
******************************************************************************/
void OpenGLTextPrimitive::renderViewport(SceneRenderer* renderer, const Point2& pos, int alignment)
{
	GLint vc[4];
	glGetIntegerv(GL_VIEWPORT, vc);

	Point2 windowPos((pos.x() + 1.0) * vc[2] / 2, (-pos.y() + 1.0) * vc[3] / 2);
	renderWindow(renderer, windowPos, alignment);
}

/******************************************************************************
* Renders the text string at the given 2D window (pixel) coordinates.
******************************************************************************/
void OpenGLTextPrimitive::renderWindow(SceneRenderer* renderer, const Point2& pos, int alignment)
{
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_texture.isCreated());
	OVITO_STATIC_ASSERT(sizeof(FloatType) == sizeof(float) && sizeof(Point2) == sizeof(float)*2);
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(text().isEmpty() || !vpRenderer || renderer->isPicking())
		return;

	vpRenderer->rebindVAO();

	// Enable texturing when using compatibility OpenGL. In the core profile, this is enabled by default.
	if(vpRenderer->isCoreProfile() == false)
		OVITO_CHECK_OPENGL(glEnable(GL_TEXTURE_2D));

	// Prepare texture.
	_texture.bind();

	if(_needTextureUpdate) {
		_needTextureUpdate = false;

		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0));
		OVITO_CHECK_OPENGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

		// Measure text size.
		QRect rect;
		qreal devicePixelRatio = 1.0;
		{
			if(vpRenderer->glcontext()->surface()->surfaceClass() == QSurface::Window) {
				QWindow* window = static_cast<QWindow*>(vpRenderer->glcontext()->surface());
				devicePixelRatio = window->devicePixelRatio();
			}
			_textureImage.setDevicePixelRatio(devicePixelRatio);
			QPainter painter(&_textureImage);
			painter.setFont(font());
			rect = painter.boundingRect(QRect(), Qt::AlignLeft | Qt::AlignTop, text());
		}

		// Generate texture image.
		_textureImage = QImage((rect.width() * devicePixelRatio)+1, (rect.height() * devicePixelRatio)+1, QImage::Format_ARGB32_Premultiplied);
		_textureImage.setDevicePixelRatio(devicePixelRatio);
		_textureImage.fill((QColor)backgroundColor());
		{
			QPainter painter(&_textureImage);
			painter.setFont(font());
			painter.setPen((QColor)color());
			painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop, text());
		}
		_textOffset = rect.topLeft();
		//_textureImage.save(QString("%1.png").arg(text()));

		// Upload texture data.
		QImage textureImage = QGLWidget::convertToGLFormat(_textureImage);
		OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.width(), textureImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage.constBits()));
	}

	// Transform rectangle to normalized device coordinates.
	int x = 0, y = 0;
	int w = vpRenderer->antialiasingLevel() * _textureImage.width();
	int h = vpRenderer->antialiasingLevel() * _textureImage.height();
	if(alignment & Qt::AlignRight) x = -w;
	else if(alignment & Qt::AlignHCenter) x = -w / 2;
	if(alignment & Qt::AlignBottom) y = -h;
	else if(alignment & Qt::AlignVCenter) y = -h / 2;
	x += pos.x();
	y += pos.y();
	x = (x / vpRenderer->antialiasingLevel()) * vpRenderer->antialiasingLevel();
	y = (y / vpRenderer->antialiasingLevel()) * vpRenderer->antialiasingLevel();
	QRectF rect2(x, y, w, h);
	GLint vc[4];
	glGetIntegerv(GL_VIEWPORT, vc);
	Point2 corners[4] = {
			Point2(rect2.left() / vc[2] * 2 - 1, 1 - rect2.bottom() / vc[3] * 2),
			Point2(rect2.right() / vc[2] * 2 - 1, 1 - rect2.bottom() / vc[3] * 2),
			Point2(rect2.left() / vc[2] * 2 - 1, 1 - rect2.top() / vc[3] * 2),
			Point2(rect2.right() / vc[2] * 2 - 1, 1 - rect2.top() / vc[3] * 2)
	};

	bool wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	bool wasBlendEnabled = glIsEnabled(GL_BLEND);
	OVITO_CHECK_OPENGL(glDisable(GL_DEPTH_TEST));
	OVITO_CHECK_OPENGL(glEnable(GL_BLEND));
	OVITO_CHECK_OPENGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	if(!_shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader."));

	if(vpRenderer->glformat().majorVersion() >= 3) {

		if(!_vertexBuffer.bind())
			throw Exception(QStringLiteral("Failed to bind OpenGL vertex buffer."));

		// Set up look-up table for texture coordinates.
		static const QVector2D uvcoords[] = {{0,0}, {1,0}, {0,1}, {1,1}};
		OVITO_CHECK_OPENGL(_shader->setUniformValueArray("uvcoords", uvcoords, 4));

		OVITO_CHECK_OPENGL(_vertexBuffer.write(0, corners, 4 * sizeof(Point2)));
		OVITO_CHECK_OPENGL(_shader->enableAttributeArray("vertex_pos"));
		OVITO_CHECK_OPENGL(_shader->setAttributeBuffer("vertex_pos", GL_FLOAT, 0, 2));
		_vertexBuffer.release();

		OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		_shader->disableAttributeArray("vertex_pos");
	}
	else {
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0,0);
		glVertex2f(corners[0].x(), corners[0].y());
		glTexCoord2f(1,0);
		glVertex2f(corners[1].x(), corners[1].y());
		glTexCoord2f(0,1);
		glVertex2f(corners[2].x(), corners[2].y());
		glTexCoord2f(1,1);
		glVertex2f(corners[3].x(), corners[3].y());
		glEnd();
	}

	_shader->release();

	// Restore old state.
	if(wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
	if(!wasBlendEnabled) glDisable(GL_BLEND);

	// Turn off texturing.
	if(vpRenderer->isCoreProfile() == false)
		glDisable(GL_TEXTURE_2D);

	OVITO_REPORT_OPENGL_ERRORS();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
