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
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/dataset/DataSet.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/rendering/RenderSettings.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>
#include "ViewportSceneRenderer.h"
#include "OpenGLLinePrimitive.h"
#include "OpenGLParticlePrimitive.h"
#include "OpenGLTextPrimitive.h"
#include "OpenGLImagePrimitive.h"
#include "OpenGLArrowPrimitive.h"
#include "OpenGLMeshPrimitive.h"
#include "OpenGLHelpers.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ViewportSceneRenderer, SceneRenderer);

/******************************************************************************
* Returns the default OpenGL surface format requested by OVITO when creating
* OpenGL contexts.
******************************************************************************/
QSurfaceFormat ViewportSceneRenderer::getDefaultSurfaceFormat()
{
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setMajorVersion(OVITO_OPENGL_REQUESTED_VERSION_MAJOR);
	format.setMinorVersion(OVITO_OPENGL_REQUESTED_VERSION_MINOR);
	if(Application::instance().cmdLineParser().isSet(QStringLiteral("glversion"))) {
		QStringList tokens = Application::instance().cmdLineParser().value(QStringLiteral("glversion")).split(QChar('.'));
		if(tokens.size() == 2) {
			int majorVersion = tokens[0].toInt();
			int minorVersion = tokens[1].toInt();
			if(majorVersion >= 1) {
				format.setMajorVersion(majorVersion);
				format.setMinorVersion(minorVersion);
			}
		}
	}
	format.setProfile(QSurfaceFormat::CoreProfile);
	if(Application::instance().cmdLineParser().isSet(QStringLiteral("glcompatprofile"))) {
		format.setProfile(QSurfaceFormat::CompatibilityProfile);
		format.setOption(QSurfaceFormat::DeprecatedFunctions);
	}
	format.setStencilBufferSize(1);
	return format;
}

/******************************************************************************
* This method is called just before renderFrame() is called.
******************************************************************************/
void ViewportSceneRenderer::beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp)
{
	SceneRenderer::beginFrame(time, params, vp);

	if(Application::instance().headlessMode())
		throw Exception(tr("Cannot use OpenGL renderer in headless mode."));

	_glcontext = QOpenGLContext::currentContext();
	if(!_glcontext)
		throw Exception(tr("Cannot render scene: There is no active OpenGL context"));

	// Obtain surface format.
    OVITO_REPORT_OPENGL_ERRORS();
	_glformat = _glcontext->format();

	// OpenGL in a VirtualBox machine Windows guest reports "2.1 Chromium 1.9" as version string,
	// which is not correctly parsed by Qt. We have to workaround this.
	if(qstrncmp((const char*)glGetString(GL_VERSION), "2.1 ", 4) == 0) {
		_glformat.setMajorVersion(2);
		_glformat.setMinorVersion(1);
	}

	// Obtain a functions object that allows to call basic OpenGL functions in a platform-independent way.
    OVITO_REPORT_OPENGL_ERRORS();
	_glFunctions = _glcontext->functions();

	// Obtain a functions object that allows to call OpenGL 1.4 functions in a platform-independent way.
	_glFunctions14 = _glcontext->versionFunctions<QOpenGLFunctions_1_4>();
	if(!_glFunctions14 || !_glFunctions14->initializeOpenGLFunctions())
		_glFunctions14 = nullptr;

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

	if(!_glFunctions14 && !_glFunctions20 && !_glFunctions30 && !_glFunctions32)
		throw Exception(tr("Could not resolve OpenGL functions. Invalid OpenGL context."));

	// Check if this context implements the core profile.
	_isCoreProfile = (_glformat.profile() == QSurfaceFormat::CoreProfile)
			|| (glformat().majorVersion() > 3)
			|| (glformat().majorVersion() == 3 && glformat().minorVersion() >= 2);

	// Qt reports the core profile only for OpenGL >= 3.2. Assume core profile also for 3.1 contexts.
	if(glformat().majorVersion() == 3 && glformat().minorVersion() == 1 && _glformat.profile() != QSurfaceFormat::CompatibilityProfile)
		_isCoreProfile = true;

	// Determine whether it's okay to use point sprites.
	_usePointSprites = ViewportWindow::pointSpritesEnabled();

	// Determine whether its okay to use geometry shaders.
	_useGeometryShaders = ViewportWindow::geometryShadersEnabled() && QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Geometry);

	// Set up a vertex array object (VAO). An active VAO is required during rendering according to the OpenGL core profile.
	if(glformat().majorVersion() >= 3) {
		_vertexArrayObject.reset(new QOpenGLVertexArrayObject());
		OVITO_CHECK_OPENGL(_vertexArrayObject->create());
		OVITO_CHECK_OPENGL(_vertexArrayObject->bind());
	}
    OVITO_REPORT_OPENGL_ERRORS();

	// Set viewport background color.
    OVITO_REPORT_OPENGL_ERRORS();
	if(isInteractive()) {
		Color backgroundColor;
		if(!viewport()->renderPreviewMode())
			backgroundColor = Viewport::viewportColor(ViewportSettings::COLOR_VIEWPORT_BKG);
		else
			backgroundColor = renderSettings()->backgroundColor();
		OVITO_CHECK_OPENGL(glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), 1));
	}

	// Reset OpenGL state.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

