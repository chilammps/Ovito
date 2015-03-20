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
#include "OpenGLMeshPrimitive.h"
#include "ViewportSceneRenderer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructor.
******************************************************************************/
OpenGLMeshPrimitive::OpenGLMeshPrimitive(ViewportSceneRenderer* renderer) :
	_contextGroup(QOpenGLContextGroup::currentContextGroup()), _hasAlpha(false)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shader.
	_shader = renderer->loadShaderProgram("mesh", ":/core/glsl/mesh/mesh.vs", ":/core/glsl/mesh/mesh.fs");
	_pickingShader = renderer->loadShaderProgram("mesh.picking", ":/core/glsl/mesh/picking/mesh.vs", ":/core/glsl/mesh/picking/mesh.fs");
}

/******************************************************************************
* Sets the mesh to be stored in this buffer object.
******************************************************************************/
void OpenGLMeshPrimitive::setMesh(const TriMesh& mesh, const ColorA& meshColor)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	// Allocate render vertex buffer.
	_vertexBuffer.create(QOpenGLBuffer::StaticDraw, mesh.faceCount(), 3);
	if(mesh.hasVertexColors() || mesh.hasFaceColors())
		_hasAlpha = false;
	else
		_hasAlpha = (meshColor.a() != 1);

	if(mesh.faceCount() == 0)
		return;

	ColoredVertexWithNormal* renderVertices = _vertexBuffer.map(QOpenGLBuffer::ReadWrite);

	quint32 allMask = 0;

	// Compute face normals.
	std::vector<Vector_3<float>> faceNormals(mesh.faceCount());
	auto faceNormal = faceNormals.begin();
	for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++faceNormal) {
		const Point3& p0 = mesh.vertex(face->vertex(0));
		Vector3 d1 = mesh.vertex(face->vertex(1)) - p0;
		Vector3 d2 = mesh.vertex(face->vertex(2)) - p0;
		*faceNormal = d1.cross(d2);
		if(*faceNormal != Vector_3<float>::Zero()) {
			faceNormal->normalize();
			allMask |= face->smoothingGroups();
		}
	}

	// Initialize render vertices.
	ColoredVertexWithNormal* rv = renderVertices;
	faceNormal = faceNormals.begin();
	ColorAT<float> defaultVertexColor = meshColor;
	for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++faceNormal) {

		// Initialize render vertices for this face.
		for(size_t v = 0; v < 3; v++, rv++) {
			if(face->smoothingGroups())
				rv->normal = Vector_3<float>::Zero();
			else
				rv->normal = *faceNormal;
			rv->pos = mesh.vertex(face->vertex(v));
			if(mesh.hasVertexColors()) {
				rv->color = mesh.vertexColor(face->vertex(v));
				_hasAlpha |= (rv->color.a() != 1);
			}
			else if(mesh.hasFaceColors()) {
				rv->color = mesh.faceColor(face - mesh.faces().constBegin());
				_hasAlpha |= (rv->color.a() != 1);
			}
			else {
				rv->color = defaultVertexColor;
			}
		}
	}

	if(allMask) {
		std::vector<Vector_3<float>> groupVertexNormals(mesh.vertexCount());
		for(int group = 0; group < OVITO_MAX_NUM_SMOOTHING_GROUPS; group++) {
			quint32 groupMask = quint32(1) << group;
            if((allMask & groupMask) == 0)
            	continue;	// Group is not used.

			// Reset work arrays.
            std::fill(groupVertexNormals.begin(), groupVertexNormals.end(), Vector_3<float>::Zero());

			// Compute vertex normals at original vertices for current smoothing group.
            faceNormal = faceNormals.begin();
			for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++faceNormal) {
				// Skip faces that do not belong to the current smoothing group.
				if((face->smoothingGroups() & groupMask) == 0) continue;

				// Add face's normal to vertex normals.
				for(size_t fv = 0; fv < 3; fv++)
					groupVertexNormals[face->vertex(fv)] += *faceNormal;
			}

			// Transfer vertex normals from original vertices to render vertices.
			rv = renderVertices;
			for(const auto& face : mesh.faces()) {
				if(face.smoothingGroups() & groupMask) {
					for(size_t fv = 0; fv < 3; fv++, ++rv)
						rv->normal += groupVertexNormals[face.vertex(fv)];
				}
				else rv += 3;
			}
		}
	}

	_vertexBuffer.unmap();

	// Save a list of coordinates which will be used to sort faces back-to-front.
	if(_hasAlpha) {
		_triangleCoordinates.resize(mesh.faceCount());
		auto tc = _triangleCoordinates.begin();
		for(auto face = mesh.faces().constBegin(); face != mesh.faces().constEnd(); ++face, ++tc) {
			// Compute centroid of triangle.
			const auto& v1 = mesh.vertex(face->vertex(0));
			const auto& v2 = mesh.vertex(face->vertex(1));
			const auto& v3 = mesh.vertex(face->vertex(2));
			tc->x() = (v1.x() + v2.x() + v3.x()) / 3;
			tc->y() = (v1.y() + v2.y() + v3.y()) / 3;
			tc->z() = (v1.z() + v2.z() + v3.z()) / 3;
		}
	}
	else _triangleCoordinates.clear();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool OpenGLMeshPrimitive::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _vertexBuffer.isCreated() && (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void OpenGLMeshPrimitive::render(SceneRenderer* renderer)
{
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(faceCount() <= 0 || !vpRenderer)
		return;

	// If object is translucent, don't render it during the first rendering pass.
	// Queue primitive so that it gets rendered during the second pass.
	if(!renderer->isPicking() && _hasAlpha && vpRenderer->translucentPass() == false) {
		vpRenderer->registerTranslucentPrimitive(shared_from_this());
		return;
	}

	vpRenderer->rebindVAO();

	glDisable(GL_CULL_FACE);

	QOpenGLShaderProgram* shader;
	if(!renderer->isPicking())
		shader = _shader;
	else
		shader = _pickingShader;

	if(!shader->bind())
		throw Exception(QStringLiteral("Failed to bind OpenGL shader."));

	shader->setUniformValue("modelview_projection_matrix", (QMatrix4x4)(vpRenderer->projParams().projectionMatrix * vpRenderer->modelViewTM()));

	_vertexBuffer.bindPositions(vpRenderer, shader, offsetof(ColoredVertexWithNormal, pos));
	if(!renderer->isPicking()) {
		shader->setUniformValue("normal_matrix", (QMatrix3x3)(vpRenderer->modelViewTM().linear().inverse().transposed()));
		if(_hasAlpha) {
			glEnable(GL_BLEND);
			vpRenderer->glfuncs()->glBlendEquation(GL_FUNC_ADD);
			vpRenderer->glfuncs()->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		}
		_vertexBuffer.bindColors(vpRenderer, shader, 4, offsetof(ColoredVertexWithNormal, color));
		_vertexBuffer.bindNormals(vpRenderer, shader, offsetof(ColoredVertexWithNormal, normal));
	}
	else {
		_pickingShader->setUniformValue("pickingBaseID", (GLint)vpRenderer->registerSubObjectIDs(faceCount()));
		vpRenderer->activateVertexIDs(_pickingShader, _vertexBuffer.elementCount() * _vertexBuffer.verticesPerElement());
	}

	if(!renderer->isPicking() && _hasAlpha && !_triangleCoordinates.empty()) {
		OVITO_ASSERT(_triangleCoordinates.size() == faceCount());
		OVITO_ASSERT(_vertexBuffer.verticesPerElement() == 3);
		// Render faces in back-to-front order to avoid artifacts at overlapping translucent faces.
		std::vector<GLuint> indices(faceCount());
		std::iota(indices.begin(), indices.end(), 0);
		// First compute distance of each face from the camera along viewing direction (=camera z-axis).
		std::vector<FloatType> distances(faceCount());
		Vector3 direction = vpRenderer->modelViewTM().inverse().column(2);
		std::transform(_triangleCoordinates.begin(), _triangleCoordinates.end(), distances.begin(), [direction](const Point3& p) {
			return direction.dot(p - Point3::Origin());
		});
		// Now sort face indices with respect to distance (back-to-front order).
		std::sort(indices.begin(), indices.end(), [&distances](GLuint a, GLuint b) {
			return distances[a] < distances[b];
		});
		// Create OpenGL index buffer which can be used with glDrawElements.
		OpenGLBuffer<GLuint> primitiveIndices(QOpenGLBuffer::IndexBuffer);
		primitiveIndices.create(QOpenGLBuffer::StaticDraw, 3 * faceCount());
		GLuint* p = primitiveIndices.map(QOpenGLBuffer::WriteOnly);
		for(size_t i = 0; i < indices.size(); i++, p += 3)
			std::iota(p, p + 3, indices[i]*3);
		primitiveIndices.unmap();
		primitiveIndices.oglBuffer().bind();
		OVITO_CHECK_OPENGL(glDrawElements(GL_TRIANGLES, _vertexBuffer.elementCount() * _vertexBuffer.verticesPerElement(), GL_UNSIGNED_INT, nullptr));
		primitiveIndices.oglBuffer().release();
	}
	else {
		// Render faces in arbitrary order.
		OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLES, 0, _vertexBuffer.elementCount() * _vertexBuffer.verticesPerElement()));
	}

	_vertexBuffer.detachPositions(vpRenderer, shader);
	if(!renderer->isPicking()) {
		_vertexBuffer.detachColors(vpRenderer, shader);
		_vertexBuffer.detachNormals(vpRenderer, shader);
		if(_hasAlpha) glDisable(GL_BLEND);
	}
	else {
		vpRenderer->deactivateVertexIDs(_pickingShader);
	}
	shader->release();

	OVITO_REPORT_OPENGL_ERRORS();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
