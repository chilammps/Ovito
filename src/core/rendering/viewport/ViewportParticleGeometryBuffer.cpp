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
#include "ViewportParticleGeometryBuffer.h"
#include "ViewportSceneRenderer.h"

/// The maximum resolution of the texture used for billboard rendering of atoms. Specified as a power of two.
#define BILLBOARD_TEXTURE_LEVELS 	8

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ViewportParticleGeometryBuffer, ParticleGeometryBuffer);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportParticleGeometryBuffer::ViewportParticleGeometryBuffer(ViewportSceneRenderer* renderer) :
	_renderer(renderer),
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_particleCount(-1),
	_flatImposterShader(nullptr),
	_shadedImposterShaderWithoutDepth(nullptr),
	_shadedImposterShaderWithDepth(nullptr),
	_raytracedSphereShader(nullptr)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glPositionsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glPositionsBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glRadiiBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glRadiiBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	if(!_glColorsBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glColorsBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

	initializeBillboardTextures();

	// Initialize OpenGL shaders.
	if(QOpenGLShaderProgram::hasOpenGLShaderPrograms() && QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Vertex) && QOpenGLShader::hasOpenGLShaders(QOpenGLShader::Fragment)) {
		try {
			_flatImposterShader = loadShaderProgram("particle_flat_sphere", ":/core/glsl/particle_sprite_sphere_without_depth.vertex.glsl", ":/core/glsl/particle_flat.fragment.glsl");
			_shadedImposterShaderWithoutDepth = loadShaderProgram("particle_textured_sprite_sphere_without_depth", ":/core/glsl/particle_sprite_sphere_without_depth.vertex.glsl", ":/core/glsl/particle_sprite_sphere_without_depth.fragment.glsl");
			_shadedImposterShaderWithDepth = loadShaderProgram("particle_textured_sprite_sphere_with_depth", ":/core/glsl/particle_sprite_sphere_with_depth.vertex.glsl", ":/core/glsl/particle_sprite_sphere_with_depth.fragment.glsl");
			_raytracedSphereShader = loadShaderProgram("particle_raytraced_sphere", ":/core/glsl/particle_raytraced_sphere.vertex.glsl", ":/core/glsl/particle_raytraced_sphere.fragment.glsl", ":/core/glsl/particle_raytraced_sphere.geometry.glsl");
		}
		catch(const Exception& ex) {
			ex.logError();
			_flatImposterShader = nullptr;
			_shadedImposterShaderWithoutDepth = nullptr;
			_shadedImposterShaderWithDepth = nullptr;
			_raytracedSphereShader = nullptr;
		}
	}
}

/******************************************************************************
* Destructor.
******************************************************************************/
ViewportParticleGeometryBuffer::~ViewportParticleGeometryBuffer()
{
	destroyOpenGLResources();
}

/******************************************************************************
* Allocates a particle buffer with the given number of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setSize(int particleCount)
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_glRadiiBuffer.isCreated());
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(particleCount >= 0);
	OVITO_ASSERT(particleCount < std::numeric_limits<int>::max() / sizeof(Point3));
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	_particleCount = particleCount;
}

/******************************************************************************
* Sets the coordinates of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticlePositions(const Point3* coordinates)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glPositionsBuffer.allocate(coordinates, _particleCount * sizeof(Point3));
	_glPositionsBuffer.release();
}

/******************************************************************************
* Sets the radii of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleRadii(const FloatType* radii)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glRadiiBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glRadiiBuffer.allocate(radii, _particleCount * sizeof(FloatType));
	_glRadiiBuffer.release();
}

/******************************************************************************
* Sets the radius of all particles to the given value.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleRadius(FloatType radius)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glRadiiBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glRadiiBuffer.allocate(_particleCount * sizeof(FloatType));
	FloatType* bufferData = static_cast<FloatType*>(_glRadiiBuffer.map(QOpenGLBuffer::WriteOnly));
	if(!bufferData)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
	std::fill(bufferData, bufferData + _particleCount, radius);
	_glRadiiBuffer.unmap();
	_glRadiiBuffer.release();
}

/******************************************************************************
* Sets the colors of the particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleColors(const Color* colors)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glColorsBuffer.allocate(colors, _particleCount * sizeof(Color));
	_glColorsBuffer.release();
}

/******************************************************************************
* Sets the color of all particles to the given value.
******************************************************************************/
void ViewportParticleGeometryBuffer::setParticleColor(const Color color)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);
	OVITO_ASSERT(_glColorsBuffer.isCreated());
	OVITO_ASSERT(_particleCount >= 0);

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_glColorsBuffer.allocate(_particleCount * sizeof(Color));
	Color* bufferData = static_cast<Color*>(_glColorsBuffer.map(QOpenGLBuffer::WriteOnly));
	if(!bufferData)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
	std::fill(bufferData, bufferData + _particleCount, color);
	_glColorsBuffer.unmap();
	_glColorsBuffer.release();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportParticleGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _glPositionsBuffer.isCreated()
			&& _particleCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportParticleGeometryBuffer::render()
{
	OVITO_ASSERT(_glPositionsBuffer.isCreated());
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	OVITO_ASSERT(_particleCount >= 0);

	// Disable lighting.
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glAlphaFunc(GL_GREATER, 0);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);

	if(renderingQuality() < HighQuality || shadingMode() == FlatShading || _raytracedSphereShader == nullptr)
		renderPointSprites();
	else
		renderRaytracedSpheres();

	// Cleanup
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glPopAttrib();
}