/******************************************************************************
* This method is called after renderFrame() has been called.
******************************************************************************/
void ViewportSceneRenderer::endFrame()
{
    OVITO_REPORT_OPENGL_ERRORS();
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

	// Set up OpenGL state.
    OVITO_REPORT_OPENGL_ERRORS();
	OVITO_CHECK_OPENGL(glDisable(GL_STENCIL_TEST));
	OVITO_CHECK_OPENGL(glEnable(GL_DEPTH_TEST));
	OVITO_CHECK_OPENGL(glDepthFunc(GL_LESS));
	OVITO_CHECK_OPENGL(glDepthRange(0, 1));
	OVITO_CHECK_OPENGL(glDepthMask(GL_TRUE));
	OVITO_CHECK_OPENGL(glClearDepth(1));
	OVITO_CHECK_OPENGL(glDisable(GL_SCISSOR_TEST));
	_translucentPass = false;

	// Clear background.
	OVITO_CHECK_OPENGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	renderScene();

	// Render visual 3D representation of the modifiers.
	renderModifiers(false);

	if(isInteractive()) {

		// Render construction grid.
		if(viewport()->isGridVisible())
			renderGrid();

		// Render input mode 3D overlays.
		MainWindow* mainWindow = renderDataset()->mainWindow();
		if(mainWindow) {
			for(const auto& handler : mainWindow->viewportInputManager()->stack()) {
				if(handler->hasOverlay())
					handler->renderOverlay3D(viewport(), this);
			}
		}
	}

	// Render visual 2D representation of the modifiers.
	renderModifiers(true);

	// Render input mode 2D overlays.
	if(isInteractive()) {
		MainWindow* mainWindow = renderDataset()->mainWindow();
		if(mainWindow) {
			for(const auto& handler : mainWindow->viewportInputManager()->stack()) {
				if(handler->hasOverlay())
					handler->renderOverlay2D(viewport(), this);
			}
		}
	}

	// Render translucent objects in a second pass.
	_translucentPass = true;
	for(auto& record : _translucentPrimitives) {
		setWorldTransform(std::get<0>(record));
		std::get<1>(record)->render(this);
	}
	_translucentPrimitives.clear();

	return true;
}

