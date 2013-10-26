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
#include <core/scene/SceneNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/GroupNode.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/display/DisplayObject.h>
#include <core/dataset/DataSet.h>
#include <core/viewport/input/ViewportInputManager.h>
#include "ViewportSceneRenderer.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ViewportSceneRenderer, SceneRenderer);

/******************************************************************************
* This method is called just before renderFrame() is called.
******************************************************************************/
void ViewportSceneRenderer::beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp)
{
	SceneRenderer::beginFrame(time, params, vp);

	_glcontext = QOpenGLContext::currentContext();
	if(!_glcontext)
		throw Exception(tr("Cannot render scene: There is no active OpenGL context"));

	OVITO_CHECK_OPENGL();

	// Obtain a functions object that allows to call basic OpenGL functions in a platform-independent way.
	_glFunctions = _glcontext->functions();

	// Obtain a functions object that allows to call OpenGL 2.0 functions in a platform-independent way.
	_glFunctions20 = _glcontext->versionFunctions<QOpenGLFunctions_2_0>();
	if(!_glFunctions20 || !_glFunctions20->initializeOpenGLFunctions())
		_glFunctions20 = nullptr;

	// Obtain a functions object that allows to call OpenGL 3.0 functions in a platform-independent way.
	_glFunctions30 = _glcontext->versionFunctions<QOpenGLFunctions_3_0>();
	if(!_glFunctions30 || !_glFunctions30->initializeOpenGLFunctions())
		_glFunctions30 = nullptr;

	// Obtain a functions object that allows to call OpenGL 3.2 core functions in a platform-independent way.
	_glFunctions32 = _glcontext->versionFunctions<QOpenGLFunctions_3_2_Core>();
	if(!_glFunctions32 || !_glFunctions32->initializeOpenGLFunctions())
		_glFunctions32 = nullptr;

	if(!_glFunctions20 && !_glFunctions30 && !_glFunctions32)
		throw Exception(tr("Could not resolve OpenGL functions. Invalid OpenGL context."));

	// Obtain surface format.
	_glformat = _glcontext->format();

	// Check if this context implements the core profile.
	_isCoreProfile = (_glformat.profile() == QSurfaceFormat::CoreProfile);
	// Qt reports the core profile only for OpenGL >= 3.2. Some implementations of OpenGL 3.1
	// may also follow the core profile. The only way to detect this seems to be inspecting the 
	// version string.
	if(glformat().majorVersion() == 3 && glformat().minorVersion() == 1) {
		const char* versionString = (const char*)glGetString(GL_VERSION);
		if(QByteArray::fromRawData(versionString, qstrlen(versionString)).indexOf("Core Profile") >= 0)
			_isCoreProfile = true;
	}

	// Set up a vertex array object. This is only required when using OpenGL Core Profile.
	if(isCoreProfile()) {
		_vertexArrayObject.reset(new QOpenGLVertexArrayObject());
		OVITO_CHECK_OPENGL(_vertexArrayObject->create());
		OVITO_CHECK_OPENGL(_vertexArrayObject->bind());
	}
	OVITO_CHECK_OPENGL();

	// Set viewport background color.
	OVITO_CHECK_OPENGL();
	Color backgroundColor = Viewport::viewportColor(ViewportSettings::COLOR_VIEWPORT_BKG);
	OVITO_CHECK_OPENGL(glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), 1));
}

/******************************************************************************
* This method is called after renderFrame() has been called.
******************************************************************************/
void ViewportSceneRenderer::endFrame()
{
	OVITO_CHECK_OPENGL();
	OVITO_CHECK_OPENGL(_vertexArrayObject.reset());
	_glcontext = nullptr;

	SceneRenderer::endFrame();
}

/******************************************************************************
* Renders the current animation frame.
******************************************************************************/
bool ViewportSceneRenderer::renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress)
{
	OVITO_ASSERT(_glcontext == QOpenGLContext::currentContext());

	// Clear background.
	OVITO_CHECK_OPENGL();
	OVITO_CHECK_OPENGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	OVITO_CHECK_OPENGL(glEnable(GL_DEPTH_TEST));

	renderScene();

	// Render visual 3D representation of the modifiers.
	renderModifiers(false);

	// Render input mode 3D overlays.
	if(isInteractive()) {
		for(const auto& handler : ViewportInputManager::instance().stack()) {
			if(handler->hasOverlay())
				handler->renderOverlay3D(viewport(), this, handler == ViewportInputManager::instance().currentHandler());
		}
	}

	// Render visual 2D representation of the modifiers.
	renderModifiers(true);

	// Render input mode 2D overlays.
	if(isInteractive()) {
		for(const auto& handler : ViewportInputManager::instance().stack()) {
			if(handler->hasOverlay())
				handler->renderOverlay2D(viewport(), this, handler == ViewportInputManager::instance().currentHandler());
		}
	}

	return true;
}