/******************************************************************************
* Renders the particles using OpenGL point sprites.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderPointSprites()
{
	OVITO_STATIC_ASSERT(sizeof(FloatType) == 4);
	OVITO_STATIC_ASSERT(sizeof(Color) == 12);
	OVITO_STATIC_ASSERT(sizeof(Point3) == 12);

	// Use point sprites.
	OVITO_CHECK_OPENGL(glEnable(GL_POINT_SPRITE));

	// Load billboard texture.
	if(shadingMode() == FlatShading)
		activateBillboardTexture(FLAT_TEXTURE);
	else if(_shadedImposterShaderWithDepth && _shadedImposterShaderWithoutDepth)
		activateBillboardTexture(FRAGMENT_SHADER_TEXTURE);
	else
		activateBillboardTexture(FLAT_TEXTURE);

	// This is how our point sprite's size will be modified by its
	// distance from the viewer
	float param = renderer()->projParams().projectionMatrix(1,1) * renderer()->viewport()->size().height();
	if(renderer()->projParams().isPerspective) {
		float quadratic[] = { 0, 0, 100.0f / (param * param) };
		renderer()->glfuncs()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, quadratic);
		OVITO_CHECK_OPENGL(glPointSize(10.0));
	}
	else {
		float constant[] = { 1, 0, 0 };
		renderer()->glfuncs()->glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, constant);
		OVITO_CHECK_OPENGL(glPointSize(param));
	}
	// No fading of small points.
	OVITO_CHECK_OPENGL(renderer()->glfuncs()->glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 0.0f));
	OVITO_CHECK_OPENGL(renderer()->glfuncs()->glPointParameterf(GL_POINT_SIZE_MIN, 0.01f));

	// Specify point sprite texture coordinate replacement mode.
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

	// Activate OpenGL shader program.
	QOpenGLShaderProgram* shader =
			(shadingMode() == FlatShading) ? _flatImposterShader :
			(renderingQuality() == LowQuality ? _shadedImposterShaderWithoutDepth : _shadedImposterShaderWithDepth);
	if(shader && shader->bind()) {
		// Let the vertex shader compute the point size.
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		shader->setUniformValue("basePointSize", (float)param);

		// Pass particle radii to vertex shader.
		if(!_glRadiiBuffer.bind())
			throw Exception(tr("Failed to bind OpenGL vertex buffer."));
		shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
		shader->enableAttributeArray("particle_radius");
		_glRadiiBuffer.release();
	}
	else shader = nullptr;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, sizeof(Point3), NULL));
	_glPositionsBuffer.release();

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(glColorPointer(3, GL_FLOAT, sizeof(Color), NULL));
	_glColorsBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	// Render specular highlights in a second pass if not using shader program.
	if(shadingMode() != FlatShading && !shader) {

		activateBillboardTexture(SPECULAR_TEXTURE);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);
		GLint oldDepthFunc;
		glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
		glDepthFunc(GL_LEQUAL);
		glDisableClientState(GL_COLOR_ARRAY);
		glColor4f(1,1,1,1);

		OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

		glDepthFunc(oldDepthFunc);
	}

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	if(shader) {
		shader->disableAttributeArray("particle_radius");
		shader->release();
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	}

	// Cleanup
	glDisable(GL_POINT_SPRITE);
}

/******************************************************************************
* Renders the particles using raytracing implemented in an OpenGL fragment shader.
******************************************************************************/
void ViewportParticleGeometryBuffer::renderRaytracedSpheres()
{
	glDisable(GL_TEXTURE_2D);

	OVITO_CHECK_POINTER(_raytracedSphereShader);
	QOpenGLShaderProgram* shader = _raytracedSphereShader;

	shader->bind();

	//GLint viewportCoords[4];
	//glGetIntegerv(GL_VIEWPORT, viewportCoords);
	//shader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
	//shader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if(!_glPositionsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, sizeof(Point3), NULL));
	_glPositionsBuffer.release();

	if(!_glColorsBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	OVITO_CHECK_OPENGL(glColorPointer(3, GL_FLOAT, sizeof(Color), NULL));
	_glColorsBuffer.release();

	// Pass particle radii to vertex and geometry shader.
	if(!_glRadiiBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	shader->setAttributeBuffer("particle_radius", GL_FLOAT, 0, 1);
	shader->enableAttributeArray("particle_radius");
	_glRadiiBuffer.release();

	OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _particleCount));

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	shader->disableAttributeArray("particle_radius");
	shader->release();
}