/******************************************************************************
* Changes the current local to world transformation matrix.
******************************************************************************/
void ViewportSceneRenderer::setWorldTransform(const AffineTransformation& tm)
{
	_modelWorldTM = tm;
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
* Returns the final size of the rendered image in pixels.
******************************************************************************/
QSize ViewportSceneRenderer::outputSize() const
{
	return viewport()->size();
}

/******************************************************************************
* Requests a new line geometry buffer from the renderer.
******************************************************************************/
std::shared_ptr<LinePrimitive> ViewportSceneRenderer::createLinePrimitive()
{
	return std::make_shared<OpenGLLinePrimitive>(this);
}

/******************************************************************************
* Requests a new particle geometry buffer from the renderer.
******************************************************************************/
std::shared_ptr<ParticlePrimitive> ViewportSceneRenderer::createParticlePrimitive(ParticlePrimitive::ShadingMode shadingMode,
		ParticlePrimitive::RenderingQuality renderingQuality, ParticlePrimitive::ParticleShape shape,
		bool translucentParticles) {
	return std::make_shared<OpenGLParticlePrimitive>(this, shadingMode, renderingQuality, shape, translucentParticles);
}

/******************************************************************************
* Requests a new text geometry buffer from the renderer.
******************************************************************************/
std::shared_ptr<TextPrimitive> ViewportSceneRenderer::createTextPrimitive()
{
	return std::make_shared<OpenGLTextPrimitive>(this);
}

/******************************************************************************
* Requests a new image geometry buffer from the renderer.
******************************************************************************/
std::shared_ptr<ImagePrimitive> ViewportSceneRenderer::createImagePrimitive()
{
	return std::make_shared<OpenGLImagePrimitive>(this);
}

/******************************************************************************
* Requests a new arrow geometry buffer from the renderer.
******************************************************************************/
std::shared_ptr<ArrowPrimitive> ViewportSceneRenderer::createArrowPrimitive(ArrowPrimitive::Shape shape,
		ArrowPrimitive::ShadingMode shadingMode,
		ArrowPrimitive::RenderingQuality renderingQuality)
{
	return std::make_shared<OpenGLArrowPrimitive>(this, shape, shadingMode, renderingQuality);
}

/******************************************************************************
* Requests a new triangle mesh buffer from the renderer.
******************************************************************************/
std::shared_ptr<MeshPrimitive> ViewportSceneRenderer::createMeshPrimitive()
{
	return std::make_shared<OpenGLMeshPrimitive>(this);
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
	renderDataset()->sceneRoot()->visitObjectNodes([this, viewport, time, &bb](ObjectNode* node) -> bool {

		// Ignore node if it is the view node of the viewport or if it is the target of the view node.
		if(viewport->viewNode()) {
			if(viewport->viewNode() == node || viewport->viewNode()->lookatTargetNode() == node)
				return true;
		}

		// Evaluate geometry pipeline of object node.
		const PipelineFlowState& state = node->evalPipeline(time);
		for(const auto& dataObj : state.objects()) {
			for(DisplayObject* displayObj : dataObj->displayObjects()) {
				if(displayObj && displayObj->isEnabled()) {
					TimeInterval interval;
					bb.addBox(displayObj->viewDependentBoundingBox(time, viewport,
							dataObj, node, state).transformed(node->getWorldTransform(time, interval)));
				}
			}
		}

		if(PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(node->dataProvider()))
			boundingBoxModifiers(pipelineObj, node, bb);

		return true;
	});

	// Include visual geometry of input mode overlays in bounding box.
	MainWindow* mainWindow = viewport->dataset()->mainWindow();
	if(mainWindow) {
		for(const auto& handler : mainWindow->viewportInputManager()->stack()) {
			if(handler->hasOverlay())
				bb.addBox(handler->overlayBoundingBox(viewport, this));
		}
	}

	// Include construction grid in bounding box.
	if(viewport->isGridVisible()) {
		FloatType gridSpacing;
		Box2I gridRange;
		std::tie(gridSpacing, gridRange) = determineGridRange(viewport);
		if(gridSpacing > 0) {
			bb.addBox(viewport->gridMatrix() * Box3(
					Point3(gridRange.minc.x() * gridSpacing, gridRange.minc.y() * gridSpacing, 0),
					Point3(gridRange.maxc.x() * gridSpacing, gridRange.maxc.y() * gridSpacing, 0)));
		}
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
		OVITO_ASSERT(useGeometryShaders());
		loadShader(program.data(), QOpenGLShader::Geometry, geometryShaderFile);
	}

	if(!program->link()) {
		Exception ex(QString("The OpenGL shader program %1 failed to link.").arg(id));
		ex.appendDetailMessage(program->log());
		throw ex;
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
		Exception ex(QString("The shader source file %1 failed to compile.").arg(filename));
		ex.appendDetailMessage(program->log());
		ex.appendDetailMessage(QStringLiteral("Problematic shader source:"));
		ex.appendDetailMessage(shaderSource);
		throw ex;
	}
}

/******************************************************************************
* Renders a 2d polyline in the viewport.
******************************************************************************/
void ViewportSceneRenderer::render2DPolyline(const Point2* points, int count, const ColorA& color, bool closed)
{
	OVITO_STATIC_ASSERT(sizeof(points[0]) == 2*sizeof(GLfloat));

	// Load OpenGL shader.
	QOpenGLShaderProgram* shader = loadShaderProgram("line", ":/core/glsl/lines/line.vs", ":/core/glsl/lines/line.fs");
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
		OVITO_CHECK_OPENGL(shader->enableAttributeArray("position"));
		OVITO_CHECK_OPENGL(shader->setAttributeBuffer("position", GL_FLOAT, 0, 2));
		vertexBuffer.release();
	}
	else {
		OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
		OVITO_CHECK_OPENGL(glVertexPointer(2, GL_FLOAT, 0, points));
	}

	if(glformat().majorVersion() >= 3) {
		OVITO_CHECK_OPENGL(shader->disableAttributeArray("color"));
		OVITO_CHECK_OPENGL(shader->setAttributeValue("color", color.r(), color.g(), color.b(), color.a()));
	}
	else {
		OVITO_CHECK_OPENGL(glColor4(color));
	}

	OVITO_CHECK_OPENGL(glDrawArrays(closed ? GL_LINE_LOOP : GL_LINE_STRIP, 0, count));

	if(glformat().majorVersion() >= 3) {
		shader->disableAttributeArray("position");
	}
	else {
		OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
	}
	shader->release();
	if(wasDepthTestEnabled) glEnable(GL_DEPTH_TEST);
}

