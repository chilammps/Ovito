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

namespace Ovito {

class DataSet;			// defined in DataSet.h
class SceneNode;		// defined in SceneNode.h

/**
 * \brief This is the base class for scene renderers.
 */
class SceneRenderer : public RefTarget
{
public:

	/// Returns the data set that is being rendered.
	DataSet* dataset() const { return _dataset; }

	/// Sets the data set that is being rendered.
	void setDataset(DataSet* dataset) { _dataset = dataset; }

	/// Returns the view projection parameters.
	const ViewProjectionParameters& projParams() const { return _projParams; }

	/// Sets the view projection parameters.
	void setProjParams(const ViewProjectionParameters& params) { _projParams = params; }

	/// Returns the animation time being rendered.
	TimePoint time() const { return _time; }

	/// Sets the animation time being rendered.
	void setTime(TimePoint time) { _time = time; }

	/// Returns the viewport whose contents are currently being rendered.
	/// This may be NULL.
	Viewport* viewport() const { return _viewport; }

	/// Specifies the viewport whose contents are currently being rendered.
	void setViewport(Viewport* vp) { _viewport = vp; }

	/// This method is called just before renderFrame() is called.
	virtual void beginRender() = 0;

	/// Renders the current animation frame.
	virtual void renderFrame() = 0;

	/// This method is called after renderFrame() has been called.
	virtual void endRender() = 0;

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) = 0;

	/// Requests a new line geometry buffer from the renderer.
	virtual OORef<LineGeometryBuffer> createLineGeometryBuffer() = 0;

	/// Requests a new particle geometry buffer from the renderer.
	virtual OORef<ParticleGeometryBuffer> createParticleGeometryBuffer() = 0;

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
