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

#ifndef __OVITO_VIEWPORT_SCENE_RENDERER_H
#define __OVITO_VIEWPORT_SCENE_RENDERER_H

#include <core/Core.h>
#include <core/rendering/SceneRenderer.h>
#include "OpenGLHelpers.h"

#include <QOpenGLFunctions_1_4>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_3_2_Core>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief This is the default scene renderer used to render the contents
 *        of the interactive viewports.
 */
class OVITO_CORE_EXPORT ViewportSceneRenderer : public SceneRenderer
{
public:

	/// Default constructor.
	ViewportSceneRenderer(DataSet* dataset) : SceneRenderer(dataset),
		_glcontext(nullptr),
		_modelViewTM(AffineTransformation::Identity()),
		_glVertexIDBufferSize(-1) {}

	/// Renders the current animation frame.
	virtual bool renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress) override;

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// This method is called after renderFrame() has been called.
	virtual void endFrame() override;

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) override;

	/// Returns the current local-to-world transformation matrix.
	virtual const AffineTransformation& worldTransform() const override {
		return _modelWorldTM;
	}

	/// Returns the current model-to-view transformation matrix.
	const AffineTransformation& modelViewTM() const { return _modelViewTM; }

	/// \brief Computes the bounding box of the the 3D visual elements
	///        shown only in the interactive viewports.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the world coordinate system that contains
	///         everything to be rendered.
	Box3 boundingBoxInteractive(TimePoint time, Viewport* viewport);

	/// Requests a new line geometry buffer from the renderer.
	virtual std::shared_ptr<LinePrimitive> createLinePrimitive() override;

	/// Requests a new particle geometry buffer from the renderer.
	virtual std::shared_ptr<ParticlePrimitive> createParticlePrimitive(ParticlePrimitive::ShadingMode shadingMode,
			ParticlePrimitive::RenderingQuality renderingQuality, ParticlePrimitive::ParticleShape shape,
			bool translucentParticles) override;

	/// Requests a new text geometry buffer from the renderer.
	virtual std::shared_ptr<TextPrimitive> createTextPrimitive() override;

	/// Requests a new image geometry buffer from the renderer.
	virtual std::shared_ptr<ImagePrimitive> createImagePrimitive() override;

	/// Requests a new arrow geometry buffer from the renderer.
	virtual std::shared_ptr<ArrowPrimitive> createArrowPrimitive(ArrowPrimitive::Shape shape,
			ArrowPrimitive::ShadingMode shadingMode,
			ArrowPrimitive::RenderingQuality renderingQuality) override;

	/// Requests a new triangle mesh buffer from the renderer.
	virtual std::shared_ptr<MeshPrimitive> createMeshPrimitive() override;

	/// Renders a 2d polyline in the viewport.
	void render2DPolyline(const Point2* points, int count, const ColorA& color, bool closed);

	/// Returns whether this renderer is rendering an interactive viewport.
	/// \return true if rendering a real-time viewport; false if rendering an output image.
	virtual bool isInteractive() const override { return true; }

	/// Returns the supersampling level.
	virtual int antialiasingLevel() const { return 1; }

	/// Returns the final size of the rendered image in pixels.
	virtual QSize outputSize() const override;

	/// Returns the OpenGL context this renderer uses.
	QOpenGLContext* glcontext() const { return _glcontext; }

	/// Returns a pointer to the OpenGL functions object.
	QOpenGLFunctions* glfuncs() const { return _glFunctions; }

	/// Returns the surface format of the current OpenGL context.
	const QSurfaceFormat& glformat() const { return _glformat; }

	/// Indicates whether the current OpenGL implementation is according to the core profile.
	bool isCoreProfile() const { return _isCoreProfile; }

	/// Indicates whether it is okay to use OpenGL point sprites. Otherwise emulate them using explicit triangle geometry.
	bool usePointSprites() const { return _usePointSprites; }

	/// Indicates whether it is okay to use GLSL geometry shaders.
	bool useGeometryShaders() const { return _useGeometryShaders; }

	/// Translates an OpenGL error code to a human-readable message string.
	static const char* openglErrorString(GLenum errorCode);

	/// Loads and compiles an OpenGL shader program.
	QOpenGLShaderProgram* loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile = QString());

	/// The OpenGL glPointParameterf() function.
	void glPointParameterf(GLenum pname, GLfloat param) {
		if(_glFunctions32) _glFunctions32->glPointParameterf(pname, param);
		else if(_glFunctions30) _glFunctions30->glPointParameterf(pname, param);
		else if(_glFunctions20) _glFunctions20->glPointParameterf(pname, param);
		else if(_glFunctions14) _glFunctions14->glPointParameterf(pname, param);
	}

	/// The OpenGL glPointParameterfv() function.
	void glPointParameterfv(GLenum pname, const GLfloat* params) {
		if(_glFunctions32) _glFunctions32->glPointParameterfv(pname, params);
		else if(_glFunctions30) _glFunctions30->glPointParameterfv(pname, params);
		else if(_glFunctions20) _glFunctions20->glPointParameterfv(pname, params);
		else if(_glFunctions14) _glFunctions14->glPointParameterfv(pname, params);
	}

	/// The OpenGL glMultiDrawArrays() function.
	void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount) {
		if(_glFunctions32) _glFunctions32->glMultiDrawArrays(mode, first, count, drawcount);
		else if(_glFunctions30) _glFunctions30->glMultiDrawArrays(mode, first, count, drawcount);
		else if(_glFunctions20) _glFunctions20->glMultiDrawArrays(mode, first, count, drawcount);
		else if(_glFunctions14) _glFunctions14->glMultiDrawArrays(mode, first, count, drawcount);
	}

	/// Make sure vertex IDs are available to use by the OpenGL shader.
	void activateVertexIDs(QOpenGLShaderProgram* shader, GLint vertexCount, bool alwaysUseVBO = false);

	/// This needs to be called to deactivate vertex IDs, which were activated by a call to activateVertexIDs().
	void deactivateVertexIDs(QOpenGLShaderProgram* shader, bool alwaysUseVBO = false);

	/// Registers a range of sub-IDs belonging to the current object being rendered.
	/// This is an internal method used by the PickingSceneRenderer class to implement the picking mechanism.
	virtual quint32 registerSubObjectIDs(quint32 subObjectCount) { return 0; }

	/// Returns the line rendering width to use in object picking mode.
	virtual FloatType defaultLinePickingWidth() override;

	/// Returns the default OpenGL surface format requested by OVITO when creating OpenGL contexts.
	static QSurfaceFormat getDefaultSurfaceFormat();

	/// Returns whether we are currently rendering translucent objects.
	bool translucentPass() const { return _translucentPass; }

	/// Adds a primitive to the list of translucent primitives which will be rendered during the second
	/// rendering pass.
	void registerTranslucentPrimitive(const std::shared_ptr<PrimitiveBase>& primitive) {
		OVITO_ASSERT(!translucentPass());
		_translucentPrimitives.emplace_back(worldTransform(), primitive);
	}

	/// Binds the default vertex array object again in case another VAO was bound in between.
	/// This method should be called before calling an OpenGL rendering function.
	void rebindVAO() {
		if(_vertexArrayObject) _vertexArrayObject->bind();
	}

