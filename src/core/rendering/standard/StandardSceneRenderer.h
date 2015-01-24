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

#ifndef __OVITO_STANDARD_SCENE_RENDERER_H
#define __OVITO_STANDARD_SCENE_RENDERER_H

#include <core/Core.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief This is the default scene renderer used for high-quality image output.
 */
class OVITO_CORE_EXPORT StandardSceneRenderer : public ViewportSceneRenderer
{
public:

	/// Default constructor.
	Q_INVOKABLE StandardSceneRenderer(DataSet* dataset) : ViewportSceneRenderer(dataset), _antialiasingLevel(3) {
		INIT_PROPERTY_FIELD(StandardSceneRenderer::_antialiasingLevel);
	}

	/// Returns the number of sub-pixels to render.
	virtual int antialiasingLevel() const override { return _antialiasingLevel; }

	/// Sets the number of sub-pixels to render.
	void setAntialiasingLevel(int newLevel) { _antialiasingLevel = newLevel; }

	/// Prepares the renderer for rendering and sets the data set that is being rendered.
	virtual bool startRender(DataSet* dataset, RenderSettings* settings) override;

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// Renders the current animation frame.
	virtual bool renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress) override;

	/// Is called after rendering has finished.
	virtual void endRender() override;

	/// Returns whether this renderer is rendering an interactive viewport.
	/// \return true if rendering a real-time viewport; false if rendering an output image.
	virtual bool isInteractive() const override { return false; }

	/// Returns the final size of the rendered image in pixels.
	virtual QSize outputSize() const override { return SceneRenderer::outputSize(); }

public:

	Q_PROPERTY(int antialiasingLevel READ antialiasingLevel WRITE setAntialiasingLevel);

private:

	/// Controls the number of sub-pixels to render.
	PropertyField<int> _antialiasingLevel;

	/// The offscreen surface used to render into an image buffer using OpenGL.
	QScopedPointer<QOffscreenSurface> _offscreenSurface;

	/// The temporary OpenGL rendering context.
	QScopedPointer<QOpenGLContext> _offscreenContext;

	/// The OpenGL framebuffer.
	QScopedPointer<QOpenGLFramebufferObject> _framebufferObject;

	/// The resolution of the offscreen framebuffer.
	QSize _framebufferSize;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "OpenGL renderer");

	DECLARE_PROPERTY_FIELD(_antialiasingLevel);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_STANDARD_SCENE_RENDERER_H
