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

#include "TestWindow.h"
#include <core/rendering/viewport/OpenGLHelpers.h>
#include <core/utilities/Exception.h>

using namespace Ovito;

/******************************************************************************
* Constructor.
******************************************************************************/
TestWindow::TestWindow(int id) :
	_id(id), _context(nullptr),
	_glVertexIDBufferSize(-1)
{
	// Indicate that the window is to be used for OpenGL rendering.
	setSurfaceType(QWindow::OpenGLSurface);
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setMajorVersion(OVITO_OPENGL_REQUESTED_VERSION_MAJOR);
	format.setMinorVersion(OVITO_OPENGL_REQUESTED_VERSION_MINOR);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setStencilBufferSize(1);
	setFormat(format);
}

/******************************************************************************
* Immediately redraws the contents of this window.
******************************************************************************/
void TestWindow::renderContent()
{
}

/******************************************************************************
* Handles the expose events.
******************************************************************************/
void TestWindow::exposeEvent(QExposeEvent*)
{
	if(isExposed()) {
		renderNow();
	}
}

/******************************************************************************
* Handles the resize events.
******************************************************************************/
void TestWindow::resizeEvent(QResizeEvent*)
{
	if(isExposed()) {
		renderNow();
	}
}

/******************************************************************************
* Immediately redraws the contents of this window.
******************************************************************************/
void TestWindow::renderNow()
{
	if(!isExposed())
		return;

	_projParams.aspectRatio = (FloatType)height() / width();
	_projParams.isPerspective = false;
	_projParams.znear = -4;
	_projParams.zfar = 4;
	_projParams.fieldOfView = 1;
	_projParams.viewMatrix = AffineTransformation::Identity();
	_projParams.inverseViewMatrix = _projParams.viewMatrix.inverse();
	_projParams.projectionMatrix = Matrix4::ortho(-_projParams.fieldOfView / _projParams.aspectRatio, _projParams.fieldOfView / _projParams.aspectRatio,
						-_projParams.fieldOfView, _projParams.fieldOfView,
						_projParams.znear, _projParams.zfar);
	_projParams.inverseProjectionMatrix = _projParams.projectionMatrix.inverse();

	msg() << "------------------------------------------------------";

	// Create OpenGL context on first redraw.
	if(!_context) {
		_context = new QOpenGLContext(this);
		_context->setFormat(requestedFormat());
		if(!_context->create()) {
			msg() << "Failed to create OpenGL context for window" << _id;
			return;
		}
		_context->makeCurrent(this);

		QSurfaceFormat format = _context->format();
		msg() << "OpenGL depth buffer size:   " << format.depthBufferSize();
		(msg() << "OpenGL version:             ").nospace() << format.majorVersion() << "." << format.minorVersion();
		msg() << "OpenGL profile:             " << (format.profile() == QSurfaceFormat::CoreProfile ? "core" : (format.profile() == QSurfaceFormat::CompatibilityProfile ? "compatibility" : "none"));
		msg() << "OpenGL has alpha:           " << format.hasAlpha();
		msg() << "OpenGL vendor:              " << QString((const char*)glGetString(GL_VENDOR));
		msg() << "OpenGL renderer:            " << QString((const char*)glGetString(GL_RENDERER));
		msg() << "OpenGL version string:      " << QString((const char*)glGetString(GL_VERSION));
		msg() << "OpenGL shading language:    " << QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		msg() << "OpenGL shader programs:     " << QOpenGLShaderProgram::hasOpenGLShaderPrograms();
		msg() << "OpenGL vertex shaders:      " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex);
		msg() << "OpenGL fragment shaders:    " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment);
		msg() << "OpenGL geometry shaders:    " << QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry);
		msg() << "OpenGL swap behavior:       " << (format.swapBehavior() == QSurfaceFormat::SingleBuffer ? QStringLiteral("single buffer") : (format.swapBehavior() == QSurfaceFormat::DoubleBuffer ? QStringLiteral("double buffer") : (format.swapBehavior() == QSurfaceFormat::TripleBuffer ? QStringLiteral("triple buffer") : QStringLiteral("other"))));
		msg() << "OpenGL stencil buffer size: " << format.stencilBufferSize();
		msg() << "OpenGL deprecated func:     " << format.testOption(QSurfaceFormat::DeprecatedFunctions);
	}

	if(!_context->isValid())
		return;

	if(!_context->makeCurrent(this)) {
		msg() << "Failed to make OpenGL context current. Window:" << _id;
		return;
	}

	// Obtain a functions object that allows to call basic OpenGL functions in a platform-independent way.
	_glFunctions = glcontext()->functions();

	// Obtain a functions object that allows to call OpenGL 2.0 functions in a platform-independent way.
	_glFunctions20 = glcontext()->versionFunctions<QOpenGLFunctions_2_0>();
	if(!_glFunctions20 || !_glFunctions20->initializeOpenGLFunctions())
		_glFunctions20 = nullptr;

	// Obtain a functions object that allows to call OpenGL 3.0 functions in a platform-independent way.
	_glFunctions30 = glcontext()->versionFunctions<QOpenGLFunctions_3_0>();
	if(!_glFunctions30 || !_glFunctions30->initializeOpenGLFunctions())
		_glFunctions30 = nullptr;

	// Obtain a functions object that allows to call OpenGL 3.2 core functions in a platform-independent way.
	_glFunctions32 = glcontext()->versionFunctions<QOpenGLFunctions_3_2_Core>();
	if(!_glFunctions32 || !_glFunctions32->initializeOpenGLFunctions())
		_glFunctions32 = nullptr;

	if(!_glFunctions20 && !_glFunctions30 && !_glFunctions32) {
		msg() << "ERORR: Could not resolve OpenGL functions. Window:" << _id;
		return;
	}

	QSize vpSize = size();
	glViewport(0, 0, vpSize.width(), vpSize.height());

	// Obtain surface format.
	_glformat = glcontext()->format();

	// Check if this context implements the core profile.
	_isCoreProfile = (_glformat.profile() == QSurfaceFormat::CoreProfile)
			|| (glformat().majorVersion() > 3)
			|| (glformat().majorVersion() == 3 && glformat().minorVersion() >= 2);

	// Qt reports the core profile only for OpenGL >= 3.2. Assume core profile also for 3.1 contexts.
	if(glformat().majorVersion() == 3 && glformat().minorVersion() == 1 && _glformat.profile() != QSurfaceFormat::CompatibilityProfile)
		_isCoreProfile = true;

	OVITO_CHECK_OPENGL(glDisable(GL_STENCIL_TEST));
	OVITO_CHECK_OPENGL(glEnable(GL_DEPTH_TEST));
	OVITO_CHECK_OPENGL(glDepthFunc(GL_LESS));
	OVITO_CHECK_OPENGL(glDepthRange(0, 1));
	OVITO_CHECK_OPENGL(glDepthMask(GL_TRUE));
	OVITO_CHECK_OPENGL(glClearDepth(1));
	OVITO_CHECK_OPENGL(glDisable(GL_SCISSOR_TEST));

	OVITO_CHECK_OPENGL(glClearColor(0.2, 0.2, 0.2, 1));
	OVITO_CHECK_OPENGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	try {
		// Set up a vertex array object (VAO). An active VAO is required during rendering according to the OpenGL core profile.
		QScopedPointer<QOpenGLVertexArrayObject> vertexArrayObject;
		if(glformat().majorVersion() >= 3) {
			vertexArrayObject.reset(new QOpenGLVertexArrayObject());
			OVITO_CHECK_OPENGL(vertexArrayObject->create());
			OVITO_CHECK_OPENGL(vertexArrayObject->bind());
		}

		renderContent();
		_context->swapBuffers(this);
	}
	catch(const Exception& ex) {
		ex.logError();
	}
	_context->doneCurrent();
}

