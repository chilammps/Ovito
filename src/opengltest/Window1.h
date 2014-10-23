#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window1 : public ParticleWindow
{
public:

	/// Constructor.
	Window1(int id = 1) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return std::tuple<QString, QString, QString>(
			":/core/glsl/particles/imposter/sphere/without_depth_tri.vs",
			":/core/glsl/particles/imposter/sphere/without_depth.fs",
			QString());
	}

	virtual void renderContent() override {
		if(!_billboardTexture.isCreated())
			initializeBillboardTexture();

		using namespace Ovito;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		initParticleBuffers(6);

		OVITO_CHECK_OPENGL(shader->bind());

		activateBillboardTexture();

		// Need to render only the front facing sides.
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);

		// The texture coordinates of a quad made of two triangles.
		static const QVector2D texcoords[6] = {{0,1},{1,1},{1,0},{0,1},{1,0},{0,0}};
		shader->setUniformValueArray("imposter_texcoords", &texcoords[0], 6);

		// The coordinate offsets of the six vertices of a quad made of two triangles.
		static const QVector4D voffsets[6] = {{-1,-1,0,0},{1,-1,0,0},{1,1,0,0},{-1,-1,0,0},{1,1,0,0},{-1,1,0,0}};
		shader->setUniformValueArray("imposter_voffsets", &voffsets[0], 6);

		shader->setUniformValue("projection_matrix", (QMatrix4x4)projParams().projectionMatrix);
		shader->setUniformValue("modelview_matrix", (QMatrix4x4)modelViewTM());
		shader->setUniformValue("modelviewprojection_matrix", (QMatrix4x4)(projParams().projectionMatrix * modelViewTM()));

		_positionsBuffer.bindPositions(this, shader);
		_radiiBuffer.bind(this, shader, "particle_radius", GL_FLOAT, 0, 1);
		_colorsBuffer.bindColors(this, shader, 3);

		activateVertexIDs(shader, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement());

		// By default, render particles in arbitrary order.
		OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLES, 0, _positionsBuffer.elementCount() * _positionsBuffer.verticesPerElement()));

		deactivateVertexIDs(shader);

		_positionsBuffer.detachPositions(this, shader);
		_radiiBuffer.detach(this, shader, "particle_radius");
		_colorsBuffer.detachColors(this, shader);

		shader->release();
		deactivateBillboardTexture();
	}

};

