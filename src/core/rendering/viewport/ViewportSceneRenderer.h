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

#include <QOpenGLFunctions_3_2_Core>

namespace Ovito {

class PipelineObject;		// defined in PipelineObject.h

/**
 * \brief This is the default scene renderer used to render the contents
 *        of the interactive viewports.
 */
class ViewportSceneRenderer : public SceneRenderer
{
public:

	/// Default constructor.
	Q_INVOKABLE ViewportSceneRenderer() : _glcontext(nullptr), _antialiasingLevel(2), _modelViewTM(AffineTransformation::Identity()) {
		INIT_PROPERTY_FIELD(ViewportSceneRenderer::_antialiasingLevel)
	}

	/// Renders the current animation frame.
	virtual void renderFrame() override;

	/// Changes the current local to world transformation matrix.
	virtual void setWorldTransform(const AffineTransformation& tm) override;

	/// Returns the current model-to-view transformation matrix.
	const AffineTransformation& modelViewTM() const { return _modelViewTM; }

	/// Requests a new line geometry buffer from the renderer.
	virtual OORef<LineGeometryBuffer> createLineGeometryBuffer() override {
		return new ViewportLineGeometryBuffer(this);
	}

	/// Requests a new particle geometry buffer from the renderer.
	virtual OORef<ParticleGeometryBuffer> createParticleGeometryBuffer() override {
		return new ViewportParticleGeometryBuffer(this);
	}

	/// Returns the OpenGL context this renderer uses.
	QOpenGLContext* glcontext() const { return _glcontext; }

	/// Returns a pointer to the OpenGL functions object.
	QOpenGLFunctions_3_2_Core* glfuncs() const { return _glFunctions; }

	/// Translates an OpenGL error code to a human-readable message string.
	static const char* openglErrorString(GLenum errorCode);

	/// Returns the number of sub-pixels to render.
	int antialiasingLevel() const { return _antialiasingLevel; }

	/// Sets the number of sub-pixels to render.
	void setAntialiasingLevel(int newLevel) { _antialiasingLevel = newLevel; }

	/// Loads and compiles an OpenGL shader program.
	QOpenGLShaderProgram* loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile = QString());

public:

	Q_PROPERTY(int antialiasingLevel READ antialiasingLevel WRITE setAntialiasingLevel)

protected:

	/// \brief Renders a single node.
	virtual void renderNode(SceneNode* node) override;

	/// \brief Renders the selected modifiers.
	void renderPipelineObject(PipelineObject* pipelineObj, ObjectNode* objNode);

private:

	/// The OpenGL context this renderer uses.
	QOpenGLContext* _glcontext;

	/// The OpenGL functions object.
	QOpenGLFunctions_3_2_Core* _glFunctions;

	/// Controls the number of sub-pixels to render.
	PropertyField<int> _antialiasingLevel;

	/// The current model-to-view transformation matrix.
	AffineTransformation _modelViewTM;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_antialiasingLevel);
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
