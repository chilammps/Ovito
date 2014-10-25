#include "ParticleWindow.h"
#include "OpenGLBuffer.h"

class Window4 : public ParticleWindow
{
public:

	/// Constructor.
	Window4(int id = 4) : ParticleWindow(id) {}

	virtual std::tuple<QString, QString, QString> shaderFiles() const override {
		return std::tuple<QString, QString, QString>(
				":/core/glsl/particles/geometry/sphere/sphere_tristrip.vs",
				":/core/glsl/particles/geometry/sphere/sphere.fs",
				QString());
	}

	virtual void renderContent() override {

		using namespace Ovito;

		QOpenGLShaderProgram* shader = getShader();
		if(!shader) return;

		initParticleBuffers(14);

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

		// Prepare arrays required for glMultiDrawArrays().

		std::vector<GLint> _primitiveStartIndices;
		std::vector<GLsizei> _primitiveVertexCounts;
		_primitiveStartIndices.resize(particleCount());
		_primitiveVertexCounts.resize(particleCount());
		GLint index = 0;
		for(GLint& s : _primitiveStartIndices) {
			s = index;
			index += _positionsBuffer.verticesPerElement();
		}
		std::fill(_primitiveVertexCounts.begin(), _primitiveVertexCounts.end(), _positionsBuffer.verticesPerElement());

		OVITO_CHECK_OPENGL(glMultiDrawArrays(GL_TRIANGLE_STRIP,
				_primitiveStartIndices.data(),
				_primitiveVertexCounts.data(),
				_primitiveStartIndices.size()));

		_positionsBuffer.detachPositions(this, shader);
		_radiiBuffer.detach(this, shader, "particle_radius");
		_colorsBuffer.detachColors(this, shader);

		shader->release();
	}

};

