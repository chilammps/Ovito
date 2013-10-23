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

#include <core/Core.h>
#include <core/rendering/FrameBuffer.h>
#include <core/rendering/RenderSettings.h>
#include <core/reference/CloneHelper.h>
#include <core/scene/ObjectNode.h>

#include "TachyonRenderer.h"
#include "TachyonRendererEditor.h"

extern "C" {

#include <tachyon/tachyonlib/render.h>
#include <tachyon/tachyonlib/camera.h>
#include <tachyon/tachyonlib/trace.h>

};

#if TACHYON_MAJOR_VERSION <= 0 && TACHYON_MINOR_VERSION < 99
	#error "The OVITO Tachyon plugin requires version 0.99 of the Tachyon library or higher."
#endif

namespace TachyonPlugin {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Tachyon, TachyonRenderer, NonInteractiveSceneRenderer)
SET_OVITO_OBJECT_EDITOR(TachyonRenderer, TachyonRendererEditor)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _enableAntialiasing, "EnableAntialiasing", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _enableDirectLightSource, "EnableDirectLightSource", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _enableShadows, "EnableShadows", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _antialiasingSamples, "AntialiasingSamples", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _defaultLightSourceIntensity, "DefaultLightSourceIntensity", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _enableAmbientOcclusion, "EnableAmbientOcclusion", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _ambientOcclusionSamples, "AmbientOcclusionSamples", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _ambientOcclusionBrightness, "AmbientOcclusionBrightness", PROPERTY_FIELD_MEMORIZE)
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _enableAntialiasing, "Enable anti-aliasing")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _antialiasingSamples, "Anti-aliasing samples")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _enableDirectLightSource, "Direct light")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _enableShadows, "Shadows")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _defaultLightSourceIntensity, "Direct light intensity")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _enableAmbientOcclusion, "Ambient occlusion")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _ambientOcclusionSamples, "Ambient occlusion samples")
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _ambientOcclusionBrightness, "Ambient occlusion brightness")

/******************************************************************************
* Default constructor.
******************************************************************************/
TachyonRenderer::TachyonRenderer() :
	  _enableAntialiasing(true), _enableDirectLightSource(true), _enableShadows(true),
	  _antialiasingSamples(12), _enableAmbientOcclusion(true), _ambientOcclusionSamples(12),
	  _defaultLightSourceIntensity(0.90f), _ambientOcclusionBrightness(0.80f)
{
	INIT_PROPERTY_FIELD(TachyonRenderer::_enableAntialiasing)
	INIT_PROPERTY_FIELD(TachyonRenderer::_antialiasingSamples)
	INIT_PROPERTY_FIELD(TachyonRenderer::_enableDirectLightSource)
	INIT_PROPERTY_FIELD(TachyonRenderer::_enableShadows)
	INIT_PROPERTY_FIELD(TachyonRenderer::_defaultLightSourceIntensity)
	INIT_PROPERTY_FIELD(TachyonRenderer::_enableAmbientOcclusion)
	INIT_PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionSamples)
	INIT_PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionBrightness)
}

/******************************************************************************
* Prepares the renderer for rendering of the given scene.
******************************************************************************/
bool TachyonRenderer::startRender(DataSet* dataset, RenderSettings* settings)
{
	if(!NonInteractiveSceneRenderer::startRender(dataset, settings))
		return false;

	rt_initialize(0, NULL);

	return true;
}

