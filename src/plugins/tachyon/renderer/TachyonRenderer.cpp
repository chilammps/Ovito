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

#include <tachyon/render.h>
#include <tachyon/camera.h>
#include <tachyon/threads.h>
#include <tachyon/trace.h>

};

#if TACHYON_MAJOR_VERSION <= 0 && TACHYON_MINOR_VERSION < 99
	#error "The OVITO Tachyon plugin requires version 0.99 or newer of the Tachyon library."
#endif

namespace Ovito { namespace Tachyon {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Tachyon, TachyonRenderer, NonInteractiveSceneRenderer);
SET_OVITO_OBJECT_EDITOR(TachyonRenderer, TachyonRendererEditor);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _antialiasingEnabled, "EnableAntialiasing", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _directLightSourceEnabled, "EnableDirectLightSource", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _shadowsEnabled, "EnableShadows", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _antialiasingSamples, "AntialiasingSamples", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _defaultLightSourceIntensity, "DefaultLightSourceIntensity", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _ambientOcclusionEnabled, "EnableAmbientOcclusion", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _ambientOcclusionSamples, "AmbientOcclusionSamples", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(TachyonRenderer, _ambientOcclusionBrightness, "AmbientOcclusionBrightness", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _antialiasingEnabled, "Enable anti-aliasing");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _antialiasingSamples, "Anti-aliasing samples");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _directLightSourceEnabled, "Direct light");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _shadowsEnabled, "Shadows");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _defaultLightSourceIntensity, "Direct light intensity");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _ambientOcclusionEnabled, "Ambient occlusion");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _ambientOcclusionSamples, "Ambient occlusion samples");
SET_PROPERTY_FIELD_LABEL(TachyonRenderer, _ambientOcclusionBrightness, "Ambient occlusion brightness");

