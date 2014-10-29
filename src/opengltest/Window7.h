#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window7 : public ParticleWindow
{
public:

	/// Constructor.
	Window7(int id = 7) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return std::tuple<QString, QString, QString>(
				":/gltest/glsl/cube_flat2.vs",
				":/gltest/glsl/cube_flat2.fs",
				":/gltest/glsl/cube_flat2.gs");
	}

	virtual void renderContent() override {

		using namespace Ovito;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		initParticleBuffers(1);

		OVITO_CHECK_OPENGL(shader->bind());

		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);

		// This is to draw the cube with a single triangle strip.
		// The cube vertices:
		static const QVector3D cubeVerts[14] = {
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
		shader->setUniformValueArray("cubeVerts", cubeVerts, 14);

		shader->setUniformValue("projection_matrix", (QMatrix4x4)projParams().projectionMatrix);
		shader->setUniformValue("modelview_matrix", (QMatrix4x4)modelViewTM());
		shader->setUniformValue("modelviewprojection_matrix", (QMatrix4x4)(projParams().projectionMatrix * modelViewTM()));

		_positionsBuffer.bindPositions(this, shader);
		_radiiBuffer.bind(this, shader, "particle_radius", GL_FLOAT, 0, 1);
		_colorsBuffer.bindColors(this, shader, 3);

		activateVertexIDs(shader, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement());

		// By default, render particle in arbitrary order.
		OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _positionsBuffer.elementCount()));

		deactivateVertexIDs(shader);

		_positionsBuffer.detachPositions(this, shader);
		_radiiBuffer.detach(this, shader, "particle_radius");
		_colorsBuffer.detachColors(this, shader);

		shader->release();
	}

};