/******************************************************************************
* Renders a single animation frame into the given frame buffer.
******************************************************************************/
bool TachyonRenderer::renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress)
{
	progress->setLabelText(tr("Preparing scene"));

	// Create new scene and set up parameters.
	_rtscene = rt_newscene();
	rt_resolution(_rtscene, renderSettings()->outputImageWidth(), renderSettings()->outputImageHeight());
	if(_enableAntialiasing)
		rt_aa_maxsamples(_rtscene, _antialiasingSamples);

	// Create Tachyon frame buffer.
	QImage img(renderSettings()->outputImageWidth(), renderSettings()->outputImageHeight(), QImage::Format_RGB888);
	rt_rawimage_rgb24(_rtscene, img.bits());

	// Set background color.
	Color backgroundColor(renderSettings()->backgroundColorController()->getValueAtTime(time()));
	rt_background(_rtscene, rt_color(backgroundColor.r(), backgroundColor.g(), backgroundColor.b()));

	// Set equation used for rendering specular highlights.
	rt_phong_shader(_rtscene, RT_SHADER_BLINN_FAST);	// Fast version of Blinn's equation

	// Set up camera.
	if(projParams().isPerspective) {
		rt_camera_projection(_rtscene, RT_PROJECTION_PERSPECTIVE);

		// Calculate projection point and directions in camera space.
		Point3 p0 = projParams().inverseProjectionMatrix * Point3(0,0,0);
		Vector3 direction = projParams().inverseProjectionMatrix * Point3(0,0,0) - Point3::Origin();
		Vector3 up = projParams().inverseProjectionMatrix * Point3(0,1,0) - p0;
		// Transform to world space.
		p0 = Point3::Origin() + projParams().inverseViewMatrix.translation();
		direction = (projParams().inverseViewMatrix * direction).normalized();
		up = (projParams().inverseViewMatrix * up).normalized();
		rt_camera_position(_rtscene, rt_vector(p0.x(), p0.y(), -p0.z()), rt_vector(direction.x(), direction.y(), -direction.z()), rt_vector(up.x(), up.y(), -up.z()));
		rt_camera_zoom(_rtscene, 0.5 / tan(projParams().fieldOfView * 0.5));
	}
	else {
		rt_camera_projection(_rtscene, RT_PROJECTION_ORTHOGRAPHIC);

		// Calculate projection point and directions in camera space.
		Point3 p0 = projParams().inverseProjectionMatrix * Point3(0,0,0);
		Vector3 direction = projParams().inverseProjectionMatrix * Point3(0,0,1) - p0;
		Vector3 up = projParams().inverseProjectionMatrix * Point3(0,1,0) - p0;
		// Transform to world space.
		p0 = projParams().inverseViewMatrix * p0;
		direction = (projParams().inverseViewMatrix * direction).normalized();
		up = (projParams().inverseViewMatrix * up).normalized();
		p0 += direction * projParams().znear;
		rt_camera_position(_rtscene, rt_vector(p0.x(), p0.y(), -p0.z()), rt_vector(direction.x(), direction.y(), -direction.z()), rt_vector(up.x(), up.y(), -up.z()));
		rt_camera_zoom(_rtscene, 0.5 / projParams().fieldOfView);
	}

	// Set up light.
	if(_enableDirectLightSource) {
		apitexture lightTex;
		memset(&lightTex, 0, sizeof(lightTex));
		lightTex.col.r = _defaultLightSourceIntensity;
		lightTex.col.g = _defaultLightSourceIntensity;
		lightTex.col.b = _defaultLightSourceIntensity;
		lightTex.ambient = 1.0;
		lightTex.opacity = 1.0;
		lightTex.diffuse = 1.0;
		void* lightTexPtr = rt_texture(_rtscene, &lightTex);
		Vector3 lightDir = projParams().inverseViewMatrix * Vector3(0.2,-0.2,-1);
		rt_directional_light(_rtscene, lightTexPtr, rt_vector(lightDir.x(), lightDir.y(), -lightDir.z()));
	}

	if(_enableAmbientOcclusion || (_enableDirectLightSource && _enableShadows)) {
		// Full shading mode required.
		rt_shadermode(_rtscene, RT_SHADER_FULL);
	}
	else {
		// This will turn off shadows.
		rt_shadermode(_rtscene, RT_SHADER_MEDIUM);
	}

	if(_enableAmbientOcclusion) {
		apicolor skycol;
		skycol.r = _ambientOcclusionBrightness;
		skycol.g = _ambientOcclusionBrightness;
		skycol.b = _ambientOcclusionBrightness;
		rt_rescale_lights(_rtscene, 0.2);
		rt_ambient_occlusion(_rtscene, _ambientOcclusionSamples, skycol);
	}

	// Export Ovito scene objects to Tachyon scene.
	renderScene();

	// Render scene.
	progress->setMaximum(renderSettings()->outputImageWidth() * renderSettings()->outputImageHeight());
	progress->setLabelText(tr("Rendering scene"));

	scenedef * scene = (scenedef *)_rtscene;

	/* if certain key aspects of the scene parameters have been changed */
	/* since the last frame rendered, or when rendering the scene the   */
	/* first time, various setup, initialization and memory allocation  */
	/* routines need to be run in order to prepare for rendering.       */
	if (scene->scenecheck)
		rendercheck(scene);

	camera_init(scene);      /* Initialize all aspects of camera system  */

	int tileSize = scene->numthreads * 8;
	for(int ystart = 0; ystart < scene->vres && !progress->wasCanceled(); ystart += tileSize) {
		for(int xstart = 0; xstart < scene->hres && !progress->wasCanceled(); xstart += tileSize) {
			int xstop = std::min(scene->hres, xstart + tileSize);
			int ystop = std::min(scene->vres, ystart + tileSize);
			for(int thr = 0; thr < scene->numthreads; thr++) {
				thr_parms* parms = &((thr_parms *) scene->threadparms)[thr];
				parms->startx = 1 + xstart;
				parms->stopx  = xstop;
				parms->xinc   = 1;
				parms->starty = thr + 1 + ystart;
				parms->stopy  = ystop;
				parms->yinc   = scene->numthreads;
			}

			/* if using threads, wake up the child threads...  */
			rt_thread_barrier(((thr_parms *) scene->threadparms)[0].runbar, 1);

			/* Actually Ray Trace The Image */
			thread_trace(&((thr_parms *) scene->threadparms)[0]);

			// Copy rendered image back into Ovito's frame buffer.
			// Flip image since Tachyon fills the buffer upside down.
			OVITO_ASSERT(frameBuffer->image().format() == QImage::Format_ARGB32);
			int bperline = renderSettings()->outputImageWidth() * 3;
			for(int y = ystart; y < ystop; y++) {
				uchar* dst = frameBuffer->image().scanLine(frameBuffer->image().height() - 1 - y) + xstart * 4;
				uchar* src = img.bits() + y*bperline + xstart * 3;
				for(int x = xstart; x < xstop; x++, dst += 4, src += 3) {
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
					dst[3] = 255;
				}
			}
			frameBuffer->update(QRect(xstart, frameBuffer->image().height() - ystop, xstop - xstart, ystop - ystart));

			progress->setValue(progress->value() + (xstop - xstart) * (ystop - ystart));
		}
	}

	// Clean up.
	rt_deletescene(_rtscene);

	return (progress->wasCanceled() == false);
}

