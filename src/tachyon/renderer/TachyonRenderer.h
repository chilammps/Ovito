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

#ifndef __OVITO_TACHYON_RENDERER_H
#define __OVITO_TACHYON_RENDERER_H

#include <core/Core.h>
#include <core/rendering/noninteractive/NonInteractiveSceneRenderer.h>
#define TACHYON_INTERNAL 1
#include <tachyon/tachyonlib/tachyon.h>

namespace TachyonPlugin {

using namespace Ovito;

/*
 * A scene renderer that is based on the Tachyon open source ray-tracing engine
 */
class TachyonRenderer : public NonInteractiveSceneRenderer
{
public:

	/// Default constructor.
	Q_INVOKABLE TachyonRenderer();

	///	Prepares the renderer for rendering of the given scene.
	/// Throws an exception on error. Returns false when the operation has been aborted by the user.
	virtual bool startRender(DataSet* dataset, RenderSettings* settings) override;

	/// Renders a single animation frame into the given frame buffer.
	/// Throws an exception on error. Returns false when the operation has been aborted by the user.
	virtual bool renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress) override;

	///	Finishes the rendering pass. This is called after all animation frames have been rendered
	/// or when the rendering operation has been aborted.
	virtual void endRender() override;

	/// Renders the line geometry stored in the given buffer.
	virtual void renderLines(const DefaultLineGeometryBuffer& lineBuffer) override;

	/// Renders the particles stored in the given buffer.
	virtual void renderParticles(const DefaultParticleGeometryBuffer& particleBuffer) override;

	/// Renders the arrow elements stored in the given buffer.
	virtual void renderArrows(const DefaultArrowGeometryBuffer& arrowBuffer) override;

	/// Renders the text stored in the given buffer.
	virtual void renderText(const DefaultTextGeometryBuffer& textBuffer) override;

	/// Renders the image stored in the given buffer.
	virtual void renderImage(const DefaultImageGeometryBuffer& imageBuffer) override;

private:

	/// Creates a texture with the given color.
	void* getTachyonTexture(FloatType r, FloatType g, FloatType b, FloatType alpha = FloatType(1));

private:

	/// Controls anti-aliasing.
	PropertyField<bool> _enableAntialiasing;

	/// Controls quality of anti-aliasing.
	PropertyField<int> _antialiasingSamples;

	/// Enables direct light source.
	PropertyField<bool> _enableDirectLightSource;

	/// Enables shadows for the direct light source.
	PropertyField<bool> _enableShadows;

	/// Controls the brightness of the default direct light source.
	PropertyField<FloatType> _defaultLightSourceIntensity;

	/// Enables ambient occlusion lighting.
	PropertyField<bool> _enableAmbientOcclusion;

	/// Controls quality of ambient occlusion.
	PropertyField<int> _ambientOcclusionSamples;

	/// Controls the brightness of the sky light source used for ambient occlusion.
	PropertyField<FloatType> _ambientOcclusionBrightness;

	/// The Tachyon internal scene handle.
	SceneHandle _rtscene;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Tachyon renderer");

	DECLARE_PROPERTY_FIELD(_enableAntialiasing);
	DECLARE_PROPERTY_FIELD(_antialiasingSamples);
	DECLARE_PROPERTY_FIELD(_enableDirectLightSource);
	DECLARE_PROPERTY_FIELD(_enableShadows);
	DECLARE_PROPERTY_FIELD(_defaultLightSourceIntensity);
	DECLARE_PROPERTY_FIELD(_enableAmbientOcclusion);
	DECLARE_PROPERTY_FIELD(_ambientOcclusionSamples);
	DECLARE_PROPERTY_FIELD(_ambientOcclusionBrightness);
};

};	// End of namespace

#endif // __OVITO_TACHYON_RENDERER_H
