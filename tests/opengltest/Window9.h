#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window9 : public ParticleWindow
{
public:

	/// Constructor.
	Window9(int id = 9) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return std::tuple<QString, QString, QString>(
				":/core/glsl/particles/geometry/sphere/sphere.vs",
				":/core/glsl/particles/geometry/sphere/sphere.fs",
				":/gltest/glsl/sphere_const_array.gs");
	}

	virtual void renderContent() override {

		using namespace Ovito;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		initParticleBuffers(1);

		OVITO_CHECK_OPENGL(shader->bind());

		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);

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

