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
#include <core/viewport/ViewportConfiguration.h>
#include <core/rendering/RenderSettings.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include "StandardSceneRenderer.h"
#include "StandardSceneRendererEditor.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, StandardSceneRenderer, ViewportSceneRenderer);
SET_OVITO_OBJECT_EDITOR(StandardSceneRenderer, StandardSceneRendererEditor);
DEFINE_PROPERTY_FIELD(StandardSceneRenderer, _antialiasingLevel, "AntialiasingLevel");
SET_PROPERTY_FIELD_LABEL(StandardSceneRenderer, _antialiasingLevel, "Antialiasing level");

/******************************************************************************
* Prepares the renderer for rendering and sets the data set that is being rendered.
******************************************************************************/
bool StandardSceneRenderer::startRender(DataSet* dataset, RenderSettings* settings)
{
	if(Application::instance().headlessMode())
		throw Exception(tr("Cannot use OpenGL renderer when program is running in headless mode. "
				"Please use a different rendering engine or start program on a machine where access to "
				"graphics hardware is possible."));

	if(!ViewportSceneRenderer::startRender(dataset, settings))
		return false;

	int sampling = std::max(1, antialiasingLevel());

	QOpenGLContext* glcontext;
	// in GUI mode, use the OpenGL context managed by the main window to render to the offscreen buffer.
	if(Application::instance().guiMode())
		glcontext = renderDataset()->mainWindow()->getOpenGLContext();
	else {
		// Create new OpenGL context for rendering in console mode.
		OVITO_ASSERT(QOpenGLContext::currentContext() == nullptr);
		_offscreenContext.reset(new QOpenGLContext());
		_offscreenContext->setFormat(ViewportSceneRenderer::getDefaultSurfaceFormat());
		if(!_offscreenContext->create())
			throw Exception(tr("Failed to create OpenGL context for rendering."));
		glcontext = _offscreenContext.data();
	}

	// Create offscreen buffer.
	if(_offscreenSurface.isNull())
		_offscreenSurface.reset(new QOffscreenSurface());
	_offscreenSurface->setFormat(glcontext->format());
	_offscreenSurface->create();
	if(!_offscreenSurface->isValid())
		throw Exception(tr("Failed to create offscreen rendering surface."));

	// Make the context current.
	if(!glcontext->makeCurrent(_offscreenSurface.data()))
		throw Exception(tr("Failed to make OpenGL context current."));
	OVITO_REPORT_OPENGL_ERRORS();

	// Create OpenGL framebuffer.
	_framebufferSize = QSize(settings->outputImageWidth() * sampling, settings->outputImageHeight() * sampling);
	QOpenGLFramebufferObjectFormat framebufferFormat;
	framebufferFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
	_framebufferObject.reset(new QOpenGLFramebufferObject(_framebufferSize.width(), _framebufferSize.height(), framebufferFormat));
	if(!_framebufferObject->isValid())
		throw Exception(tr("Failed to create OpenGL framebuffer object for offscreen rendering."));
	OVITO_REPORT_OPENGL_ERRORS();

	// Bind OpenGL buffer.
	if(!_framebufferObject->bind())
		throw Exception(tr("Failed to bind OpenGL framebuffer object for offscreen rendering."));
	OVITO_REPORT_OPENGL_ERRORS();

	return true;
}

/******************************************************************************
* This method is called just before renderFrame() is called.
******************************************************************************/
void StandardSceneRenderer::beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp)
{
	// Make GL context current.
	QOpenGLContext* glcontext;
	if(Application::instance().guiMode())
		glcontext = renderDataset()->mainWindow()->getOpenGLContext();
	else
		glcontext = _offscreenContext.data();
	if(!glcontext->makeCurrent(_offscreenSurface.data()))
		throw Exception(tr("Failed to make OpenGL context current."));
	OVITO_REPORT_OPENGL_ERRORS();

	ViewportSceneRenderer::beginFrame(time, params, vp);

	// Setup GL viewport.
	OVITO_CHECK_OPENGL(glViewport(0, 0, _framebufferSize.width(), _framebufferSize.height()));

	// Set rendering background color.
	if(!renderSettings()->generateAlphaChannel()) {
		Color backgroundColor = renderSettings()->backgroundColor();
		glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), 1);
	}
	else glClearColor(0, 0, 0, 0);
}

/******************************************************************************
* Renders the current animation frame.
******************************************************************************/
bool StandardSceneRenderer::renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress)
{
	// Let the base class do the main rendering work.
	if(!ViewportSceneRenderer::renderFrame(frameBuffer, progress))
		return false;

	// Flush the contents to the FBO before extracting image.
	glcontext()->swapBuffers(_offscreenSurface.data());

	// Fetch rendered image from OpenGL framebuffer.
    QImage bufferImage = _framebufferObject->toImage();

	// Scale it down to the output size.
	QImage image = bufferImage.scaled(frameBuffer->image().width(), frameBuffer->image().height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// Copy OpenGL image to the output frame buffer.
	frameBuffer->image() = image;
	frameBuffer->update();

	return true;
}

/******************************************************************************
* Is called after rendering has finished.
******************************************************************************/
void StandardSceneRenderer::endRender()
{
	QOpenGLFramebufferObject::bindDefault();
	QOpenGLContext* ctxt = QOpenGLContext::currentContext();
	if(ctxt) ctxt->doneCurrent();
	_framebufferObject.reset();
	_offscreenContext.reset();
	_offscreenSurface.reset();
	ViewportSceneRenderer::endRender();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
