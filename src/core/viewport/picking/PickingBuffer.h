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
 * \file PickingBuffer.h
 * \brief Contains the definition of the Ovito::PickingBuffer class.
 */
#ifndef __OVITO_PICKING_BUFFER_H
#define __OVITO_PICKING_BUFFER_H

#include <core/Core.h>

namespace Ovito {

/**
 *
 */
class PickingBuffer
{
public:

	/// Default constructor.
	PickingBuffer() {}

	/// Prepares the renderer for rendering and sets the data set that is being rendered.
	virtual bool startRender(DataSet* dataset, RenderSettings* settings) override;

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// Renders the current animation frame.
	virtual bool renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress) override;

	/// This method is called after renderFrame() has been called.
	virtual void endFrame() override;

	/// Is called after rendering has finished.
	virtual void endRender() override;

private:

	/// The offscreen surface used to render into an image buffer using OpenGL.
	QOffscreenSurface _offscreenSurface;

	/// The OpenGL rendering context.
	QScopedPointer<QOpenGLContext> _offscreenContext;

	/// The OpenGL framebuffer.
	QScopedPointer<QOpenGLFramebufferObject> _framebufferObject;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_PICKING_SCENE_RENDERER_H