/******************************************************************************
* Changes the current local to world transformation matrix.
******************************************************************************/
void ViewportSceneRenderer::setWorldTransform(const AffineTransformation& tm)
{
	_modelViewTM = projParams().viewMatrix * tm;
}

/******************************************************************************
* Translates an OpenGL error code to a human-readable message string.
******************************************************************************/
const char* ViewportSceneRenderer::openglErrorString(GLenum errorCode)
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
* Renders the visual representation of the modifiers.
******************************************************************************/
void ViewportSceneRenderer::renderModifiers(bool renderOverlay)
{
	// Visit all pipeline objects in the scene.
	dataset()->sceneRoot()->visitChildren([this, renderOverlay](SceneNode* node) -> bool {
		if(node->isObjectNode()) {
			ObjectNode* objNode = static_object_cast<ObjectNode>(node);
			PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(objNode->sceneObject());
			if(pipelineObj)
				renderModifiers(pipelineObj, objNode, renderOverlay);
		}
		return true;
	});
}

/******************************************************************************
* Renders the visual representation of the modifiers.
******************************************************************************/
void ViewportSceneRenderer::renderModifiers(PipelineObject* pipelineObj, ObjectNode* objNode, bool renderOverlay)
{
	OVITO_CHECK_OBJECT_POINTER(pipelineObj);

	// Render the visual representation of the modifier that is currently being edited.
	for(ModifierApplication* modApp : pipelineObj->modifierApplications()) {
		Modifier* mod = modApp->modifier();

		TimeInterval interval;
		// Setup transformation.
		setWorldTransform(objNode->getWorldTransform(time(), interval));

		// Render selected modifier.
		mod->render(time(), objNode, modApp, this, renderOverlay);
	}

	// Continue with nested pipeline objects.
	for(int i = 0; i < pipelineObj->inputObjectCount(); i++) {
		PipelineObject* input = dynamic_object_cast<PipelineObject>(pipelineObj->inputObject(i));
		if(input)
			renderModifiers(input, objNode, renderOverlay);
	}
}

/******************************************************************************
* Determines the bounding box of the visual representation of the modifiers.
******************************************************************************/
void ViewportSceneRenderer::boundingBoxModifiers(PipelineObject* pipelineObj, ObjectNode* objNode, Box3& boundingBox)
{
	OVITO_CHECK_OBJECT_POINTER(pipelineObj);
	TimeInterval interval;

	// Render the visual representation of the modifier that is currently being edited.
	for(ModifierApplication* modApp : pipelineObj->modifierApplications()) {
		Modifier* mod = modApp->modifier();

		// Compute bounding box and transform it to world space.
		boundingBox.addBox(mod->boundingBox(time(), objNode, modApp).transformed(objNode->getWorldTransform(time(), interval)));
	}

	// Continue with nested pipeline objects.
	for(int i = 0; i < pipelineObj->inputObjectCount(); i++) {
		PipelineObject* input = dynamic_object_cast<PipelineObject>(pipelineObj->inputObject(i));
		if(input)
			boundingBoxModifiers(input, objNode, boundingBox);
	}
}

/******************************************************************************
* Computes the bounding box of the the 3D visual elements
* shown only in the interactive viewports.
******************************************************************************/
Box3 ViewportSceneRenderer::boundingBoxInteractive(TimePoint time, Viewport* viewport)
{
	OVITO_CHECK_POINTER(viewport);
	Box3 bb;

	// Visit all pipeline objects in the scene.
	dataset()->sceneRoot()->visitObjectNodes([this, &bb](ObjectNode* node) -> bool {
		PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(node->sceneObject());
		if(pipelineObj)
			boundingBoxModifiers(pipelineObj, node, bb);
		return true;
	});

	// Include input mode overlays.
	for(const auto& handler : ViewportInputManager::instance().stack()) {
		if(handler->hasOverlay())
			bb.addBox(handler->overlayBoundingBox(viewport, this, handler == ViewportInputManager::instance().currentHandler()));
	}

	return bb;
}

