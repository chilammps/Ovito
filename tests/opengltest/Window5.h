#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window5 : public ParticleWindow
{
public:

	/// Constructor.
	Window5(int id = 5) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return std::tuple<QString, QString, QString>(
				":/core/glsl/particles/geometry/cube/cube.vs",
				":/core/glsl/particles/geometry/cube/cube.fs",
				":/core/glsl/particles/geometry/cube/cube.gs");
	}

	virtual void renderContent() override {

		using namespace Ovito;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		initParticleBuffers(1);

		OVITO_CHECK_OPENGL(shader->bind());

		// Need to render only the front facing sides of the cubes.
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// This is to draw the cube with a single triangle strip.
		// The cube vertices:
		static const GLfloat cubeVerts[14][3] = {
			{ 1,  1,  1},
			{ 1, -1,  1},
			{ 1,  1, -1},
			{ 1, -1, -1},
			{-1, -1, -1},
			{ 1, -1,  1},
			{-1, -1,  1},
			{ 1,  1,  1},
			{-1,  1,  1},
			{ 1,  1, -1},
			{-1,  1, -1},
			{-1, -1, -1},
			{-1,  1,  1},
			{-1, -1,  1},
		};
		shader->setUniformValueArray("cubeVerts", &cubeVerts[0][0], 14, 3);

		// The normal vectors for the cube triangle strip.
		static const QVector3D normals[14] = {
			{ 1,  0,  0},
			{ 1,  0,  0},
			{ 1,  0,  0},
			{ 1,  0,  0},
			{ 0,  0, -1},
			{ 0, -1,  0},
			{ 0, -1,  0},
			{ 0,  0,  1},
			{ 0,  0,  1},
			{ 0,  1,  0},
			{ 0,  1,  0},
			{ 0,  0, -1},
			{-1,  0,  0},
			{-1,  0,  0}
		};
		OVITO_CHECK_OPENGL(shader->setUniformValueArray("normals", normals, 14));
		shader->setUniformValue("normal_matrix", (QMatrix3x3)(modelViewTM().linear().inverse().transposed()));

		shader->setUniformValue("projection_matrix", (QMatrix4x4)projParams().projectionMatrix);
		shader->setUniformValue("inverse_projection_matrix", (QMatrix4x4)projParams().inverseProjectionMatrix);
		shader->setUniformValue("modelview_matrix", (QMatrix4x4)modelViewTM());
		shader->setUniformValue("modelviewprojection_matrix", (QMatrix4x4)(projParams().projectionMatrix * modelViewTM()));
		shader->setUniformValue("is_perspective", projParams().isPerspective);

		GLint viewportCoords[4];
		glGetIntegerv(GL_VIEWPORT, viewportCoords);
		shader->setUniformValue("viewport_origin", (float)viewportCoords[0], (float)viewportCoords[1]);
		shader->setUniformValue("inverse_viewport_size", 2.0f / (float)viewportCoords[2], 2.0f / (float)viewportCoords[3]);

		_positionsBuffer.bindPositions(this, shader);
		_radiiBuffer.bind(this, shader, "particle_radius", GL_FLOAT, 0, 1);
		_colorsBuffer.bindColors(this, shader, 3);

		// By default, render particle in arbitrary order.
		OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _positionsBuffer.elementCount()));

		_positionsBuffer.detachPositions(this, shader);
		_radiiBuffer.detach(this, shader, "particle_radius");
		_colorsBuffer.detachColors(this, shader);

		shader->release();
	}

};

