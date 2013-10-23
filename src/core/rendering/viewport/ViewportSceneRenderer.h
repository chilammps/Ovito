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
 * \file ViewportSceneRenderer.h
 * \brief Contains the definition of the Ovito::ViewportSceneRenderer class.
 */
#ifndef __OVITO_VIEWPORT_SCENE_RENDERER_H
#define __OVITO_VIEWPORT_SCENE_RENDERER_H

#include <core/Core.h>
#include <core/rendering/SceneRenderer.h>
#include <base/utilities/Color.h>
#include "ViewportLineGeometryBuffer.h"
#include "ViewportParticleGeometryBuffer.h"
#include "ViewportTextGeometryBuffer.h"
#include "ViewportImageGeometryBuffer.h"
#include "ViewportArrowGeometryBuffer.h"

#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_3_2_Core>

namespace Ovito {

class PipelineObject;		// defined in PipelineObject.h

// The minimum OpenGL version required by Ovito:
#define OVITO_OPENGL_MINIMUM_VERSION_MAJOR 			2
#define OVITO_OPENGL_MINIMUM_VERSION_MINOR			0

// The standard OpenGL version used by Ovito:
#define OVITO_OPENGL_REQUESTED_VERSION_MAJOR 		3
#define OVITO_OPENGL_REQUESTED_VERSION_MINOR		2

/**
 * \brief This is the default scene renderer used to render the contents
 *        of the interactive viewports.
 */
class OVITO_CORE_EXPORT ViewportSceneRenderer : public SceneRenderer
{
public:

	/// Default constructor.
	ViewportSceneRenderer() : _glcontext(nullptr), _modelViewTM(AffineTransformation::Identity()) {}

	/// Renders the current animation frame.
	virtual bool renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress) override;

	/// This method is called just before renderFrame() is called.
	virtual void beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp) override;

	/// This method is called after renderFrame() has been called.
	virtual void endFrame() override;

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) override;

	/// Returns the current model-to-view transformation matrix.
	const AffineTransformation& modelViewTM() const { return _modelViewTM; }

	/// \brief Computes the bounding box of the the 3D visual elements
	///        shown only in the interactive viewports.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the world coordinate system that contains
	///         everything to be rendered.
	Box3 boundingBoxInteractive(TimePoint time, Viewport* viewport);

	/// Requests a new line geometry buffer from the renderer.
	virtual OORef<LineGeometryBuffer> createLineGeometryBuffer() override {
		return new ViewportLineGeometryBuffer(this);
	}

	/// Requests a new particle geometry buffer from the renderer.
	virtual OORef<ParticleGeometryBuffer> createParticleGeometryBuffer(ParticleGeometryBuffer::ShadingMode shadingMode = ParticleGeometryBuffer::NormalShading, ParticleGeometryBuffer::RenderingQuality renderingQuality = ParticleGeometryBuffer::MediumQuality) override {
		return new ViewportParticleGeometryBuffer(this, shadingMode, renderingQuality);
	}

	/// Requests a new text geometry buffer from the renderer.
	virtual OORef<TextGeometryBuffer> createTextGeometryBuffer() override {
		return new ViewportTextGeometryBuffer(this);
	}

	/// Requests a new image geometry buffer from the renderer.
	virtual OORef<ImageGeometryBuffer> createImageGeometryBuffer() override {
		return new ViewportImageGeometryBuffer(this);
	}

	/// Requests a new arrow geometry buffer from the renderer.
	virtual OORef<ArrowGeometryBuffer> createArrowGeometryBuffer(ArrowGeometryBuffer::Shape shape, ArrowGeometryBuffer::ShadingMode shadingMode = ArrowGeometryBuffer::NormalShading, ArrowGeometryBuffer::RenderingQuality renderingQuality = ArrowGeometryBuffer::MediumQuality) override {
		return new ViewportArrowGeometryBuffer(this, shape, shadingMode, renderingQuality);
	}

	/// Returns whether this renderer is rendering an interactive viewport.
	/// \return true if rendering a real-time viewport; false if rendering an output image.
	virtual bool isInteractive() const override { return true; }

	/// Returns the supersampling level.
	virtual int antialiasingLevel() const { return 1; }

	/// Returns the OpenGL context this renderer uses.
	QOpenGLContext* glcontext() const { return _glcontext; }

	/// Returns a pointer to the OpenGL functions object.
	QOpenGLFunctions* glfuncs() const { return _glFunctions; }

	/// Returns a pointer to the OpenGL 2.0 functions object.
	QOpenGLFunctions_2_0* glfuncs20() const { return _glFunctions20; }

	/// Returns a pointer to the OpenGL 3.0 functions object.
	QOpenGLFunctions_3_0* glfuncs30() const { return _glFunctions30; }

	/// Returns a pointer to the OpenGL 3.2 core profile functions object.
	QOpenGLFunctions_3_2_Core* glfuncs32() const { return _glFunctions32; }

	/// Returns the surface format of the current OpenGL context.
	const QSurfaceFormat& glformat() const { return _glformat; }

	/// Indicates whether the current OpenGL implementation is according to the core profile.
	bool isCoreProfile() const { return _isCoreProfile; }

	/// Translates an OpenGL error code to a human-readable message string.
	static const char* openglErrorString(GLenum errorCode);

	/// Loads and compiles an OpenGL shader program.
	QOpenGLShaderProgram* loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile = QString());

protected:

	/// \brief Renders the visual representation of the modifiers.
	void renderModifiers(bool renderOverlay);

	/// \brief Renders the visual representation of the modifiers.
	void renderModifiers(PipelineObject* pipelineObj, ObjectNode* objNode, bool renderOverlay);

	/// \brief Determines the bounding box of the visual representation of the modifiers.
	void boundingBoxModifiers(PipelineObject* pipelineObj, ObjectNode* objNode, Box3& boundingBox);

	/// \brief Loads and compiles a GLSL shader and adds it to the given program object.
	void loadShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderType shaderType, const QString& filename);

private:

	/// The OpenGL context this renderer uses.
	QOpenGLContext* _glcontext;

	/// The OpenGL functions object.
	QOpenGLFunctions* _glFunctions;

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

	/// Indicates whether the current OpenGL implementation is according to the core profile.
	bool _isCoreProfile;

	/// The current model-to-view transformation matrix.
	AffineTransformation _modelViewTM;

	Q_OBJECT
	OVITO_OBJECT
};

// OpenGL function call debugging macro
#ifdef OVITO_DEBUG
	#define OVITO_CHECK_OPENGL(cmd)									\
	{																\
		cmd;														\
		GLenum error;												\
		while((error = ::glGetError()) != GL_NO_ERROR) {			\
			qDebug() << "WARNING: OpenGL call" << #cmd << "failed "	\
			"in line" << __LINE__ << "of file" << __FILE__ 			\
			<< "with error" << Ovito::ViewportSceneRenderer::openglErrorString(error);			\
		}															\
	}
#else
	#define OVITO_CHECK_OPENGL(cmd)			cmd
#endif

};

#endif // __OVITO_VIEWPORT_SCENE_RENDERER_H