/******************************************************************************
* Creates the textures used for billboard rendering of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::initializeBillboardTextures()
{
	static std::vector<std::array<GLubyte,2>> textureImages[NUM_TEXTURES][BILLBOARD_TEXTURE_LEVELS];
	static bool generatedImages = false;

	if(generatedImages == false) {
		generatedImages = true;
		for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
			int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));
			for(int texture = 0; texture < NUM_TEXTURES; texture++)
				textureImages[texture][mipmapLevel].resize(resolution*resolution);
			size_t pixelOffset = 0;
			for(int y = 0; y < resolution; y++) {
				for(int x = 0; x < resolution; x++, pixelOffset++) {
					Vector2 r((FloatType(x - resolution/2) + 0.5) / (resolution/2), (FloatType(y - resolution/2) + 0.5) / (resolution/2));
					FloatType r2 = r.squaredLength();
					FloatType r2_clamped = std::min(r2, FloatType(1));
					FloatType diffuse_brightness = sqrt(1 - r2_clamped) * 0.6 + 0.4;

					textureImages[DIFFUSE_TEXTURE][mipmapLevel][pixelOffset][0] =
							textureImages[FRAGMENT_SHADER_TEXTURE][mipmapLevel][pixelOffset][0] =
									(GLubyte)(std::min(diffuse_brightness, (FloatType)1.0) * 255.0);

					if(r2 < 1.0) {
						// Set opaque alpha value.
						textureImages[FLAT_TEXTURE][mipmapLevel][pixelOffset][1] = 255;
						textureImages[DIFFUSE_TEXTURE][mipmapLevel][pixelOffset][1] = 255;

						// Store specular brightness in alpha channel of texture.
						Vector2 sr = r + Vector2(0.6883, 0.982);
						FloatType specular = std::max(FloatType(1) - sr.squaredLength(), FloatType(0));
						specular *= specular;
						specular *= specular * (1 - r2_clamped*r2_clamped);
						textureImages[SPECULAR_TEXTURE][mipmapLevel][pixelOffset][1] =
								textureImages[FRAGMENT_SHADER_TEXTURE][mipmapLevel][pixelOffset][1] =
										(GLubyte)(std::min(specular, FloatType(1)) * 255.0);
					}
					else {
						// Set transparent pixel.
						textureImages[FLAT_TEXTURE][mipmapLevel][pixelOffset][1] = 0;
						textureImages[FRAGMENT_SHADER_TEXTURE][mipmapLevel][pixelOffset][1] = 0;
						textureImages[DIFFUSE_TEXTURE][mipmapLevel][pixelOffset][1] = 0;
						textureImages[SPECULAR_TEXTURE][mipmapLevel][pixelOffset][1] = 0;
					}

					// Flat-colored atoms appear at full brightness.
					textureImages[FLAT_TEXTURE][mipmapLevel][pixelOffset][0] = 255;

					// White specular color.
					textureImages[SPECULAR_TEXTURE][mipmapLevel][x+y*resolution][0] = 255;
				}
			}
		}
	}

	// Create OpenGL textures
	glGenTextures(NUM_TEXTURES, _textures);

	// Make sure textures get deleted again when this object is destroyed.
	attachOpenGLResources();

	// Transfer pixel data to OpenGL textures.
	for(int texture = 0; texture < NUM_TEXTURES; texture++) {
		OVITO_CHECK_OPENGL(glBindTexture(GL_TEXTURE_2D, _textures[texture]));
		for(int mipmapLevel = 0; mipmapLevel < BILLBOARD_TEXTURE_LEVELS; mipmapLevel++) {
			int resolution = (1 << (BILLBOARD_TEXTURE_LEVELS - mipmapLevel - 1));
			OVITO_CHECK_OPENGL(glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_LUMINANCE_ALPHA,
					resolution, resolution, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, textureImages[texture][mipmapLevel].data()));
		}
	}
}

/******************************************************************************
* This method that takes care of freeing the shared OpenGL resources owned
* by this class.
******************************************************************************/
void ViewportParticleGeometryBuffer::freeOpenGLResources()
{
	OVITO_CHECK_OPENGL(glDeleteTextures(NUM_TEXTURES, _textures));
}

