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
#include <core/viewport/ViewportWindow.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/rendering/RenderSettings.h>
#include "AmbientOcclusionRenderer.h"

namespace Viz {

IMPLEMENT_OVITO_OBJECT(Viz, AmbientOcclusionRenderer, ViewportSceneRenderer);

/******************************************************************************
* Prepares the renderer for rendering and sets the data set that is being rendered.
******************************************************************************/
bool AmbientOcclusionRenderer::startRender(DataSet* dataset, RenderSettings* settings)
{
	if(!ViewportSceneRenderer::startRender(dataset, settings))
		return false;

	// Set up surface format with a depth buffer.
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setMajorVersion(3);
	format.setMinorVersion(2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setSwapBehavior(QSurfaceFormat::SingleBuffer);

	_offscreenContext.reset(new QOpenGLContext());
	_offscreenContext->setFormat(format);
	if(!_offscreenContext->create())
		throw Exception(tr("Failed to create OpenGL context."));

	// Create offscreen buffer.
	_offscreenSurface.setFormat(_offscreenContext->format());
	_offscreenSurface.create();
	if(!_offscreenSurface.isValid())
		throw Exception(tr("Failed to create offscreen rendering surface."));

	// Make the context current.
	if(!_offscreenContext->makeCurrent(&_offscreenSurface))
		throw Exception(tr("Failed to make OpenGL context current."));

	// Check OpenGL version.
	if(_offscreenContext->format().majorVersion() < 3) {
		throw Exception(tr(
				"The OpenGL implementation available on this system does not support OpenGL version 3.0 or newer.\n\n"
				"Ovito requires modern graphics hardware to accelerate 3d rendering. You current system configuration is not compatible with Ovito.\n\n"
				"To avoid this error message, please install the newest graphics driver, or upgrade your graphics card.\n\n"
				"The currently installed OpenGL graphics driver reports the following information:\n\n"
				"OpenGL Vendor: %1\n"
				"OpenGL Renderer: %2\n"
				"OpenGL Version: %3")
				.arg(QString((const char*)glGetString(GL_VENDOR)))
				.arg(QString((const char*)glGetString(GL_RENDERER)))
				.arg(QString((const char*)glGetString(GL_VERSION)))
				);
	}

	// Create OpenGL framebuffer.
	QOpenGLFramebufferObjectFormat framebufferFormat;
	framebufferFormat.setAttachment(QOpenGLFramebufferObject::Depth);
	_framebufferObject.reset(new QOpenGLFramebufferObject(_resolution, framebufferFormat));
	if(!_framebufferObject->isValid())
		throw Exception(tr("Failed to create OpenGL framebuffer object for offscreen rendering."));

	// Bind OpenGL buffer.
	if(!_framebufferObject->bind())
		throw Exception(tr("Failed to bind OpenGL framebuffer object for offscreen rendering."));

	return true;
}

/******************************************************************************
* This method is called just before renderFrame() is called.
******************************************************************************/
void AmbientOcclusionRenderer::beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp)
{
	// Make GL context current.
	if(!_offscreenContext->makeCurrent(&_offscreenSurface))
		throw Exception(tr("Failed to make OpenGL context current."));

	ViewportSceneRenderer::beginFrame(time, params, vp);

	// Setup GL viewport and background color.
	OVITO_CHECK_OPENGL(glViewport(0, 0, _resolution.width(), _resolution.height()));
	OVITO_CHECK_OPENGL(glClearColor(0, 0, 0, 0));

	// Clear buffer.
	OVITO_CHECK_OPENGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	OVITO_CHECK_OPENGL(glEnable(GL_DEPTH_TEST));
}

/******************************************************************************
* This method is called after renderFrame() has been called.
******************************************************************************/
void AmbientOcclusionRenderer::endFrame()
{
	// Flush the contents to the FBO before extracting image.
	glFlush();

	// Fetch rendered image from OpenGL framebuffer.
	QSize size = _framebufferObject->size();
	_image = QImage(size, QImage::Format_ARGB32);
	glReadPixels(0, 0, size.width(), size.height(), GL_BGRA, GL_UNSIGNED_BYTE, _image.bits());
	if(glGetError()) {
		glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, _image.bits());
		_image = _image.rgbSwapped();
	}

	ViewportSceneRenderer::endFrame();
}

/******************************************************************************
* Is called after rendering has finished.
******************************************************************************/
void AmbientOcclusionRenderer::endRender()
{
	_framebufferObject.reset();
	_offscreenContext.reset();
	_offscreenSurface.destroy();
	ViewportSceneRenderer::endRender();
}

};
