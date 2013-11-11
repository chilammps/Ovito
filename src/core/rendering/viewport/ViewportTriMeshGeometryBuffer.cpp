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

IMPLEMENT_OVITO_OBJECT(Core, ViewportTriMeshGeometryBuffer, TriMeshGeometryBuffer);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportTriMeshGeometryBuffer::ViewportTriMeshGeometryBuffer(ViewportSceneRenderer* renderer) :
	_contextGroup(QOpenGLContextGroup::currentContextGroup()),
	_renderVertexCount(-1)
{
	OVITO_ASSERT(renderer->glcontext()->shareGroup() == _contextGroup);

	if(!_glVertexBuffer.create())
		throw Exception(tr("Failed to create OpenGL vertex buffer."));
	_glVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);

	// Initialize OpenGL shader.
	_shader = renderer->loadShaderProgram("mesh", ":/core/glsl/mesh/mesh.vs", ":/core/glsl/mesh/mesh.fs");
	_pickingShader = renderer->loadShaderProgram("mesh.picking", ":/core/glsl/mesh/picking/mesh.vs", ":/core/glsl/mesh/picking/mesh.fs");
}

/******************************************************************************
* Sets the mesh to be stored in this buffer object.
******************************************************************************/
void ViewportTriMeshGeometryBuffer::setMesh(const TriMesh& mesh, const ColorA& meshColor)
{
	OVITO_ASSERT(_glVertexBuffer.isCreated());
	OVITO_ASSERT(QOpenGLContextGroup::currentContextGroup() == _contextGroup);

	// Allocate render vertex buffer.
	if(!_glVertexBuffer.bind())
		throw Exception(tr("Failed to bind OpenGL vertex buffer."));
	_renderVertexCount = mesh.faceCount() * 3;
	_glVertexBuffer.allocate(_renderVertexCount * sizeof(ColoredVertexWithNormal));
	if(_renderVertexCount == 0) {
		_glVertexBuffer.release();
		return;
	}

	ColoredVertexWithNormal* renderVertices = static_cast<ColoredVertexWithNormal*>(_glVertexBuffer.map(QOpenGLBuffer::ReadWrite));
	if(!renderVertices)
		throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));

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
#if 1
			// There is not support for semi-transparent meshes yet.
			rv->color.a() = 1;
#endif
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

	_glVertexBuffer.unmap();
	_glVertexBuffer.release();
}