/******************************************************************************
* Activates a texture for billboard rendering of particles.
******************************************************************************/
void ViewportParticleGeometryBuffer::activateBillboardTexture(BillboardTexture which)
{
	glEnable(GL_TEXTURE_2D);

	if(!_textures[which])
		initializeBillboardTextures();

	OVITO_ASSERT(_textures[which]);
	glBindTexture(GL_TEXTURE_2D, _textures[which]);

	if(which == FRAGMENT_SHADER_TEXTURE)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	OVITO_ASSERT(BILLBOARD_TEXTURE_LEVELS >= 3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, BILLBOARD_TEXTURE_LEVELS - 3);
}

/******************************************************************************
* Loads an OpenGL shader program.
******************************************************************************/
QOpenGLShaderProgram* ViewportParticleGeometryBuffer::loadShaderProgram(const QString& id, const QString& vertexShaderFile, const QString& fragmentShaderFile, const QString& geometryShaderFile)
{
	// The OpenGL shaders are only created once per OpenGL context group.
	QOpenGLShaderProgram* program = _contextGroup->findChild<QOpenGLShaderProgram*>(id);
	if(program)
		return program;

	program = new QOpenGLShaderProgram(_contextGroup);
	program->setObjectName(id);
	if(!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderFile)) {
		qDebug() << "OpenGL shader log:";
		qDebug() << program->log();
		delete program;
		throw Exception(QString("The vertex shader source file %1 failed to compile. See log for details.").arg(vertexShaderFile));
	}

	if(!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderFile)) {
		qDebug() << "OpenGL shader log:";
		qDebug() << program->log();
		delete program;
		throw Exception(QString("The fragment shader source file %1 failed to compile. See log for details.").arg(fragmentShaderFile));
	}

	if(!geometryShaderFile.isEmpty()) {
		if(!program->addShaderFromSourceFile(QOpenGLShader::Geometry, geometryShaderFile)) {
			qDebug() << "OpenGL shader log:";
			qDebug() << program->log();
			delete program;
			throw Exception(QString("The geometry shader source file %1 failed to compile. See log for details.").arg(geometryShaderFile));
		}
	}

	if(!program->link()) {
		qDebug() << "OpenGL shader log:";
		qDebug() << program->log();
		delete program;
		throw Exception(QString("The OpenGL shader program %1 failed to link. See log for details.").arg(id));
	}

	OVITO_ASSERT(_contextGroup->findChild<QOpenGLShaderProgram*>(id) == program);
	return program;
}


};