/******************************************************************************
* Translates an OpenGL error code to a human-readable message string.
******************************************************************************/
const char* openglErrorString(GLenum errorCode)
{
	switch(errorCode) {
	case GL_NO_ERROR: return "GL_NO_ERROR - No error has been recorded.";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM - An unacceptable value is specified for an enumerated argument.";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE - A numeric argument is out of range.";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION - The specified operation is not allowed in the current state.";
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW - This command would cause a stack overflow.";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW - This command would cause a stack underflow.";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY - There is not enough memory left to execute the command.";
	case GL_TABLE_TOO_LARGE: return "GL_TABLE_TOO_LARGE - The specified table exceeds the implementation's maximum supported table size.";
	default: return "Unknown OpenGL error code.";
	}
}

/******************************************************************************
* Reports OpenGL error status codes.
******************************************************************************/
void checkOpenGLErrorStatus(const char* command, const char* sourceFile, int sourceLine, int windowId)
{
	GLenum error;
	while((error = ::glGetError()) != GL_NO_ERROR) {
		qDebug() << "WINDOW" << windowId << ": OpenGL call" << command << "failed "
				"in line" << sourceLine << "of file" << sourceFile
				<< "with error" << openglErrorString(error);
	}
}

/******************************************************************************
* Loads an OpenGL shader program.
******************************************************************************/
QOpenGLShaderProgram* TestWindow::loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile)
{
	// The OpenGL shaders are only created once per OpenGL context group.
	QScopedPointer<QOpenGLShaderProgram> program(findChild<QOpenGLShaderProgram*>(id));
	if(program)
		return program.take();

	program.reset(new QOpenGLShaderProgram(this));
	program->setObjectName(id);

	// Load and compile vertex shader source.
	loadShader(program.data(), QOpenGLShader::Vertex, vertexShaderFile);

	// Load and compile fragment shader source.
	loadShader(program.data(), QOpenGLShader::Fragment, fragmentShaderFile);

	// Load and compile geometry shader source.
	if(!geometryShaderFile.isEmpty()) {
		loadShader(program.data(), QOpenGLShader::Geometry, geometryShaderFile);
	}

	if(!program->link()) {
		msg() << "OpenGL shader log:";
		msg() << program->log();
		throw Exception(QString("The OpenGL shader program %1 failed to link.").arg(id));
	}

	return program.take();
}

