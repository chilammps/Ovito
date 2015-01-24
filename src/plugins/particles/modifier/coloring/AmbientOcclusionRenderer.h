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

#ifndef __OVITO_AMBIENT_OCCLUSION_RENDERER_H
#define __OVITO_AMBIENT_OCCLUSION_RENDERER_H

#include <plugins/particles/Particles.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A renderer used to compute ambient occlusion lighting.
 */
class OVITO_PARTICLES_EXPORT AmbientOcclusionRenderer : public ViewportSceneRenderer
{
public:

	/// Constructor.
	AmbientOcclusionRenderer(DataSet* dataset, QSize resolution, QOffscreenSurface& offscreenSurface) : ViewportSceneRenderer(dataset), _resolution(resolution), _offscreenSurface(offscreenSurface) {
		setPicking(true);
	}

	/// Prepares the renderer for rendering and sets the data set that is being rendered.
	virtual bool startRender(DataSet* dataset, RenderSettings* settings) override;

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// This method is called after renderFrame() has been called.
	virtual void endFrame() override;

	/// Is called after rendering has finished.
	virtual void endRender() override;

	/// Returns the rendered image.
	const QImage& image() const { return _image; }

	/// Returns the final size of the rendered image in pixels.
	virtual QSize outputSize() const override { return _image.size(); }

	/// Registers a range of sub-IDs belonging to the current object being rendered.
	virtual quint32 registerSubObjectIDs(quint32 subObjectCount) override { return 1; }

	/// Returns whether this renderer is rendering an interactive viewport.
	virtual bool isInteractive() const override { return false; }

private:

	/// The OpenGL framebuffer.
	QScopedPointer<QOpenGLFramebufferObject> _framebufferObject;

	/// The OpenGL rendering context.
	QScopedPointer<QOpenGLContext> _offscreenContext;

	/// The offscreen surface used to render into an image buffer using OpenGL.
	QOffscreenSurface& _offscreenSurface;

	/// The rendered image.
	QImage _image;

	/// The rendering resolution.
	QSize _resolution;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_AMBIENT_OCCLUSION_RENDERER_H