protected:

	/// \brief Loads and compiles a GLSL shader and adds it to the given program object.
	void loadShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderType shaderType, const QString& filename);

	/// Determines the range of the construction grid to display.
	std::tuple<FloatType, Box2I> determineGridRange(Viewport* vp);

	/// Renders the construction grid in a viewport.
	void renderGrid();

private:

	/// The OpenGL context this renderer uses.
	QOpenGLContext* _glcontext;

	/// The OpenGL functions object.
	QOpenGLFunctions* _glFunctions;

	/// The OpenGL 1.4 functions object.
	QOpenGLFunctions_1_4* _glFunctions14;

	/// The OpenGL 2.0 functions object.
	QOpenGLFunctions_2_0* _glFunctions20;

	/// The OpenGL 3.0 functions object.
	QOpenGLFunctions_3_0* _glFunctions30;

	/// The OpenGL 3.2 core profile functions object.
	QOpenGLFunctions_3_2_Core* _glFunctions32;

	/// The OpenGL vertex array object that is required by OpenGL 3.2 core profile.
	QScopedPointer<QOpenGLVertexArrayObject> _vertexArrayObject;

	/// The OpenGL surface format.
	QSurfaceFormat _glformat;

	/// Indicates whether the current OpenGL implementation is based on the core or the compatibility profile.
	bool _isCoreProfile;

	/// Indicates whether it is okay to use OpenGL point sprites. Otherwise emulate them using explicit triangle geometry.
	bool _usePointSprites;

	/// Indicates whether it is okay to use GLSL geometry shaders.
	bool _useGeometryShaders;

	/// The current model-to-world transformation matrix.
	AffineTransformation _modelWorldTM;

	/// The current model-to-view transformation matrix.
	AffineTransformation _modelViewTM;

	/// The internal OpenGL vertex buffer that stores vertex IDs.
	QOpenGLBuffer _glVertexIDBuffer;

	/// The number of IDs stored in the OpenGL buffer.
	GLint _glVertexIDBufferSize;

	/// The geometry buffer used to render the construction grid of a viewport.
	std::shared_ptr<LinePrimitive> _constructionGridGeometry;

	/// Indicates that we are currently rendering the translucent objects during a second rendering pass.
	bool _translucentPass;

	/// List of translucent graphics primitives collected during the first rendering pass, which
	/// need to be rendered during the second pass.
	std::vector<std::tuple<AffineTransformation, std::shared_ptr<PrimitiveBase>>> _translucentPrimitives;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_SCENE_RENDERER_H