/******************************************************************************
* Loads and compiles a GLSL shader and adds it to the given program object.
******************************************************************************/
void TestWindow::loadShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderType shaderType, const QString& filename)
{
	// Load shader source.
	QFile shaderSourceFile(filename);
	if(!shaderSourceFile.open(QFile::ReadOnly)) {
		throw Exception(QString("Unable to open shader source file %1").arg(filename));
	}
	QByteArray shaderSource;

	// Insert GLSL version string at the top.
	// Pick GLSL language version based on current OpenGL version.
	if((glformat().majorVersion() >= 3 && glformat().minorVersion() >= 2) || glformat().majorVersion() > 3)
		shaderSource.append("#version 150\n");
	else if(glformat().majorVersion() >= 3)
		shaderSource.append("#version 130\n");
	else
		shaderSource.append("#version 120\n");

	// Preprocess shader source while reading it from the file.
	//
	// This is a workaround for some older OpenGL driver, which do not perform the
	// preprocessing of shader source files correctly (probably the __VERSION__ macro is not working).
	//
	// Here, in our own simple preprocessor implementation, we only handle
	//    #if __VERSION__ >= 130
	//       ...
	//    #else
	//       ...
	//    #endif
	// statements, which are used by most shaders to discriminate core and compatibility profiles.
	bool isFiltered = false;
	int ifstack = 0;
	int filterstackpos = 0;
	while(!shaderSourceFile.atEnd()) {
		QByteArray line = shaderSourceFile.readLine();
		if(line.contains("__VERSION__") && line.contains("130")) {
			OVITO_ASSERT(line.contains("#if"));
			OVITO_ASSERT(!isFiltered);
			if(line.contains(">=") && glformat().majorVersion() < 3) isFiltered = true;
			if(line.contains("<") && glformat().majorVersion() >= 3) isFiltered = true;
			filterstackpos = ifstack;
			continue;
		}
		else if(line.contains("#if")) {
			ifstack++;
		}
		else if(line.contains("#else")) {
			if(ifstack == filterstackpos) {
				isFiltered = !isFiltered;
				continue;
			}
		}
		else if(line.contains("#endif")) {
			if(ifstack == filterstackpos) {
				filterstackpos = -1;
				isFiltered = false;
				continue;
			}
			ifstack--;
		}

		if(!isFiltered) {
			shaderSource.append(line);
		}
	}

	// Load and compile vertex shader source.
	if(!program->addShaderFromSourceCode(shaderType, shaderSource)) {
		msg() << "OpenGL shader log:";
		msg() << program->log();
		throw Exception(QString("The shader source file %1 failed to compile.").arg(filename));
	}
}


/******************************************************************************
* Makes vertex IDs available to the shader.
******************************************************************************/
void TestWindow::activateVertexIDs(QOpenGLShaderProgram* shader, GLint vertexCount, bool alwaysUseVBO)
{
	// Older OpenGL implementations do not provide the built-in gl_VertexID shader
	// variable. Therefore we have to provide the IDs in a vertex buffer.
	if(glformat().majorVersion() < 3 || alwaysUseVBO) {
		if(!_glVertexIDBuffer.isCreated() || _glVertexIDBufferSize < vertexCount) {
			if(!_glVertexIDBuffer.isCreated()) {
				// Create the ID buffer only once and keep it until the number of particles changes.
				if(!_glVertexIDBuffer.create())
					throw Exception(QStringLiteral("Failed to create OpenGL vertex ID buffer."));
				_glVertexIDBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
			}
			if(!_glVertexIDBuffer.bind())
				throw Exception(QStringLiteral("Failed to bind OpenGL vertex ID buffer."));
			_glVertexIDBuffer.allocate(vertexCount * sizeof(GLfloat));
			_glVertexIDBufferSize = vertexCount;
			if(vertexCount > 0) {
				GLfloat* bufferData = static_cast<GLfloat*>(_glVertexIDBuffer.map(QOpenGLBuffer::WriteOnly));
				if(!bufferData)
					throw Exception(QStringLiteral("Failed to map OpenGL vertex ID buffer to memory."));
				GLfloat* bufferDataEnd = bufferData + vertexCount;
				for(GLint index = 0; bufferData != bufferDataEnd; ++index, ++bufferData)
					*bufferData = index;
				_glVertexIDBuffer.unmap();
			}
		}
		else {
			if(!_glVertexIDBuffer.bind())
				throw Exception(QStringLiteral("Failed to bind OpenGL vertex ID buffer."));
		}

		// This vertex attribute will be mapped to the gl_VertexID variable.
		shader->enableAttributeArray("vertexID");
		shader->setAttributeBuffer("vertexID", GL_FLOAT, 0, 1);
		_glVertexIDBuffer.release();
	}
}

/******************************************************************************
* Disables vertex IDs.
******************************************************************************/
void TestWindow::deactivateVertexIDs(QOpenGLShaderProgram* shader, bool alwaysUseVBO)
{
	if(glformat().majorVersion() < 3 || alwaysUseVBO)
		shader->disableAttributeArray("vertexID");
}
