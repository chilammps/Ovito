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
 * \file SceneRenderer.h
 * \brief Contains the definition of the Ovito::SceneRenderer class.
 */

#ifndef __OVITO_SCENE_RENDERER_H
#define __OVITO_SCENE_RENDERER_H

#include <core/Core.h>
#include <core/animation/TimeInterval.h>
#include <core/reference/RefTarget.h>
#include <core/viewport/Viewport.h>
#include "LineGeometryBuffer.h"
#include "ParticleGeometryBuffer.h"
#include "TextGeometryBuffer.h"
#include "ImageGeometryBuffer.h"

namespace Ovito {

class DataSet;			// defined in DataSet.h
class SceneNode;		// defined in SceneNode.h
class RenderSettings;	// defined in RenderSettings.h
class FrameBuffer;		// defined in FrameBuffer.h

/**
 * \brief This is the base class for scene renderers.
 */
class SceneRenderer : public RefTarget
{
public:

	/// Returns the data set that is being rendered.
	DataSet* dataset() const { return _dataset; }

	/// Returns the general rendering settings.
	RenderSettings* renderSettings() const { return _settings; }

	/// Prepares the renderer for rendering and sets the data set that is being rendered.
	virtual bool startRender(DataSet* dataset, RenderSettings* settings) {
		_dataset = dataset;
		_settings = settings;
		return true;
	}

	/// Is called after rendering has finished.
	virtual void endRender() { _dataset = nullptr; _settings = nullptr; }

	/// Returns the view projection parameters.
	const ViewProjectionParameters& projParams() const { return _projParams; }

	/// Changes the view projection parameters.
	void setProjParams(const ViewProjectionParameters& params) { _projParams = params; }

	/// Returns the animation time being rendered.
	TimePoint time() const { return _time; }

	/// Returns the viewport whose contents are currently being rendered.
	/// This may be NULL.
	Viewport* viewport() const { return _viewport; }

	/// This method is called just before renderFrame() is called.
	/// Sets the view projection parameters, the animation frame to render.
	/// and the viewport whose being rendered.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) {
		_time = time;
		setProjParams(params);
		_viewport = vp;
	}

	/// Renders the current animation frame.
	/// Returns false if the operation has been canceled by the user.
	virtual bool renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress) = 0;

	/// This method is called after renderFrame() has been called.
	virtual void endFrame() {}

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) = 0;

	/// Requests a new line geometry buffer from the renderer.
	virtual OORef<LineGeometryBuffer> createLineGeometryBuffer() = 0;

	/// Requests a new particle geometry buffer from the renderer.
	virtual OORef<ParticleGeometryBuffer> createParticleGeometryBuffer() = 0;

	/// Requests a new text geometry buffer from the renderer.
	virtual OORef<TextGeometryBuffer> createTextGeometryBuffer() = 0;

	/// Requests a new image geometry buffer from the renderer.
	virtual OORef<ImageGeometryBuffer> createImageGeometryBuffer() = 0;

protected:

	/// Constructor.
	SceneRenderer();

	/// \brief Renders all nodes in the scene.
	virtual void renderScene();

	/// \brief Render a scene node (and all its children).
	virtual void renderNode(SceneNode* node);

private:

	/// The data set that is being rendered.
	DataSet* _dataset;

	/// The general render settings.
	RenderSettings* _settings;

	/// The viewport whose contents are currently being rendered.
	Viewport* _viewport;

	/// The view projection parameters.
	ViewProjectionParameters _projParams;

	/// The animation time being rendered.
	TimePoint _time;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_SCENE_RENDERER_H
