#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window2 : public ParticleWindow
{
public:

	/// Constructor.
	Window2(int id = 2) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return std::tuple<QString, QString, QString>(
			":/core/glsl/particles/imposter/sphere/without_depth.vs",
			":/core/glsl/particles/imposter/sphere/without_depth.fs",
			":/core/glsl/particles/imposter/sphere/without_depth.gs");
	}

	virtual void renderContent() override {
		if(!_billboardTexture.isCreated())
			initializeBillboardTexture();

		using namespace Ovito;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		initParticleBuffers(1);

		OVITO_CHECK_OPENGL(shader->bind());

		activateBillboardTexture();

		// Need to render only the front facing sides.
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		shader->setUniformValue("projection_matrix", (QMatrix4x4)projParams().projectionMatrix);
		shader->setUniformValue("modelview_matrix", (QMatrix4x4)modelViewTM());
		shader->setUniformValue("modelviewprojection_matrix", (QMatrix4x4)(projParams().projectionMatrix * modelViewTM()));

		_positionsBuffer.bindPositions(this, shader);
		_radiiBuffer.bind(this, shader, "particle_radius", GL_FLOAT, 0, 1);
		_colorsBuffer.bindColors(this, shader, 3);

		activateVertexIDs(shader, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement());

		// By default, render particles in arbitrary order.
		OVITO_CHECK_OPENGL(glDrawArrays(GL_POINTS, 0, _positionsBuffer.elementCount()));

		deactivateVertexIDs(shader);

		_positionsBuffer.detachPositions(this, shader);
		_radiiBuffer.detach(this, shader, "particle_radius");
		_colorsBuffer.detachColors(this, shader);

		shader->release();
		deactivateBillboardTexture();
	}

};