/******************************************************************************
* Default constructor.
******************************************************************************/
TachyonRenderer::TachyonRenderer(DataSet* dataset) : NonInteractiveSceneRenderer(dataset),
		_antialiasingEnabled(true), _directLightSourceEnabled(true), _shadowsEnabled(true),
	  _antialiasingSamples(12), _ambientOcclusionEnabled(true), _ambientOcclusionSamples(12),
	  _defaultLightSourceIntensity(0.90f), _ambientOcclusionBrightness(0.80f)
{
	INIT_PROPERTY_FIELD(TachyonRenderer::_antialiasingEnabled);
	INIT_PROPERTY_FIELD(TachyonRenderer::_antialiasingSamples);
	INIT_PROPERTY_FIELD(TachyonRenderer::_directLightSourceEnabled);
	INIT_PROPERTY_FIELD(TachyonRenderer::_shadowsEnabled);
	INIT_PROPERTY_FIELD(TachyonRenderer::_defaultLightSourceIntensity);
	INIT_PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionEnabled);
	INIT_PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionSamples);
	INIT_PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionBrightness);
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
	if(progress) progress->setLabelText(tr("Preparing scene"));

	// Create new scene and set up parameters.
	_rtscene = rt_newscene();
	rt_resolution(_rtscene, renderSettings()->outputImageWidth(), renderSettings()->outputImageHeight());
	if(antialiasingEnabled())
		rt_aa_maxsamples(_rtscene, antialiasingSamples());
	//rt_normal_fixup_mode(_rtscene, 2);

	// Create Tachyon frame buffer.
	QImage img(renderSettings()->outputImageWidth(), renderSettings()->outputImageHeight(), QImage::Format_RGB888);
	rt_rawimage_rgb24(_rtscene, img.bits());

	// Set background color.
	TimeInterval iv;
	Color backgroundColor;
	renderSettings()->backgroundColorController()->getColorValue(time(), backgroundColor, iv);
	rt_background(_rtscene, rt_color(backgroundColor.r(), backgroundColor.g(), backgroundColor.b()));

	// Set equation used for rendering specular highlights.
	rt_phong_shader(_rtscene, RT_SHADER_NULL_PHONG);

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
		Point3 p0 = projParams().inverseProjectionMatrix * Point3(0,0,-1);
		Vector3 direction = projParams().inverseProjectionMatrix * Point3(0,0,1) - p0;
		Vector3 up = projParams().inverseProjectionMatrix * Point3(0,1,-1) - p0;
		// Transform to world space.
		p0 = projParams().inverseViewMatrix * p0;
		direction = (projParams().inverseViewMatrix * direction).normalized();
		up = (projParams().inverseViewMatrix * up).normalized();
		p0 += direction * projParams().znear;

		rt_camera_position(_rtscene, rt_vector(p0.x(), p0.y(), -p0.z()), rt_vector(direction.x(), direction.y(), -direction.z()), rt_vector(up.x(), up.y(), -up.z()));
		rt_camera_zoom(_rtscene, 0.5 / projParams().fieldOfView);
	}

	// Set up light.
	if(directLightSourceEnabled()) {
		apitexture lightTex;
		memset(&lightTex, 0, sizeof(lightTex));
		lightTex.col.r = lightTex.col.g = lightTex.col.b = defaultLightSourceIntensity();
		lightTex.ambient = 1.0;
		lightTex.opacity = 1.0;
		lightTex.diffuse = 1.0;
		void* lightTexPtr = rt_texture(_rtscene, &lightTex);
		Vector3 lightDir = projParams().inverseViewMatrix * Vector3(0.2f,-0.2f,-1.0f);
		rt_directional_light(_rtscene, lightTexPtr, rt_vector(lightDir.x(), lightDir.y(), -lightDir.z()));
	}

	if(ambientOcclusionEnabled() || (directLightSourceEnabled() && shadowsEnabled())) {
		// Full shading mode required.
		rt_shadermode(_rtscene, RT_SHADER_FULL);
	}
	else {
		// This will turn off shadows.
		rt_shadermode(_rtscene, RT_SHADER_MEDIUM);
	}

	if(ambientOcclusionEnabled()) {
		apicolor skycol;
		skycol.r = skycol.g = skycol.b = ambientOcclusionBrightness();
		rt_rescale_lights(_rtscene, 0.2);
		rt_ambient_occlusion(_rtscene, ambientOcclusionSamples(), skycol);
	}

	rt_trans_mode(_rtscene, RT_TRANS_VMD);
	rt_trans_max_surfaces(_rtscene, 4);

	// Export Ovito data objects to Tachyon scene.
	renderScene();

	// Render visual 3D representation of the modifiers.
	renderModifiers(false);

	// Render visual 2D representation of the modifiers.
	renderModifiers(true);

	// Render scene.
	if(progress) {
		progress->setMaximum(renderSettings()->outputImageWidth() * renderSettings()->outputImageHeight());
		progress->setLabelText(tr("Rendering scene"));
	}

	scenedef * scene = (scenedef *)_rtscene;

	/* if certain key aspects of the scene parameters have been changed */
	/* since the last frame rendered, or when rendering the scene the   */
	/* first time, various setup, initialization and memory allocation  */
	/* routines need to be run in order to prepare for rendering.       */
	if (scene->scenecheck)
		rendercheck(scene);

	camera_init(scene);      /* Initialize all aspects of camera system  */

	// Make sure the target frame buffer has the right memory format.
	if(frameBuffer->image().format() != QImage::Format_ARGB32)
		frameBuffer->image() = frameBuffer->image().convertToFormat(QImage::Format_ARGB32);

	int tileSize = scene->numthreads * 8;
	for(int ystart = 0; ystart < scene->vres; ystart += tileSize) {
		for(int xstart = 0; xstart < scene->hres; xstart += tileSize) {
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

			if(progress) {
				progress->setValue(progress->value() + (xstop - xstart) * (ystop - ystart));
				if(progress->wasCanceled())
					break;
			}
		}

		if(progress && progress->wasCanceled())
			break;
	}

	// Execute recorded overlay draw calls.
	QPainter painter(&frameBuffer->image());
	for(const auto& imageCall : _imageDrawCalls) {
		QRectF rect(std::get<1>(imageCall).x(), std::get<1>(imageCall).y(), std::get<2>(imageCall).x(), std::get<2>(imageCall).y());
		painter.drawImage(rect, std::get<0>(imageCall));
		frameBuffer->update(rect.toAlignedRect());
	}
	for(const auto& textCall : _textDrawCalls) {
		QRectF pos(std::get<3>(textCall).x(), std::get<3>(textCall).y(), 0, 0);
		painter.setPen(std::get<1>(textCall));
		painter.setFont(std::get<2>(textCall));
		QRectF boundingRect;
		painter.drawText(pos, std::get<4>(textCall) | Qt::TextSingleLine | Qt::TextDontClip, std::get<0>(textCall), &boundingRect);
		frameBuffer->update(boundingRect.toAlignedRect());
	}

	// Clean up.
	rt_deletescene(_rtscene);

	return (!progress || progress->wasCanceled() == false);
}

/******************************************************************************
* Finishes the rendering pass. This is called after all animation frames have been rendered
* or when the rendering operation has been aborted.
******************************************************************************/
void TachyonRenderer::endRender()
{
	// Shut down Tachyon library.
	rt_finalize();

	// Release draw call buffers.
	_imageDrawCalls.clear();
	_textDrawCalls.clear();

	NonInteractiveSceneRenderer::endRender();
}

