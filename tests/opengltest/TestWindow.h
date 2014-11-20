///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_GLTEST_WINDOW_H
#define __OVITO_GLTEST_WINDOW_H

#include <core/Core.h>
#include <core/viewport/Viewport.h>

#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_3_2_Core>

class TestWindow : public QWindow
{
public:

	/// Constructor.
	TestWindow(int id);

	/// Returns the window's OpenGL context used for rendering.
	QOpenGLContext* glcontext() const { return _context; }

	void renderNow();

	virtual void renderContent();

	/// Returns a pointer to the OpenGL functions object.
	QOpenGLFunctions* glfuncs() const { return _glFunctions; }

	/// Returns a pointer to the OpenGL 2.0 functions object.
	QOpenGLFunctions_2_0* glfuncs20() const { return _glFunctions20; }

	/// Returns a pointer to the OpenGL 3.0 functions object.
	QOpenGLFunctions_3_0* glfuncs30() const { return _glFunctions30; }

	/// Returns a pointer to the OpenGL 3.2 core profile functions object.
	QOpenGLFunctions_3_2_Core* glfuncs32() const { return _glFunctions32; }

	/// Indicates whether the current OpenGL implementation is according to the core profile.
	virtual bool isCoreProfile() const { return _isCoreProfile; }

	/// Returns the surface format of the current OpenGL context.
	const QSurfaceFormat& glformat() const { return _glformat; }

	/// Returns the view projection parameters.
	const Ovito::ViewProjectionParameters& projParams() const { return _projParams; }

	/// Returns the current model-to-view transformation matrix.
	Ovito::AffineTransformation modelViewTM() const { return Ovito::AffineTransformation::lookAt(
			Ovito::Point3(0.6,0.3,3),
			Ovito::Point3(0,0,0),
			Ovito::Vector3(0,1,0)); }

	/// The OpenGL glPointParameterf() function.
	void glPointParameterf(GLenum pname, GLfloat param) {
		if(glfuncs32()) glfuncs32()->glPointParameterf(pname, param);
		else if(glfuncs30()) glfuncs30()->glPointParameterf(pname, param);
		else if(glfuncs20()) glfuncs20()->glPointParameterf(pname, param);
		else msg() << "WARNING: Don't know how to call glPointParameterf() with this context";
	}

	/// The OpenGL glPointParameterfv() function.
	void glPointParameterfv(GLenum pname, const GLfloat* params) {
		if(glfuncs32()) glfuncs32()->glPointParameterfv(pname, params);
		else if(glfuncs30()) glfuncs30()->glPointParameterfv(pname, params);
		else if(glfuncs20()) glfuncs20()->glPointParameterfv(pname, params);
		else msg() << "WARNING: Don't know how to call glPointParameterfv() with this context";
	}

	/// The OpenGL glMultiDrawArrays() function.
	void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount) {
		if(glfuncs32()) glfuncs32()->glMultiDrawArrays(mode, first, count, drawcount);
		else if(glfuncs30()) glfuncs30()->glMultiDrawArrays(mode, first, count, drawcount);
		else if(glfuncs20()) glfuncs20()->glMultiDrawArrays(mode, first, count, drawcount);
		else msg() << "WARNING: Don't know how to call glMultiDrawArrays() with this context";
	}

	QDebug msg() const {
		return qDebug() << "WINDOW" << _id << ":";
	}

	/// Loads an OpenGL shader program.
	QOpenGLShaderProgram* loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile = QString());

	/// Loads and compiles a GLSL shader and adds it to the given program object.
	void loadShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderType shaderType, const QString& filename);

	virtual std::tuple<QString, QString, QString> shaderFiles() const { return std::tuple<QString, QString, QString>(QString(), QString(), QString()); }

	QOpenGLShaderProgram* getShader() {
		std::tuple<QString, QString, QString> files = shaderFiles();
		return loadShaderProgram("shader", std::get<0>(files), std::get<1>(files), std::get<2>(files));
	}


	/// Make sure vertex IDs are available to use by the OpenGL shader.
	void activateVertexIDs(QOpenGLShaderProgram* shader, GLint vertexCount, bool alwaysUseVBO = false);

	/// This needs to be called to deactivate vertex IDs, which were activated by a call to activateVertexIDs().
	void deactivateVertexIDs(QOpenGLShaderProgram* shader, bool alwaysUseVBO = false);

protected:

	/// Handles the expose events.
	virtual void exposeEvent(QExposeEvent* event) override;

	/// Handles the resize events.
	virtual void resizeEvent(QResizeEvent* event) override;

	int _id;

	/// The OpenGL functions object.
	QOpenGLFunctions* _glFunctions;

	/// The OpenGL 2.0 functions object.
	QOpenGLFunctions_2_0* _glFunctions20;

	/// The OpenGL 3.0 functions object.
	QOpenGLFunctions_3_0* _glFunctions30;

	/// The OpenGL 3.2 core profile functions object.
	QOpenGLFunctions_3_2_Core* _glFunctions32;

	/// The OpenGL surface format.
	QSurfaceFormat _glformat;

	/// The OpenGL context used for rendering.
	QOpenGLContext* _context;

	/// Indicates whether the current OpenGL implementation is based on the core or the compatibility profile.
	bool _isCoreProfile;

	/// The internal OpenGL vertex buffer that stores vertex IDs.
	QOpenGLBuffer _glVertexIDBuffer;

	/// The number of IDs stored in the OpenGL buffer.
	GLint _glVertexIDBufferSize;

	Ovito::ViewProjectionParameters _projParams;

	Q_OBJECT
};

#endif // __OVITO_GLTEST_WINDOW_H
