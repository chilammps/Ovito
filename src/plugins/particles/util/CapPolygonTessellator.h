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

#ifndef __OVITO_CAP_POLY_TESSELLATOR_H
#define __OVITO_CAP_POLY_TESSELLATOR_H

#include <plugins/particles/Particles.h>
#include <core/utilities/mesh/TriMesh.h>
#include "polytess/glu.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief Helper class that can tessellate a set of non-convex polygons into triangles.
 */
class CapPolygonTessellator
{
public:

	/// Constructor.
	CapPolygonTessellator(TriMesh& output, size_t dim) : mesh(output), dimz(dim) {
		dimx = (dimz + 1) % 3;
		dimy = (dimz + 2) % 3;
		tess = gluNewTess();
		gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
		gluTessCallback(tess, GLU_TESS_ERROR_DATA, (void (*)())errorData);
		gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (void (*)())beginData);
		gluTessCallback(tess, GLU_TESS_END_DATA, (void (*)())endData);
		gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (void (*)())vertexData);
		gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (void (*)())combineData);
	}

	/// Destructor.
	~CapPolygonTessellator() {
		gluDeleteTess(tess);
	}

	void beginPolygon() {
		gluTessNormal(tess, 0, 0, 1);
		gluTessBeginPolygon(tess, this);
	}

	void endPolygon() {
		gluTessEndPolygon(tess);
	}

	void beginContour() {
		gluTessBeginContour(tess);
	}

	void endContour() {
		gluTessEndContour(tess);
	}

	void vertex(const Point2& pos) {
		double vertexCoord[3];
		vertexCoord[0] = pos.x();
		vertexCoord[1] = pos.y();
		vertexCoord[2] = 0;
		Point3 p;
		p[dimx] = pos.x();
		p[dimy] = pos.y();
		p[dimz] = 0;
		intptr_t vindex = mesh.addVertex(p);
		p[dimz] = 1;
		mesh.addVertex(p);
		gluTessVertex(tess, vertexCoord, reinterpret_cast<void*>(vindex));
	}

	static void beginData(int type, void* polygon_data) {
		CapPolygonTessellator* tessellator = static_cast<CapPolygonTessellator*>(polygon_data);
		tessellator->primitiveType = type;
		tessellator->vertices.clear();
	}

	static void endData(void* polygon_data) {
		CapPolygonTessellator* tessellator = static_cast<CapPolygonTessellator*>(polygon_data);
		if(tessellator->primitiveType == GL_TRIANGLE_FAN) {
			OVITO_ASSERT(tessellator->vertices.size() >= 4);
			int facetVertices[3];
			facetVertices[0] = tessellator->vertices[0];
			facetVertices[1] = tessellator->vertices[1];
			for(auto v = tessellator->vertices.cbegin() + 2; v != tessellator->vertices.cend(); ++v) {
				facetVertices[2] = *v;
				tessellator->mesh.addFace().setVertices(facetVertices[0], facetVertices[1], facetVertices[2]);
				tessellator->mesh.addFace().setVertices(facetVertices[2]+1, facetVertices[1]+1, facetVertices[0]+1);
				facetVertices[1] = facetVertices[2];
			}
		}
		else if(tessellator->primitiveType == GL_TRIANGLE_STRIP) {
			OVITO_ASSERT(tessellator->vertices.size() >= 3);
			int facetVertices[3];
			facetVertices[0] = tessellator->vertices[0];
			facetVertices[1] = tessellator->vertices[1];
			bool even = true;
			for(auto v = tessellator->vertices.cbegin() + 2; v != tessellator->vertices.cend(); ++v) {
				facetVertices[2] = *v;
				tessellator->mesh.addFace().setVertices(facetVertices[0], facetVertices[1], facetVertices[2]);
				tessellator->mesh.addFace().setVertices(facetVertices[2]+1, facetVertices[1]+1, facetVertices[0]+1);
				if(even)
					facetVertices[0] = facetVertices[2];
				else
					facetVertices[1] = facetVertices[2];
				even = !even;
			}
		}
		else if(tessellator->primitiveType == GL_TRIANGLES) {
			for(auto v = tessellator->vertices.cbegin(); v != tessellator->vertices.cend(); v += 3) {
				tessellator->mesh.addFace().setVertices(v[0], v[1], v[2]);
				tessellator->mesh.addFace().setVertices(v[2]+1, v[1]+1, v[0]+1);
			}
		}
		else OVITO_ASSERT(false);
	}

	static void vertexData(void* vertex_data, void* polygon_data) {
		CapPolygonTessellator* tessellator = static_cast<CapPolygonTessellator*>(polygon_data);
		tessellator->vertices.push_back(reinterpret_cast<intptr_t>(vertex_data));
	}

	static void combineData(double coords[3], void* vertex_data[4], float weight[4], void** outDatab, void* polygon_data) {
		CapPolygonTessellator* tessellator = static_cast<CapPolygonTessellator*>(polygon_data);
		Point3 p;
		p[tessellator->dimx] = coords[0];
		p[tessellator->dimy] = coords[1];
		p[tessellator->dimz] = 0;
		intptr_t vindex = tessellator->mesh.addVertex(p);
		*outDatab = reinterpret_cast<void*>(vindex);
		p[tessellator->dimz] = 1;
		tessellator->mesh.addVertex(p);
	}

	static void errorData(int errno, void* polygon_data) {
		if(errno == GLU_TESS_NEED_COMBINE_CALLBACK)
			qDebug() << "ERROR: Could not tessellate cap polygon. It contains overlapping contours.";
		else
			qDebug() << "ERROR: Could not tessellate cap polygon. Error code: " << errno;
	}

private:

	size_t dimx, dimy, dimz;
	GLUtesselator* tess;
	TriMesh& mesh;
	int primitiveType;
	std::vector<int> vertices;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CAP_POLY_TESSELLATOR_H
