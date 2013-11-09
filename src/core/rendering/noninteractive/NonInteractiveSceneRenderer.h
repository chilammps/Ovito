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
 * \file NonInteractiveSceneRenderer.h
 * \brief Contains the definition of the Ovito::NonInteractiveSceneRenderer class.
 */
#ifndef __OVITO_NON_INTERACTIVE_SCENE_RENDERER_H
#define __OVITO_NON_INTERACTIVE_SCENE_RENDERER_H

#include <core/Core.h>
#include <core/rendering/SceneRenderer.h>
#include "DefaultArrowGeometryBuffer.h"
#include "DefaultImageGeometryBuffer.h"
#include "DefaultLineGeometryBuffer.h"
#include "DefaultParticleGeometryBuffer.h"
#include "DefaultTextGeometryBuffer.h"
#include "DefaultTriMeshGeometryBuffer.h"

namespace Ovito {

/**
 * \brief Abstract base class for non-interactive scene renderers.
 */
class OVITO_CORE_EXPORT NonInteractiveSceneRenderer : public SceneRenderer
{
public:

	/// Default constructor.
	NonInteractiveSceneRenderer() : _modelTM(AffineTransformation::Identity()) {}

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) override { _modelTM = tm; }

	/// Returns the current local-to-world transformation matrix.
	virtual const AffineTransformation& worldTransform() const override { return _modelTM; }

	/// Returns the current model-to-world transformation matrix.
	const AffineTransformation& modelTM() const { return _modelTM; }

	/// Requests a new line geometry buffer from the renderer.
	virtual OORef<LineGeometryBuffer> createLineGeometryBuffer() override {
		return new DefaultLineGeometryBuffer();
	}

	/// Requests a new particle geometry buffer from the renderer.
	virtual OORef<ParticleGeometryBuffer> createParticleGeometryBuffer(
			ParticleGeometryBuffer::ShadingMode shadingMode,
			ParticleGeometryBuffer::RenderingQuality renderingQuality,
			ParticleGeometryBuffer::ParticleShape shape) override {
		return new DefaultParticleGeometryBuffer(shadingMode, renderingQuality, shape);
	}

	/// Requests a new text geometry buffer from the renderer.
	virtual OORef<TextGeometryBuffer> createTextGeometryBuffer() override {
		return new DefaultTextGeometryBuffer();
	}

	/// Requests a new image geometry buffer from the renderer.
	virtual OORef<ImageGeometryBuffer> createImageGeometryBuffer() override {
		return new DefaultImageGeometryBuffer();
	}

	/// Requests a new arrow geometry buffer from the renderer.
	virtual OORef<ArrowGeometryBuffer> createArrowGeometryBuffer(
			ArrowGeometryBuffer::Shape shape,
			ArrowGeometryBuffer::ShadingMode shadingMode,
			ArrowGeometryBuffer::RenderingQuality renderingQuality) override {
		return new DefaultArrowGeometryBuffer(shape, shadingMode, renderingQuality);
	}

	/// Requests a new triangle mesh buffer from the renderer.
	virtual OORef<TriMeshGeometryBuffer> createTriMeshGeometryBuffer() override {
		return new DefaultTriMeshGeometryBuffer();
	}

	/// Renders the line geometry stored in the given buffer.
	virtual void renderLines(const DefaultLineGeometryBuffer& lineBuffer) = 0;

	/// Renders the particles stored in the given buffer.
	virtual void renderParticles(const DefaultParticleGeometryBuffer& particleBuffer) = 0;

	/// Renders the arrow elements stored in the given buffer.
	virtual void renderArrows(const DefaultArrowGeometryBuffer& arrowBuffer) = 0;

	/// Renders the text stored in the given buffer.
	virtual void renderText(const DefaultTextGeometryBuffer& textBuffer) = 0;

	/// Renders the image stored in the given buffer.
	virtual void renderImage(const DefaultImageGeometryBuffer& imageBuffer) = 0;

	/// Renders the triangle mesh stored in the given buffer.
	virtual void renderMesh(const DefaultTriMeshGeometryBuffer& meshBuffer) = 0;

private:

	/// The current model-to-world transformation matrix.
	AffineTransformation _modelTM;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_NON_INTERACTIVE_SCENE_RENDERER_H