/******************************************************************************
* Loads an OpenGL shader program.
******************************************************************************/
QOpenGLShaderProgram* ViewportSceneRenderer::loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile)
{
	QOpenGLContextGroup* contextGroup = glcontext()->shareGroup();
	OVITO_ASSERT(contextGroup == QOpenGLContextGroup::currentContextGroup());

	OVITO_ASSERT(QOpenGLShaderProgram::hasOpenGLShaderPrograms());
	OVITO_ASSERT(QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex));
	OVITO_ASSERT(QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment));

	// The OpenGL shaders are only created once per OpenGL context group.
	QScopedPointer<QOpenGLShaderProgram> program(contextGroup->findChild<QOpenGLShaderProgram*>(id));
	if(program)
		return program.take();

	program.reset(new QOpenGLShaderProgram(contextGroup));
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
		qDebug() << "OpenGL shader log:";
		qDebug() << program->log();
		throw Exception(QString("The OpenGL shader program %1 failed to link. See log for details.").arg(id));
	}

	OVITO_ASSERT(contextGroup->findChild<QOpenGLShaderProgram*>(id) == program.data());
	return program.take();
}

/******************************************************************************
* Loads and compiles a GLSL shader and adds it to the given program object.
******************************************************************************/
void ViewportSceneRenderer::loadShader(QOpenGLShaderProgram* program, QOpenGLShader::ShaderType shaderType, const QString& filename)
{
	// Load shader source.
	QFile shaderSourceFile(filename);
	if(!shaderSourceFile.open(QFile::ReadOnly))
		throw Exception(QString("Unable to open shader source file %1.").arg(filename));
	QByteArray shaderSource = shaderSourceFile.readAll();

	// Insert GLSL version string at the top.
	// Pick GLSL language version based on current OpenGL version.
	if(glformat().majorVersion() >= 3 && glformat().minorVersion() >= 2)
		shaderSource.prepend("#version 150\n");
	else if(glformat().majorVersion() >= 3)
		shaderSource.prepend("#version 130\n");
	else 
		shaderSource.prepend("#version 120\n");

	// Load and compile vertex shader source.
	if(!program->addShaderFromSourceCode(shaderType, shaderSource)) {
		qDebug() << "OpenGL shader log:";
		qDebug() << program->log();
		throw Exception(QString("The shader source file %1 failed to compile. See log for details.").arg(filename));
	}
}

/******************************************************************************
* Renders a 2d polyline in the viewport.
******************************************************************************/
void ViewportSceneRenderer::render2DPolyline(const Point2* points, int count, const ColorA& color, bool closed)
{
	OVITO_STATIC_ASSERT(sizeof(points[0]) == 2*sizeof(GLfloat));

	// Initialize OpenGL shader.
	QOpenGLShaderProgram* shader = loadShaderProgram("line", ":/core/glsl/line.vertex.glsl", ":/core/glsl/line.fragment.glsl");

	if(!shader->bind())
		throw Exception(tr("Failed to bind OpenGL shader."));

	bool wasDepthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);

	GLint vc[4];
	glGetIntegerv(GL_VIEWPORT, vc);
	QMatrix4x4 tm;
	tm.ortho(vc[0], vc[0] + vc[2], vc[1] + vc[3], vc[1], -1, 1);
	OVITO_CHECK_OPENGL(shader->setUniformValue("modelview_projection_matrix", tm));

	QOpenGLBuffer vertexBuffer;
	if(glformat().majorVersion() >= 3) {
		if(!vertexBuffer.create())
			throw Exception(tr("Failed to create OpenGL vertex buffer."));
		if(!vertexBuffer.bind())
				throw Exception(tr("Failed to bind OpenGL vertex buffer."));
		vertexBuffer.allocate(points, 2 * sizeof(GLfloat) * count);
		OVITO_CHECK_OPENGL(shader->enableAttributeArray("vertex_pos"));
		OVITO_CHECK_OPENGL(shader->setAttributeBuffer("vertex_pos", GL_FLOAT, 0, 2));
		vertexBuffer.release();
	}
	else {
		OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
		OVITO_CHECK_OPENGL(glVertexPointer(2, GL_FLOAT, 0, points));
	}

	if(glformat().majorVersion() >= 3) {
		OVITO_CHECK_OPENGL(shader->disableAttributeArray("vertex_color"));
		OVITO_CHECK_OPENGL(shader->setAttributeValue("vertex_color", color.r(), color.g(), color.b(), color.a()));
	}
	else {
		OVITO_CHECK_OPENGL(glColor4(color));
	}

	OVITO_CHECK_OPENGL(glDrawArrays(closed ? GL_LINE_LOOP : GL_LINE_STRIP, 0, count));

	if(glformat().majorVersion() >= 3) {
		shader->disableAttributeArray("vertex_pos");
	}
	else {
		OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
	}
	shader->release();
	if(wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
}

};
