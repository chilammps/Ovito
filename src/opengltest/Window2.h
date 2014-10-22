#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window2 : public ParticleWindow
{
public:

	/// Constructor.
	Window2(int id = 2) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return { ":/core/glsl/particles/imposter/sphere/without_depth.vs",
			":/core/glsl/particles/imposter/sphere/without_depth.fs",
			":/core/glsl/particles/imposter/sphere/without_depth.gs" };
	}

	virtual void renderContent() override {
		if(!_billboardTexture.isCreated())
			initializeBillboardTexture();

		int verticesPerParticle = 1;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		OpenGLBuffer<Vector3> _positionsBuffer(_id);
		_positionsBuffer.create(QOpenGLBuffer::StaticDraw, 2, verticesPerParticle);
		Vector3 pos[2] = {{0,0,0.5}, {0.4,0.4,0.5}};
		_positionsBuffer.fill(pos);

		OpenGLBuffer<Color> _colorsBuffer(_id);
		_colorsBuffer.create(QOpenGLBuffer::StaticDraw, 2, verticesPerParticle);
		Color colors[2] = {{1,0,0}, {0,1,0}};
		_colorsBuffer.fill(colors);

		OpenGLBuffer<FloatType> _radiiBuffer(_id);
		_radiiBuffer.create(QOpenGLBuffer::StaticDraw, 2, verticesPerParticle);
		_radiiBuffer.fillConstant(0.2f);

		OVITO_CHECK_OPENGL(shader->bind());

		activateBillboardTexture();

		// Need to render only the front facing sides.
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		shader->setUniformValue("projection_matrix", QMatrix4x4());
		shader->setUniformValue("modelview_matrix", QMatrix4x4());
		shader->setUniformValue("modelviewprojection_matrix", QMatrix4x4());

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