/******************************************************************************
* Renders the line geometry stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderLines(const DefaultLinePrimitive& lineBuffer)
{
	// Lines are not supported by this renderer.
}

/******************************************************************************
* Renders the particles stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderParticles(const DefaultParticlePrimitive& particleBuffer)
{
	auto p = particleBuffer.positions().begin();
	auto p_end = particleBuffer.positions().end();
	auto c = particleBuffer.colors().begin();
	auto r = particleBuffer.radii().begin();

	const AffineTransformation tm = modelTM();

	if(particleBuffer.particleShape() == ParticlePrimitive::SphericalShape) {
		// Rendering spherical particles.
		for(; p != p_end; ++p, ++c, ++r) {
			void* tex = getTachyonTexture(c->r(), c->g(), c->b(), c->a());
			Point3 tp = tm * (*p);
			rt_sphere(_rtscene, tex, rt_vector(tp.x(), tp.y(), -tp.z()), *r);
		}
	}
	else if(particleBuffer.particleShape() == ParticlePrimitive::SquareShape) {
		// Rendering cubic particles.
		for(; p != p_end; ++p, ++c, ++r) {
			void* tex = getTachyonTexture(c->r(), c->g(), c->b(), c->a());
			Point3 tp = tm * (*p);
			rt_box(_rtscene, tex, rt_vector(tp.x() - *r, tp.y() - *r, -tp.z() - *r), rt_vector(tp.x() + *r, tp.y() + *r, -tp.z() + *r));
		}
	}
	else if(particleBuffer.particleShape() == ParticlePrimitive::BoxShape) {
		// Rendering noncubic box particles.
		auto shape = particleBuffer.shapes().begin();
		auto shape_end = particleBuffer.shapes().end();
		for(; p != p_end && shape != shape_end; ++p, ++c, ++shape) {
			void* tex = getTachyonTexture(c->r(), c->g(), c->b(), c->a());
			Point3 tp = tm * (*p);
			rt_box(_rtscene, tex, rt_vector(tp.x() - shape->x(), tp.y() - shape->y(), -tp.z() - shape->z()),
					rt_vector(tp.x() + shape->x(), tp.y() + shape->y(), -tp.z() + shape->z()));
		}
	}
}

/******************************************************************************
* Renders the arrow elements stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderArrows(const DefaultArrowPrimitive& arrowBuffer)
{
	const AffineTransformation tm = modelTM();
	if(arrowBuffer.shape() == ArrowPrimitive::CylinderShape) {
		for(const DefaultArrowPrimitive::ArrowElement& element : arrowBuffer.elements()) {
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

	else if(arrowBuffer.shape() == ArrowPrimitive::ArrowShape) {
		for(const DefaultArrowPrimitive::ArrowElement& element : arrowBuffer.elements()) {
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
void TachyonRenderer::renderText(const DefaultTextPrimitive& textBuffer, const Point2& pos, int alignment)
{
	_textDrawCalls.push_back(std::make_tuple(textBuffer.text(), textBuffer.color(), textBuffer.font(), pos, alignment));
}

/******************************************************************************
* Renders the image stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderImage(const DefaultImagePrimitive& imageBuffer, const Point2& pos, const Vector2& size)
{
	_imageDrawCalls.push_back(std::make_tuple(imageBuffer.image(), pos, size));
}

/******************************************************************************
* Renders the triangle mesh stored in the given buffer.
******************************************************************************/
void TachyonRenderer::renderMesh(const DefaultMeshPrimitive& meshBuffer)
{
	// Stores data of a single vertex passed to Tachyon.
	struct ColoredVertexWithNormal {
		ColorAT<float> color;
		Vector_3<float> normal;
		Point_3<float> pos;
	};

	const TriMesh& mesh = meshBuffer.mesh();

	// Allocate render vertex buffer.
	int renderVertexCount = mesh.faceCount() * 3;
	if(renderVertexCount == 0)
		return;
	std::vector<ColoredVertexWithNormal> renderVertices(renderVertexCount);

	const AffineTransformation tm = modelTM();
	const Matrix3 normalTM = modelTM().linear().inverse().transposed();
	quint32 allMask = 0;

	// Compute face normals.
	std::vector<Vector_3<float>> faceNormals(mesh.faceCount());
	auto faceNormal = faceNormals.begin();
	for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++faceNormal) {
		const Point3& p0 = mesh.vertex(face->vertex(0));
		Vector3 d1 = mesh.vertex(face->vertex(1)) - p0;
		Vector3 d2 = mesh.vertex(face->vertex(2)) - p0;
		*faceNormal = normalTM * d1.cross(d2);
		if(*faceNormal != Vector_3<float>::Zero()) {
			faceNormal->normalize();
			allMask |= face->smoothingGroups();
		}
	}

	// Initialize render vertices.
	std::vector<ColoredVertexWithNormal>::iterator rv = renderVertices.begin();
	faceNormal = faceNormals.begin();
	ColorAT<float> defaultVertexColor = ColorAT<float>(meshBuffer.meshColor());
	for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++faceNormal) {

		// Initialize render vertices for this face.
		for(size_t v = 0; v < 3; v++, rv++) {
			if(face->smoothingGroups())
				rv->normal = Vector_3<float>::Zero();
			else
				rv->normal = *faceNormal;
			rv->pos = tm * mesh.vertex(face->vertex(v));
			if(mesh.hasVertexColors())
				rv->color = ColorAT<float>(mesh.vertexColor(face->vertex(v)));
			else if(mesh.hasFaceColors())
				rv->color = ColorAT<float>(mesh.faceColor(face - mesh.faces().constBegin()));
			else
				rv->color = defaultVertexColor;
		}
	}

	if(allMask) {
		std::vector<Vector_3<float>> groupVertexNormals(mesh.vertexCount());
		for(int group = 0; group < OVITO_MAX_NUM_SMOOTHING_GROUPS; group++) {
			quint32 groupMask = quint32(1) << group;
            if((allMask & groupMask) == 0) continue;

			// Reset work arrays.
            std::fill(groupVertexNormals.begin(), groupVertexNormals.end(), Vector_3<float>::Zero());

			// Compute vertex normals at original vertices for current smoothing group.
            faceNormal = faceNormals.begin();
			for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++faceNormal) {
				// Skip faces which do not belong to the current smoothing group.
				if((face->smoothingGroups() & groupMask) == 0) continue;

				// Add face's normal to vertex normals.
				for(size_t fv = 0; fv < 3; fv++)
					groupVertexNormals[face->vertex(fv)] += *faceNormal;
			}

			// Transfer vertex normals from original vertices to render vertices.
			rv = renderVertices.begin();
			for(const auto& face : mesh.faces()) {
				if(face.smoothingGroups() & groupMask) {
					for(size_t fv = 0; fv < 3; fv++, ++rv)
						rv->normal += groupVertexNormals[face.vertex(fv)];
				}
				else rv += 3;
			}
		}
	}

	// Pass transformed triangles to Tachyon renderer.
	void* tex = getTachyonTexture(1.0f, 1.0f, 1.0f, defaultVertexColor.a());
	for(auto rv = renderVertices.begin(); rv != renderVertices.end(); ) {
		auto rv0 = rv++;
		auto rv1 = rv++;
		auto rv2 = rv++;

		if(mesh.hasVertexColors() || mesh.hasFaceColors())
			tex = getTachyonTexture(1.0f, 1.0f, 1.0f, defaultVertexColor.a());

		rt_vcstri(_rtscene, tex,
				rt_vector(rv0->pos.x(), rv0->pos.y(), -rv0->pos.z()),
				rt_vector(rv1->pos.x(), rv1->pos.y(), -rv1->pos.z()),
				rt_vector(rv2->pos.x(), rv2->pos.y(), -rv2->pos.z()),
				rt_vector(-rv0->normal.x(), -rv0->normal.y(), rv0->normal.z()),
				rt_vector(-rv1->normal.x(), -rv1->normal.y(), rv1->normal.z()),
				rt_vector(-rv2->normal.x(), -rv2->normal.y(), rv2->normal.z()),
				rt_color(rv0->color.r(), rv0->color.g(), rv0->color.b()),
				rt_color(rv1->color.r(), rv1->color.g(), rv1->color.b()),
				rt_color(rv2->color.r(), rv2->color.g(), rv2->color.b()));
	}
}

/******************************************************************************
* Creates a texture with the given color.
******************************************************************************/
void* TachyonRenderer::getTachyonTexture(FloatType r, FloatType g, FloatType b, FloatType alpha)
{
	apitexture tex;
	memset(&tex, 0, sizeof(tex));
	tex.ambient  = FloatType(0.3);
	tex.diffuse  = FloatType(0.8);
	tex.specular = FloatType(0.0);
	tex.opacity  = alpha;
	tex.col.r = r;
	tex.col.g = g;
	tex.col.b = b;
	tex.texturefunc = RT_TEXTURE_CONSTANT;

	return rt_texture(_rtscene, &tex);
}

}	// End of namespace
}	// End of namespace

