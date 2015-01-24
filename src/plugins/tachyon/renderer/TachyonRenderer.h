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
#include <tachyon/tachyon.h>

namespace Ovito { namespace Tachyon {

/**
 * \brief A scene renderer that is based on the Tachyon open source ray-tracing engine
 */
class TachyonRenderer : public NonInteractiveSceneRenderer
{
public:

	/// Constructor.
	Q_INVOKABLE TachyonRenderer(DataSet* dataset);

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
	virtual void renderLines(const DefaultLinePrimitive& lineBuffer) override;

	/// Renders the particles stored in the given buffer.
	virtual void renderParticles(const DefaultParticlePrimitive& particleBuffer) override;

	/// Renders the arrow elements stored in the given buffer.
	virtual void renderArrows(const DefaultArrowPrimitive& arrowBuffer) override;

	/// Renders the text stored in the given buffer.
	virtual void renderText(const DefaultTextPrimitive& textBuffer, const Point2& pos, int alignment) override;

	/// Renders the image stored in the given buffer.
	virtual void renderImage(const DefaultImagePrimitive& imageBuffer, const Point2& pos, const Vector2& size) override;

	/// Renders the triangle mesh stored in the given buffer.
	virtual void renderMesh(const DefaultMeshPrimitive& meshBuffer) override;

	/// Returns whether anti-aliasing is enabled.
	bool antialiasingEnabled() const { return _antialiasingEnabled; }

	/// Enables/disables anti-aliasing.
	void setAntialiasingEnabled(bool on) { _antialiasingEnabled = on; }

	/// Returns the quality level of anti-aliasing.
	int antialiasingSamples() const { return _antialiasingSamples; }

	/// Sets the quality level of anti-aliasing.
	void setAntialiasingSamples(int sampleCount) { _antialiasingSamples = sampleCount; }

	/// Returns whether the default direct light source is enabled.
	bool directLightSourceEnabled() const { return _directLightSourceEnabled; }

	/// Enables/disables the default direct light source.
	void setDirectLightSourceEnabled(bool on) { _directLightSourceEnabled = on; }

	/// Returns the brightness of the default direct light source.
	FloatType defaultLightSourceIntensity() const { return _defaultLightSourceIntensity; }

	/// Sets the brightness of the default direct light source.
	void setDefaultLightSourceIntensity(FloatType brightness) { _defaultLightSourceIntensity = brightness; }

	/// Returns whether the calculation of shadows is enabled.
	bool shadowsEnabled() const { return _shadowsEnabled; }

	/// Enables/disables the calculation of shadows.
	void setShadowsEnabled(bool on) { _shadowsEnabled = on; }

	/// Returns whether the calculation of ambient occlusion is enabled.
	bool ambientOcclusionEnabled() const { return _ambientOcclusionEnabled; }

	/// Enables/disables the calculation of ambient occlusion.
	void setAmbientOcclusionEnabled(bool on) { _ambientOcclusionEnabled = on; }

	/// Returns the brightness of the ambient occlusion sky light source.
	FloatType ambientOcclusionBrightness() const { return _ambientOcclusionBrightness; }

	/// Sets the brightness of the ambient occlusion sky light source.
	void setAmbientOcclusionBrightness(FloatType brightness) { _ambientOcclusionBrightness = brightness; }

	/// Returns the number of AO samples to compute.
	int ambientOcclusionSamples() const { return _ambientOcclusionSamples; }

	/// Sets the number of AO samples to compute.
	void setAmbientOcclusionSamples(int sampleCount) { _ambientOcclusionSamples = sampleCount; }

private:

	/// Creates a texture with the given color.
	void* getTachyonTexture(FloatType r, FloatType g, FloatType b, FloatType alpha = FloatType(1));

private:

	/// Controls anti-aliasing.
	PropertyField<bool> _antialiasingEnabled;

	/// Controls quality of anti-aliasing.
	PropertyField<int> _antialiasingSamples;

	/// Enables direct light source.
	PropertyField<bool> _directLightSourceEnabled;

	/// Enables shadows for the direct light source.
	PropertyField<bool> _shadowsEnabled;

	/// Controls the brightness of the default direct light source.
	PropertyField<FloatType> _defaultLightSourceIntensity;

	/// Enables ambient occlusion lighting.
	PropertyField<bool> _ambientOcclusionEnabled;

	/// Controls quality of ambient occlusion.
	PropertyField<int> _ambientOcclusionSamples;

	/// Controls the brightness of the sky light source used for ambient occlusion.
	PropertyField<FloatType> _ambientOcclusionBrightness;

	/// The Tachyon internal scene handle.
	SceneHandle _rtscene;

	/// List of image primitives that need to be painted over the final image.
	std::vector<std::tuple<QImage,Point2,Vector2>> _imageDrawCalls;

	/// List of text primitives that need to be painted over the final image.
	std::vector<std::tuple<QString,ColorA,QFont,Point2,int>> _textDrawCalls;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Tachyon renderer");

	DECLARE_PROPERTY_FIELD(_antialiasingEnabled);
	DECLARE_PROPERTY_FIELD(_antialiasingSamples);
	DECLARE_PROPERTY_FIELD(_directLightSourceEnabled);
	DECLARE_PROPERTY_FIELD(_shadowsEnabled);
	DECLARE_PROPERTY_FIELD(_defaultLightSourceIntensity);
	DECLARE_PROPERTY_FIELD(_ambientOcclusionEnabled);
	DECLARE_PROPERTY_FIELD(_ambientOcclusionSamples);
	DECLARE_PROPERTY_FIELD(_ambientOcclusionBrightness);
};

}	// End of namespace
}	// End of namespace

#endif // __OVITO_TACHYON_RENDERER_H
