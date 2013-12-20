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
#include "ViewportTriMeshGeometryBuffer.h"
#include "ViewportSceneRenderer.h"

namespace Ovito {

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportTriMeshGeometryBuffer::ViewportTriMeshGeometryBuffer(ViewportSceneRenderer* renderer) :
	_contextGroup(QOpenGLContextGroup::currentContextGroup())
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	// Initialize OpenGL shader.
	_shader = renderer->loadShaderProgram("mesh", ":/core/glsl/mesh/mesh.vs", ":/core/glsl/mesh/mesh.fs");
	_pickingShader = renderer->loadShaderProgram("mesh.picking", ":/core/glsl/mesh/picking/mesh.vs", ":/core/glsl/mesh/picking/mesh.fs");
}

/******************************************************************************
* Sets the mesh to be stored in this buffer object.
******************************************************************************/
void ViewportTriMeshGeometryBuffer::setMesh(const TriMesh& mesh, const ColorA& meshColor)
{
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	// Allocate render vertex buffer.
	_vertexBuffer.create(QOpenGLBuffer::StaticDraw, mesh.faceCount(), 3);

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
			if(mesh.hasVertexColors() == false)
				rv->color = defaultVertexColor;
			else
				rv->color = mesh.vertexColor(face->vertex(v));
			// There is no support for semi-transparent meshes yet.
			rv->color.a() = 1;
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
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportTriMeshGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _vertexBuffer.isCreated() && (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportTriMeshGeometryBuffer::render(SceneRenderer* renderer)
{
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(faceCount() <= 0 || !vpRenderer)
		return;

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
		_vertexBuffer.bindColors(vpRenderer, shader, 4, offsetof(ColoredVertexWithNormal, color));
		_vertexBuffer.bindNormals(vpRenderer, shader, offsetof(ColoredVertexWithNormal, normal));
	}
	else {
		_pickingShader->setUniformValue("pickingBaseID", (GLint)vpRenderer->registerSubObjectIDs(faceCount()));
		vpRenderer->activateVertexIDs(_pickingShader, _vertexBuffer.elementCount() * _vertexBuffer.verticesPerElement());
	}

	OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLES, 0, _vertexBuffer.elementCount() * _vertexBuffer.verticesPerElement()));

	_vertexBuffer.detachPositions(vpRenderer, shader);
	if(!renderer->isPicking()) {
		_vertexBuffer.detachColors(vpRenderer, shader);
		_vertexBuffer.detachNormals(vpRenderer, shader);
	}
	else {
		vpRenderer->deactivateVertexIDs(_pickingShader);
	}
	shader->release();

	OVITO_CHECK_OPENGL();
}

};