/******************************************************************************
* Finishes the rendering pass. This is called after all animation frames have been rendered
* or when the rendering operation has been aborted.
******************************************************************************/
void TachyonRenderer::endRender()
{
	// Shut down Tachyon library.
	rt_finalize();

	NonInteractiveSceneRenderer::endRender();
}

/******************************************************************************
* Renders the line geometry stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderLines(const DefaultLineGeometryBuffer& lineBuffer)
{

}

/******************************************************************************
* Renders the particles stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderParticles(const DefaultParticleGeometryBuffer& particleBuffer)
{
	auto p = particleBuffer.positions().begin();
	auto p_end = particleBuffer.positions().end();
	auto c = particleBuffer.colors().begin();
	auto r = particleBuffer.radii().begin();

	const AffineTransformation tm = modelTM();
	for(; p != p_end; ++p, ++c, ++r) {
		void* tex = getTachyonTexture(c->r(), c->g(), c->b());
		Point3 tp = tm * (*p);
		rt_sphere(_rtscene, tex, rt_vector(tp.x(), tp.y(), -tp.z()), *r);
	}
}

/******************************************************************************
* Renders the arrow elements stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderArrows(const DefaultArrowGeometryBuffer& arrowBuffer)
{
	const AffineTransformation tm = modelTM();
	if(arrowBuffer.shape() == ArrowGeometryBuffer::CylinderShape) {
		for(const DefaultArrowGeometryBuffer::ArrowElement& element : arrowBuffer.elements()) {
			void* tex = getTachyonTexture(element.color.r(), element.color.g(), element.color.b(), element.color.a());
			Point3 tp = tm * element.pos;
			Vector3 ta = tm * element.dir;
			rt_fcylinder(_rtscene, tex,
						   rt_vector(tp.x(), tp.y(), -tp.z()),
						   rt_vector(ta.x(), ta.y(), -ta.z()),
						   element.width);

			rt_ring(_rtscene, tex,
					rt_vector(tp.x()+ta.x(), tp.y()+ta.y(), -tp.z()-ta.z()),
					rt_vector(ta.x(), ta.y(), -ta.z()), 0, element.width);

			rt_ring(_rtscene, tex,
					rt_vector(tp.x(), tp.y(), -tp.z()),
					rt_vector(-ta.x(), -ta.y(), ta.z()), 0, element.width);
		}
	}

	else if(arrowBuffer.shape() == ArrowGeometryBuffer::ArrowShape) {
		for(const DefaultArrowGeometryBuffer::ArrowElement& element : arrowBuffer.elements()) {
			void* tex = getTachyonTexture(element.color.r(), element.color.g(), element.color.b(), element.color.a());
			FloatType arrowHeadRadius = element.width * 2.5f;
			FloatType arrowHeadLength = arrowHeadRadius * 1.8f;
			FloatType length = element.dir.length();
			if(length == 0.0f)
				continue;

			if(length > arrowHeadLength) {
				Point3 tp = tm * element.pos;
				Vector3 ta = tm * (element.dir * ((length - arrowHeadLength) / length));
				Vector3 tb = tm * (element.dir * (arrowHeadLength / length));

				rt_fcylinder(_rtscene, tex,
							   rt_vector(tp.x(), tp.y(), -tp.z()),
							   rt_vector(ta.x(), ta.y(), -ta.z()),
							   element.width);

				rt_ring(_rtscene, tex,
						rt_vector(tp.x(), tp.y(), -tp.z()),
						rt_vector(-ta.x(), -ta.y(), ta.z()), 0, element.width);

				rt_ring(_rtscene, tex,
						rt_vector(tp.x()+ta.x(), tp.y()+ta.y(), -tp.z()-ta.z()),
						rt_vector(-ta.x(), -ta.y(), ta.z()), element.width, arrowHeadRadius);

				rt_cone(_rtscene, tex,
							   rt_vector(tp.x()+ta.x()+tb.x(), tp.y()+ta.y()+tb.y(), -tp.z()-ta.z()-tb.z()),
							   rt_vector(-tb.x(), -tb.y(), tb.z()),
							   arrowHeadRadius);
			}
			else {
				FloatType r = arrowHeadRadius * length / arrowHeadLength;

				Point3 tp = tm * element.pos;
				Vector3 ta = tm * element.dir;

				rt_ring(_rtscene, tex,
						rt_vector(tp.x(), tp.y(), -tp.z()),
						rt_vector(-ta.x(), -ta.y(), ta.z()), 0, r);

				rt_cone(_rtscene, tex,
							   rt_vector(tp.x()+ta.x(), tp.y()+ta.y(), -tp.z()-ta.z()),
							   rt_vector(-ta.x(), -ta.y(), ta.z()),
							   r);
			}
		}
	}
}

/******************************************************************************
* Renders the text stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderText(const DefaultTextGeometryBuffer& textBuffer)
{
	// Not supported by this renderer.
}

/******************************************************************************
* Renders the image stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderImage(const DefaultImageGeometryBuffer& imageBuffer)
{
	// Not supported by this renderer.
}

/******************************************************************************
* Renders the triangle mesh stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderMesh(const DefaultTriMeshGeometryBuffer& meshBuffer)
{
	// Not supported by this renderer.
}

/******************************************************************************
* Creates a texture with the given color.
******************************************************************************/
void* TachyonRenderer::getTachyonTexture(FloatType r, FloatType g, FloatType b, FloatType alpha)
{
	apitexture tex;
	memset(&tex, 0, sizeof(tex));
	tex.ambient  = 0.3;
	tex.diffuse  = 0.8;
	tex.specular = 0.0;
	tex.opacity  = alpha;
	tex.col.r = r;
	tex.col.g = g;
	tex.col.b = b;
	tex.texturefunc = RT_TEXTURE_CONSTANT;

	return rt_texture(_rtscene, &tex);
}

};

