#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window3 : public ParticleWindow
{
public:

	/// Constructor.
	Window3(int id = 3) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return { ":/core/glsl/particles/geometry/sphere/sphere.vs",
			":/core/glsl/particles/geometry/sphere/sphere.fs",
			":/core/glsl/particles/geometry/sphere/sphere.gs" };
	}

	virtual void renderContent() override {

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

		shader->setUniformValue("projection_matrix", QMatrix4x4());
		shader->setUniformValue("inverse_projection_matrix", QMatrix4x4());
		shader->setUniformValue("modelview_matrix", QMatrix4x4());
		shader->setUniformValue("modelviewprojection_matrix", QMatrix4x4());
		shader->setUniformValue("is_perspective", false);

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

