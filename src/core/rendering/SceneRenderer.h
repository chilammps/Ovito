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
#include "ArrowGeometryBuffer.h"

namespace Ovito {

class DataSet;			// defined in DataSet.h
class SceneNode;		// defined in SceneNode.h
class RenderSettings;	// defined in RenderSettings.h
class FrameBuffer;		// defined in FrameBuffer.h

/**
 * \brief This is the base class for scene renderers.
 */
class OVITO_CORE_EXPORT SceneRenderer : public RefTarget
{
public:

	/// Returns the data set that is being rendered.
	DataSet* dataset() const { OVITO_CHECK_POINTER(_dataset); return _dataset; }

	/// Returns the general rendering settings.
	RenderSettings* renderSettings() const { OVITO_CHECK_POINTER(_settings); return _settings; }

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
	Viewport* viewport() const { OVITO_CHECK_OBJECT_POINTER(_viewport); return _viewport; }

	/// \brief Computes the bounding box of the entire scene to be rendered.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the world coordinate system that contains
	///         everything to be rendered.
	virtual Box3 sceneBoundingBox(TimePoint time);

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
	virtual OORef<ParticleGeometryBuffer> createParticleGeometryBuffer(ParticleGeometryBuffer::ShadingMode shadingMode = ParticleGeometryBuffer::NormalShading, ParticleGeometryBuffer::RenderingQuality renderingQuality = ParticleGeometryBuffer::MediumQuality) = 0;

	/// Requests a new text geometry buffer from the renderer.
	virtual OORef<TextGeometryBuffer> createTextGeometryBuffer() = 0;

	/// Requests a new image geometry buffer from the renderer.
	virtual OORef<ImageGeometryBuffer> createImageGeometryBuffer() = 0;

	/// Requests a new arrow geometry buffer from the renderer.
	virtual OORef<ArrowGeometryBuffer> createArrowGeometryBuffer(ArrowGeometryBuffer::Shape shape, ArrowGeometryBuffer::ShadingMode shadingMode = ArrowGeometryBuffer::NormalShading, ArrowGeometryBuffer::RenderingQuality renderingQuality = ArrowGeometryBuffer::MediumQuality) = 0;

	/// Returns whether object picking mode is active.
	bool isPicking() const { return _isPicking; }

	/// Returns whether this renderer is rendering an interactive viewport.
	/// \return true if rendering a real-time viewport; false if rendering an output image.
	virtual bool isInteractive() const = 0;

	/// When picking mode is active, this registers an object being rendered.
	virtual quint32 registerPickObject(ObjectNode* objNode, SceneObject* sceneObj, quint32 subObjectCount = 1) { return 0; }

protected:

	/// Constructor.
	SceneRenderer();

	/// \brief Renders all nodes in the scene.
	virtual void renderScene();

	/// \brief Render a scene node (and all its children).
	virtual void renderNode(SceneNode* node);

	/// Sets whether object picking mode is active.
	void setPicking(bool enable) { _isPicking = enable; }

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

	/// Indicates that object picking mode is active.
	bool _isPicking;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_SCENE_RENDERER_H