/******************************************************************************
* Makes vertex IDs available to the shader.
******************************************************************************/
void ViewportSceneRenderer::activateVertexIDs(QOpenGLShaderProgram* shader, GLint vertexCount, bool alwaysUseVBO)
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
void ViewportSceneRenderer::deactivateVertexIDs(QOpenGLShaderProgram* shader, bool alwaysUseVBO)
{
	if(glformat().majorVersion() < 3 || alwaysUseVBO)
		shader->disableAttributeArray("vertexID");
}

/******************************************************************************
* Returns the line rendering width to use in object picking mode.
******************************************************************************/
FloatType ViewportSceneRenderer::defaultLinePickingWidth()
{
	FloatType devicePixelRatio = 1;
	if(glcontext() && glcontext()->screen())
		devicePixelRatio = glcontext()->screen()->devicePixelRatio();
	return 12.0f * devicePixelRatio;
}

/******************************************************************************
* Determines the range of the construction grid to display.
******************************************************************************/
std::tuple<FloatType, Box2I> ViewportSceneRenderer::determineGridRange(Viewport* vp)
{
	// Determine the area of the construction grid that is visible in the viewport.
	static const Point2 testPoints[] = {
		{-1,-1}, {1,-1}, {1, 1}, {-1, 1}, {0,1}, {0,-1}, {1,0}, {-1,0},
		{0,1}, {0,-1}, {1,0}, {-1,0}, {-1, 0.5}, {-1,-0.5}, {1,-0.5}, {1,0.5}, {0,0}
	};

	// Compute intersection points of test rays with grid plane.
	Box2 visibleGridRect;
	size_t numberOfIntersections = 0;
	for(size_t i = 0; i < sizeof(testPoints)/sizeof(testPoints[0]); i++) {
		Point3 p;
		if(vp->computeConstructionPlaneIntersection(testPoints[i], p, 0.1f)) {
			numberOfIntersections++;
			visibleGridRect.addPoint(p.x(), p.y());
		}
	}

	if(numberOfIntersections < 2) {
		// Cannot determine visible parts of the grid.
        return std::tuple<FloatType, Box2I>(0.0f, Box2I());
	}

	// Determine grid spacing adaptively.
	Point3 gridCenter(visibleGridRect.center().x(), visibleGridRect.center().y(), 0);
	FloatType gridSpacing = vp->nonScalingSize(vp->gridMatrix() * gridCenter) * 2.0f;
	// Round to nearest power of 10.
	gridSpacing = pow((FloatType)10, floor(log10(gridSpacing)));

	// Determine how many grid lines need to be rendered.
	int xstart = (int)floor(visibleGridRect.minc.x() / (gridSpacing * 10)) * 10;
	int xend = (int)ceil(visibleGridRect.maxc.x() / (gridSpacing * 10)) * 10;
	int ystart = (int)floor(visibleGridRect.minc.y() / (gridSpacing * 10)) * 10;
	int yend = (int)ceil(visibleGridRect.maxc.y() / (gridSpacing * 10)) * 10;

	return std::tuple<FloatType, Box2I>(gridSpacing, Box2I(Point2I(xstart, ystart), Point2I(xend, yend)));
}

