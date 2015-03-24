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

#ifndef __OVITO_NON_INTERACTIVE_SCENE_RENDERER_H
#define __OVITO_NON_INTERACTIVE_SCENE_RENDERER_H

#include <core/Core.h>
#include <core/rendering/SceneRenderer.h>
#include "DefaultArrowPrimitive.h"
#include "DefaultImagePrimitive.h"
#include "DefaultLinePrimitive.h"
#include "DefaultParticlePrimitive.h"
#include "DefaultTextPrimitive.h"
#include "DefaultMeshPrimitive.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Abstract base class for non-interactive scene renderers.
 */
class OVITO_CORE_EXPORT NonInteractiveSceneRenderer : public SceneRenderer
{
public:

	/// Constructor.
	NonInteractiveSceneRenderer(DataSet* dataset) : SceneRenderer(dataset), _modelTM(AffineTransformation::Identity()) {}

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) override { _modelTM = tm; }

	/// Returns the current local-to-world transformation matrix.
	virtual const AffineTransformation& worldTransform() const override { return _modelTM; }

	/// Returns the current model-to-world transformation matrix.
	const AffineTransformation& modelTM() const { return _modelTM; }

	/// Requests a new line geometry buffer from the renderer.
	virtual std::shared_ptr<LinePrimitive> createLinePrimitive() override {
		return std::make_shared<DefaultLinePrimitive>();
	}

	/// Requests a new particle geometry buffer from the renderer.
	virtual std::shared_ptr<ParticlePrimitive> createParticlePrimitive(
			ParticlePrimitive::ShadingMode shadingMode,
			ParticlePrimitive::RenderingQuality renderingQuality,
			ParticlePrimitive::ParticleShape shape,
			bool translucentParticles) override {
		return std::make_shared<DefaultParticlePrimitive>(shadingMode, renderingQuality, shape, translucentParticles);
	}

	/// Requests a new text geometry buffer from the renderer.
	virtual std::shared_ptr<TextPrimitive> createTextPrimitive() override {
		return std::make_shared<DefaultTextPrimitive>();
	}

	/// Requests a new image geometry buffer from the renderer.
	virtual std::shared_ptr<ImagePrimitive> createImagePrimitive() override {
		return std::make_shared<DefaultImagePrimitive>();
	}

	/// Requests a new arrow geometry buffer from the renderer.
	virtual std::shared_ptr<ArrowPrimitive> createArrowPrimitive(
			ArrowPrimitive::Shape shape,
			ArrowPrimitive::ShadingMode shadingMode,
			ArrowPrimitive::RenderingQuality renderingQuality) override {
		return std::make_shared<DefaultArrowPrimitive>(shape, shadingMode, renderingQuality);
	}

	/// Requests a new triangle mesh buffer from the renderer.
	virtual std::shared_ptr<MeshPrimitive> createMeshPrimitive() override {
		return std::make_shared<DefaultMeshPrimitive>();
	}

	/// Renders the line geometry stored in the given buffer.
	virtual void renderLines(const DefaultLinePrimitive& lineBuffer) = 0;

	/// Renders the particles stored in the given buffer.
	virtual void renderParticles(const DefaultParticlePrimitive& particleBuffer) = 0;

	/// Renders the arrow elements stored in the given buffer.
	virtual void renderArrows(const DefaultArrowPrimitive& arrowBuffer) = 0;

	/// Renders the text stored in the given buffer.
	virtual void renderText(const DefaultTextPrimitive& textBuffer, const Point2& pos, int alignment) = 0;

	/// Renders the image stored in the given buffer.
	virtual void renderImage(const DefaultImagePrimitive& imageBuffer, const Point2& pos, const Vector2& size) = 0;

	/// Renders the triangle mesh stored in the given buffer.
	virtual void renderMesh(const DefaultMeshPrimitive& meshBuffer) = 0;

private:

	/// The current model-to-world transformation matrix.
	AffineTransformation _modelTM;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_NON_INTERACTIVE_SCENE_RENDERER_H