/******************************************************************************
* Returns true if the geometry buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool ViewportTriMeshGeometryBuffer::isValid(SceneRenderer* renderer)
{
	ViewportSceneRenderer* vpRenderer = qobject_cast<ViewportSceneRenderer*>(renderer);
	if(!vpRenderer) return false;
	return _glVertexBuffer.isCreated()
			&& _renderVertexCount >= 0
			&& (_contextGroup == vpRenderer->glcontext()->shareGroup());
}

/******************************************************************************
* Renders the geometry.
******************************************************************************/
void ViewportTriMeshGeometryBuffer::render(SceneRenderer* renderer, quint32 pickingBaseID)
{
	OVITO_ASSERT(_glVertexBuffer.isCreated());
	OVITO_ASSERT(_contextGroup == QOpenGLContextGroup::currentContextGroup());
	ViewportSceneRenderer* vpRenderer = dynamic_object_cast<ViewportSceneRenderer>(renderer);

	if(_renderVertexCount <= 0 || !vpRenderer)
		return;

	glDisable(GL_CULL_FACE);

	if(!renderer->isPicking()) {
		if(!_shader->bind())
			throw Exception(tr("Failed to bind OpenGL shader."));

		_shader->setUniformValue("modelview_projection_matrix", (QMatrix4x4)(vpRenderer->projParams().projectionMatrix * vpRenderer->modelViewTM()));
		_shader->setUniformValue("normal_matrix", (QMatrix3x3)(vpRenderer->modelViewTM().linear().inverse().transposed()));

		OVITO_CHECK_OPENGL(_glVertexBuffer.bind());
		if(vpRenderer->glformat().majorVersion() >= 3) {
			OVITO_CHECK_OPENGL(_shader->enableAttributeArray("vertex_pos"));
			OVITO_CHECK_OPENGL(_shader->setAttributeBuffer("vertex_pos", GL_FLOAT, offsetof(ColoredVertexWithNormal, pos), 3, sizeof(ColoredVertexWithNormal)));
			OVITO_CHECK_OPENGL(_shader->enableAttributeArray("vertex_normal"));
			OVITO_CHECK_OPENGL(_shader->setAttributeBuffer("vertex_normal", GL_FLOAT, offsetof(ColoredVertexWithNormal, normal), 3, sizeof(ColoredVertexWithNormal)));
			OVITO_CHECK_OPENGL(_shader->enableAttributeArray("vertex_color"));
			OVITO_CHECK_OPENGL(_shader->setAttributeBuffer("vertex_color", GL_FLOAT, offsetof(ColoredVertexWithNormal, color), 4, sizeof(ColoredVertexWithNormal)));
		}
		else {
			OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
			OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertexWithNormal), reinterpret_cast<const GLvoid*>(offsetof(ColoredVertexWithNormal, pos))));
			OVITO_CHECK_OPENGL(glEnableClientState(GL_NORMAL_ARRAY));
			OVITO_CHECK_OPENGL(glNormalPointer(GL_FLOAT, sizeof(ColoredVertexWithNormal), reinterpret_cast<const GLvoid*>(offsetof(ColoredVertexWithNormal, normal))));
			OVITO_CHECK_OPENGL(glEnableClientState(GL_COLOR_ARRAY));
			OVITO_CHECK_OPENGL(glColorPointer(4, GL_FLOAT, sizeof(ColoredVertexWithNormal), reinterpret_cast<const GLvoid*>(offsetof(ColoredVertexWithNormal, color))));
		}
		_glVertexBuffer.release();

		OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLES, 0, _renderVertexCount));

		if(vpRenderer->glformat().majorVersion() >= 3) {
			_shader->disableAttributeArray("vertex_pos");
			_shader->disableAttributeArray("vertex_normal");
			_shader->disableAttributeArray("vertex_color");
		}
		else {
			OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
			OVITO_CHECK_OPENGL(glDisableClientState(GL_NORMAL_ARRAY));
			OVITO_CHECK_OPENGL(glDisableClientState(GL_COLOR_ARRAY));
		}
		_shader->release();
	}
	else {
		if(!_pickingShader->bind())
			throw Exception(tr("Failed to bind OpenGL shader."));

		_pickingShader->setUniformValue("modelview_projection_matrix", (QMatrix4x4)(vpRenderer->projParams().projectionMatrix * vpRenderer->modelViewTM()));
		_pickingShader->setUniformValue("pickingBaseID", (GLint)pickingBaseID);

		OVITO_CHECK_OPENGL(_glVertexBuffer.bind());
		if(vpRenderer->glformat().majorVersion() >= 3) {
			OVITO_CHECK_OPENGL(_pickingShader->enableAttributeArray("vertex_pos"));
			OVITO_CHECK_OPENGL(_pickingShader->setAttributeBuffer("vertex_pos", GL_FLOAT, offsetof(ColoredVertexWithNormal, pos), 3, sizeof(ColoredVertexWithNormal)));
		}
		else {
			OVITO_CHECK_OPENGL(glEnableClientState(GL_VERTEX_ARRAY));
			OVITO_CHECK_OPENGL(glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertexWithNormal), reinterpret_cast<const GLvoid*>(offsetof(ColoredVertexWithNormal, pos))));
		}
		_glVertexBuffer.release();

		if(vpRenderer->glformat().majorVersion() < 3) {
			// Create and fill vertex index buffer.
			if(!_glIndexBuffer.isCreated()) {
				if(!_glIndexBuffer.create())
					throw Exception(tr("Failed to create OpenGL vertex buffer."));
				_glIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
				if(!_glIndexBuffer.bind())
					throw Exception(tr("Failed to bind OpenGL vertex buffer."));
				_glIndexBuffer.allocate(_renderVertexCount * sizeof(GLint));
				OVITO_ASSERT(_renderVertexCount > 0);
				GLint* bufferData = static_cast<GLint*>(_glIndexBuffer.map(QOpenGLBuffer::WriteOnly));
				if(!bufferData)
					throw Exception(tr("Failed to map OpenGL vertex buffer to memory."));
				for(GLint index = 0; index < _renderVertexCount; index++)
					bufferData[index] = index;
				_glIndexBuffer.unmap();
			}
			else {
				if(!_glIndexBuffer.bind())
					throw Exception(tr("Failed to bind OpenGL vertex buffer."));
			}
			_pickingShader->enableAttributeArray("vertexID");
			_pickingShader->setAttributeBuffer("vertexID", GL_INT, 0, 1);
			_glIndexBuffer.release();
		}

		OVITO_CHECK_OPENGL(glDrawArrays(GL_TRIANGLES, 0, _renderVertexCount));

		if(vpRenderer->glformat().majorVersion() >= 3) {
			_pickingShader->disableAttributeArray("vertex_pos");
		}
		else {
			OVITO_CHECK_OPENGL(glDisableClientState(GL_VERTEX_ARRAY));
			if(renderer->isPicking())
				_pickingShader->disableAttributeArray("vertexID");
		}
		_pickingShader->release();
	}

	OVITO_CHECK_OPENGL();
}

};