/******************************************************************************
* Renders the construction grid.
******************************************************************************/
void ViewportSceneRenderer::renderGrid()
{
	if(isPicking())
		return;

	FloatType gridSpacing;
	Box2I gridRange;
	std::tie(gridSpacing, gridRange) = determineGridRange(viewport());
	if(gridSpacing <= 0) return;

	// Determine how many grid lines need to be rendered.
	int xstart = gridRange.minc.x();
	int ystart = gridRange.minc.y();
	int numLinesX = gridRange.size(0) + 1;
	int numLinesY = gridRange.size(1) + 1;

	FloatType xstartF = (FloatType)xstart * gridSpacing;
	FloatType ystartF = (FloatType)ystart * gridSpacing;
	FloatType xendF = (FloatType)(xstart + numLinesX - 1) * gridSpacing;
	FloatType yendF = (FloatType)(ystart + numLinesY - 1) * gridSpacing;

	// Allocate vertex buffer.
	int numVertices = 2 * (numLinesX + numLinesY);
	std::unique_ptr<Point3[]> vertexPositions(new Point3[numVertices]);
	std::unique_ptr<ColorA[]> vertexColors(new ColorA[numVertices]);

	// Build lines array.
	ColorA color = Viewport::viewportColor(ViewportSettings::COLOR_GRID);
	ColorA majorColor = Viewport::viewportColor(ViewportSettings::COLOR_GRID_INTENS);
	ColorA majorMajorColor = Viewport::viewportColor(ViewportSettings::COLOR_GRID_AXIS);

	Point3* v = vertexPositions.get();
	ColorA* c = vertexColors.get();
	FloatType x = xstartF;
	for(int i = xstart; i < xstart + numLinesX; i++, x += gridSpacing, c += 2) {
		*v++ = Point3(x, ystartF, 0);
		*v++ = Point3(x, yendF, 0);
		if((i % 10) != 0)
			c[0] = c[1] = color;
		else if(i != 0)
			c[0] = c[1] = majorColor;
		else
			c[0] = c[1] = majorMajorColor;
	}
	FloatType y = ystartF;
	for(int i = ystart; i < ystart + numLinesY; i++, y += gridSpacing, c += 2) {
		*v++ = Point3(xstartF, y, 0);
		*v++ = Point3(xendF, y, 0);
		if((i % 10) != 0)
			c[0] = c[1] = color;
		else if(i != 0)
			c[0] = c[1] = majorColor;
		else
			c[0] = c[1] = majorMajorColor;
	}
	OVITO_ASSERT(c == vertexColors.get() + numVertices);

	// Render grid lines.
	setWorldTransform(viewport()->gridMatrix());
	if(!_constructionGridGeometry || !_constructionGridGeometry->isValid(this))
		_constructionGridGeometry = createLinePrimitive();
	_constructionGridGeometry->setVertexCount(numVertices);
	_constructionGridGeometry->setVertexPositions(vertexPositions.get());
	_constructionGridGeometry->setVertexColors(vertexColors.get());
	_constructionGridGeometry->render(this);
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Reports OpenGL error status codes.
******************************************************************************/
void checkOpenGLErrorStatus(const char* command, const char* sourceFile, int sourceLine)
{
	GLenum error;
	while((error = ::glGetError()) != GL_NO_ERROR) {
		qDebug() << "WARNING: OpenGL call" << command << "failed "
				"in line" << sourceLine << "of file" << sourceFile
				<< "with error" << ViewportSceneRenderer::openglErrorString(error);
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
