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
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/scene/objects/geometry/TriMesh.h>
#include <plugins/particles/data/SimulationCell.h>
#include "DefectSurfaceDisplay.h"
#include "DefectSurface.h"
#include "CapPolygonTessellator.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, DefectSurfaceDisplay, DisplayObject)
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, DefectSurfaceDisplayEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(DefectSurfaceDisplay, DefectSurfaceDisplayEditor)
DEFINE_FLAGS_PROPERTY_FIELD(DefectSurfaceDisplay, _surfaceColor, "SurfaceColor", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(DefectSurfaceDisplay, _capColor, "CapColor", PROPERTY_FIELD_MEMORIZE)
DEFINE_PROPERTY_FIELD(DefectSurfaceDisplay, _showCap, "ShowCap")
DEFINE_PROPERTY_FIELD(DefectSurfaceDisplay, _smoothShading, "SmoothShading")
SET_PROPERTY_FIELD_LABEL(DefectSurfaceDisplay, _surfaceColor, "Surface color")
SET_PROPERTY_FIELD_LABEL(DefectSurfaceDisplay, _capColor, "Cap color")
SET_PROPERTY_FIELD_LABEL(DefectSurfaceDisplay, _showCap, "Show cap")
SET_PROPERTY_FIELD_LABEL(DefectSurfaceDisplay, _smoothShading, "Smooth shading")

/******************************************************************************
* Constructor.
******************************************************************************/
DefectSurfaceDisplay::DefectSurfaceDisplay() :
	_surfaceColor(1, 1, 1), _capColor(0.8, 0.8, 1.0), _showCap(true), _smoothShading(true)
{
	INIT_PROPERTY_FIELD(DefectSurfaceDisplay::_surfaceColor);
	INIT_PROPERTY_FIELD(DefectSurfaceDisplay::_capColor);
	INIT_PROPERTY_FIELD(DefectSurfaceDisplay::_showCap);
	INIT_PROPERTY_FIELD(DefectSurfaceDisplay::_smoothShading);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 DefectSurfaceDisplay::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	SimulationCell* cellObject = flowState.findObject<SimulationCell>();
	if(!cellObject)
		return Box3();

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(sceneObject, sceneObject ? sceneObject->revisionNumber() : 0, cellObject->data()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = Box3(Point3(0,0,0), Point3(1,1,1)).transformed(cellObject->cellMatrix());
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void DefectSurfaceDisplay::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Get the simulation cell.
	SimulationCell* cellObject = flowState.findObject<SimulationCell>();
	if(!cellObject)
		return;

	// Do we have to re-create the geometry buffers from scratch?
	bool recreateSurfaceBuffer = !_surfaceBuffer || !_surfaceBuffer->isValid(renderer);
	bool recreateCapBuffer = !_capBuffer || !_capBuffer->isValid(renderer);

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			sceneObject, sceneObject ? sceneObject->revisionNumber() : 0,
			cellObject->data(), surfaceColor(), capColor(), _smoothShading) || recreateSurfaceBuffer || recreateCapBuffer;

	// Re-create the geometry buffers if necessary.
	if(recreateSurfaceBuffer)
		_surfaceBuffer = renderer->createTriMeshGeometryBuffer();
	if(recreateCapBuffer && _showCap)
		_capBuffer = renderer->createTriMeshGeometryBuffer();

	// Update buffer contents.
	if(updateContents) {
		OORef<DefectSurface> defectSurfaceObj = sceneObject->convertTo<DefectSurface>(time);
		if(defectSurfaceObj) {
			TriMesh surfaceMesh;
			TriMesh capMesh;
			buildSurfaceMesh(defectSurfaceObj->mesh(), cellObject->data(), surfaceMesh);
			_surfaceBuffer->setMesh(surfaceMesh, ColorA(surfaceColor()));
			if(_showCap) {
				buildCapMesh(defectSurfaceObj->mesh(), cellObject->data(), capMesh);
				_capBuffer->setMesh(capMesh, ColorA(capColor()));
			}
		}
		else {
			_surfaceBuffer->setMesh(TriMesh(), ColorA(1,1,1,1));
			if(_showCap) _capBuffer->setMesh(TriMesh(), ColorA(1,1,1,1));
		}
	}

	// Handle picking of triangles.
	quint32 pickingBaseID = 0;
	if(renderer->isPicking())
		pickingBaseID = renderer->registerPickObject(contextNode, sceneObject, _surfaceBuffer->faceCount());

	_surfaceBuffer->render(renderer, pickingBaseID);

	if(_showCap) {
		if(renderer->isPicking())
			pickingBaseID = renderer->registerPickObject(contextNode, sceneObject, _capBuffer->faceCount());

		_capBuffer->render(renderer, pickingBaseID);
	}
	else _capBuffer.reset();
}

/******************************************************************************
* Generates the final triangle mesh, which will be rendered.
******************************************************************************/
void DefectSurfaceDisplay::buildSurfaceMesh(const HalfEdgeMesh& input, const SimulationCellData& cell, TriMesh& output)
{
	// Convert half-edge mesh to triangle mesh.
	input.convertToTriMesh(output);

	// Convert vertex positions to reduced coordinates.
	AffineTransformation inverseCellMatrix = cell.matrix().inverse();
	for(Point3& p : output.vertices())
		p = inverseCellMatrix * p;

	// Wrap mesh at periodic boundaries.
	for(size_t dim = 0; dim < 3; dim++) {
		if(cell.pbcFlags()[dim] == false) continue;

		// Make sure all vertices are located inside the periodic box.
		for(Point3& p : output.vertices()) {
			FloatType& c = p[dim];
			while(c < FloatType(0)) c += FloatType(1);
			while(c > FloatType(1)) c -= FloatType(1);
			OVITO_ASSERT(c >= FloatType(0) && c <= FloatType(1));
		}

		// Clip faces.
		int oldFaceCount = output.faceCount();
		int oldVertexCount = output.vertexCount();
		std::vector<Point3> newVertices;
		std::map<std::pair<int,int>,std::pair<int,int>> newVertexLookupMap;
		for(int findex = 0; findex < oldFaceCount; findex++) {
			splitFace(output, output.face(findex), oldVertexCount, newVertices, newVertexLookupMap, cell, dim);
		}

		// Insert newly created vertices into mesh.
		output.setVertexCount(oldVertexCount + newVertices.size());
		std::copy(newVertices.cbegin(), newVertices.cend(), output.vertices().begin() + oldVertexCount);
	}

	// Convert vertex positions back from reduced coordinates to absolute coordinates.
	AffineTransformation cellMatrix = cell.matrix();
	for(Point3& p : output.vertices())
		p = cellMatrix * p;

	// Assign smoothing group to faces to interpolate normals.
	if(_smoothShading) {
		for(auto& face : output.faces())
			face.setSmoothingGroups(1);
	}

	output.invalidateVertices();
	output.invalidateFaces();
}

/******************************************************************************
* Splits a triangle face at a periodic boundary.
******************************************************************************/
void DefectSurfaceDisplay::splitFace(TriMesh& output, TriMeshFace& face, int oldVertexCount, std::vector<Point3>& newVertices,
		std::map<std::pair<int,int>,std::pair<int,int>>& newVertexLookupMap, const SimulationCellData& cell, size_t dim)
{
	OVITO_ASSERT(face.vertex(0) != face.vertex(1));
	OVITO_ASSERT(face.vertex(1) != face.vertex(2));
	OVITO_ASSERT(face.vertex(2) != face.vertex(0));

	FloatType z[3];
	for(int v = 0; v < 3; v++)
		z[v] = output.vertex(face.vertex(v))[dim];
	FloatType zd[3] = { z[1] - z[0], z[2] - z[1], z[0] - z[2] };

	OVITO_ASSERT(z[1] - z[0] == -(z[0] - z[1]));
	OVITO_ASSERT(z[2] - z[1] == -(z[1] - z[2]));
	OVITO_ASSERT(z[0] - z[2] == -(z[2] - z[0]));

	if(std::abs(zd[0]) < 0.5f && std::abs(zd[1]) < 0.5f && std::abs(zd[2]) < 0.5f)
		return;	// Face is not crossing the periodic boundary.

	// Create four new vertices (or use existing ones created during splitting of adjacent faces).
	int properEdge = -1;
	int newVertexIndices[3][2];
	for(int i = 0; i < 3; i++) {
		if(std::abs(zd[i]) < 0.5f) {
			OVITO_ASSERT(properEdge == -1);
			properEdge = i;
			continue;
		}
		int vi1 = face.vertex(i);
		int vi2 = face.vertex((i+1)%3);
		int oi1, oi2;
		if(zd[i] <= -0.5f) {
			std::swap(vi1, vi2);
			oi1 = 1; oi2 = 0;
		}
		else {
			oi1 = 0; oi2 = 1;
		}
		auto entry = newVertexLookupMap.find(std::make_pair(vi1, vi2));
		if(entry != newVertexLookupMap.end()) {
			newVertexIndices[i][oi1] = entry->second.first;
			newVertexIndices[i][oi2] = entry->second.second;
		}
		else {
			Vector3 delta = output.vertex(vi2) - output.vertex(vi1);
			delta[dim] -= 1.0f;
			for(size_t d = dim + 1; d < 3; d++) {
				if(cell.pbcFlags()[d]) {
					FloatType& c = delta[d];
					while(c < FloatType(0.5)) c += FloatType(1);
					while(c > FloatType(0.5)) c -= FloatType(1);
				}
			}
			FloatType t = output.vertex(vi1)[dim] / (-delta[dim]);
			Point3 p = delta * t + output.vertex(vi1);
			newVertexIndices[i][oi1] = oldVertexCount + newVertices.size();
			newVertexIndices[i][oi2] = oldVertexCount + newVertices.size() + 1;
			newVertexLookupMap.insert(std::make_pair(std::pair<int,int>(vi1, vi2), std::pair<int,int>(newVertexIndices[i][oi1], newVertexIndices[i][oi2])));
			newVertices.push_back(p);
			p[dim] += 1.0f;
			newVertices.push_back(p);
		}
	}
	OVITO_ASSERT(properEdge != -1);

	// Build output triangles.
	int originalVertices[3] = { face.vertex(0), face.vertex(1), face.vertex(2) };
	face.setVertices(originalVertices[properEdge], originalVertices[(properEdge+1)%3], newVertexIndices[(properEdge+2)%3][1]);

	output.setFaceCount(output.faceCount() + 2);
	TriMeshFace& newFace1 = output.face(output.faceCount() - 2);
	TriMeshFace& newFace2 = output.face(output.faceCount() - 1);
	newFace1.setVertices(originalVertices[(properEdge+1)%3], newVertexIndices[(properEdge+1)%3][0], newVertexIndices[(properEdge+2)%3][1]);
	newFace2.setVertices(newVertexIndices[(properEdge+1)%3][1], originalVertices[(properEdge+2)%3], newVertexIndices[(properEdge+2)%3][0]);
}

/******************************************************************************
* Generates the triangle mesh for the PBC caps.
******************************************************************************/
void DefectSurfaceDisplay::buildCapMesh(const HalfEdgeMesh& input, const SimulationCellData& cell, TriMesh& output)
{
	// Convert vertex positions to reduced coordinates.
	AffineTransformation inverseCellMatrix = cell.matrix().inverse();
	std::vector<Point3> reducedPos(input.vertexCount());
	auto inputVertex = input.vertices().begin();
	for(Point3& p : reducedPos)
		p = inverseCellMatrix * (*inputVertex++)->pos();

	int isBoxCornerInside3DRegion = -1;

	// Create caps for each periodic boundary.
	for(size_t dim = 0; dim < 3; dim++) {
		if(cell.pbcFlags()[dim] == false) continue;

		// Make sure all vertices are located inside the periodic box.
		for(Point3& p : reducedPos) {
			FloatType& c = p[dim];
			while(c < FloatType(0)) c += FloatType(1);
			while(c > FloatType(1)) c -= FloatType(1);
		}

		// Reset 'visited' flag for all faces.
		input.clearFaceFlag(1);

		/// The list of clipped contours.
		std::vector<std::vector<Point2>> openContours;
		std::vector<std::vector<Point2>> closedContours;

		// Find a first edge that crosses the boundary.
		for(HalfEdgeMesh::Vertex* vert : input.vertices()) {
			for(HalfEdgeMesh::Edge* edge = vert->edges(); edge != nullptr; edge = edge->nextVertexEdge()) {
				// Skip faces that have already been visited.
				if(edge->face()->testFlag(1)) continue;

				const Point3& v1 = reducedPos[edge->vertex1()->index()];
				const Point3& v2 = reducedPos[edge->vertex2()->index()];
				if(v2[dim] - v1[dim] >= 0.5f) {
					std::vector<Point2> contour = traceContour(edge, reducedPos, cell, dim);
					clipContour(contour, { cell.pbcFlags()[(dim+1)%3],
											cell.pbcFlags()[(dim+2)%3] }, openContours, closedContours);
				}
			}
		}

		std::vector<std::vector<Point2>>& contourList = closedContours;
		std::vector<Point2> outputContour;

		// Close open contours.
		if(!openContours.empty()) {
			QBitArray visitedContours(openContours.size());
			for(auto c1 = openContours.begin(); c1 != openContours.end(); ++c1) {
				auto currentContour = c1;
				while(!visitedContours.testBit(currentContour - openContours.begin())) {
					outputContour.insert(outputContour.end(), currentContour->begin(), currentContour->end());
					visitedContours.setBit(currentContour - openContours.begin());

					FloatType exitSide = side(currentContour->back());

					// Find the next contour.
					FloatType entrySide;
					FloatType closestDist = FLOATTYPE_MAX;
					for(auto c = openContours.begin(); c != openContours.end(); ++c) {
						FloatType pos = side(c->front());
						FloatType dist = exitSide - pos;
						if(dist < 0.0f) dist += 4.0f;
						if(dist < closestDist) {
							closestDist = dist;
							currentContour = c;
							entrySide = pos;
						}
					}

					int exitCorner = (int)floor(exitSide);
					int entryCorner = (int)floor(entrySide);
					if(exitCorner != entryCorner || exitSide < entrySide) {
						for(int corner = exitCorner; ;) {
							switch(corner) {
							case 0: outputContour.push_back(Point2(0,0)); break;
							case 1: outputContour.push_back(Point2(0,1)); break;
							case 2: outputContour.push_back(Point2(1,1)); break;
							case 3: outputContour.push_back(Point2(1,0)); break;
							}
							corner = (corner + 3) % 4;
							if(corner == entryCorner) break;
						}
					}
				}

				if(!outputContour.empty()) {
					contourList.push_back(outputContour);
					outputContour.clear();
				}
			}
		}
		else {
			if(closedContours.empty()) {
				if(isBoxCornerInside3DRegion == -1)
					isBoxCornerInside3DRegion = isCornerInside3DRegion(input, reducedPos, cell.pbcFlags());
				if(isBoxCornerInside3DRegion)
					contourList.push_back( { Point2(0,0), Point2(1,0), Point2(1,1), Point2(0,1) });
			}
			else {
				if(isCornerInside2DRegion(closedContours))
					contourList.push_back( { Point2(0,0), Point2(1,0), Point2(1,1), Point2(0,1) });
			}
		}

#if 0
		size_t totalNumContourPoints = 0;
		for(const auto& c : contourList)
			totalNumContourPoints += c.size();

		QFile file(QString("dim%1.vtk").arg(dim));
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream stream(&file);
		stream << "# vtk DataFile Version 3.0" << endl;
		stream << "# Contours" << endl;
		stream << "ASCII" << endl;
		stream << "DATASET UNSTRUCTURED_GRID" << endl;
		stream << "POINTS " << totalNumContourPoints << " float" << endl;
		FloatType z = 0;
		for(const auto& contour : contourList) {
			z = 0;
			for(const Point2& p : contour) {
				stream << p.x() << " " << p.y() << " " << z << endl;
				z += 0.2 / contour.size();
			}
		}
		size_t numCells = contourList.size();
		stream << endl << "CELLS " << numCells << " " << (totalNumContourPoints + numCells) << endl;
		size_t pointIndex = 0;
		for(const auto& contour : contourList) {
			stream << contour.size();
			for(size_t i = 0; i < contour.size(); i++, pointIndex++)
				stream << " " << pointIndex;
			stream << endl;
		}
		stream << endl << "CELL_TYPES " << numCells << endl;
		for(size_t i = 0; i < numCells; i++)
			stream << "7" << endl;	// Polygon
//			stream << "4" << endl;	// Poly line
#endif

		// Feed contours into tessellator to create triangles.
		CapPolygonTessellator tessellator(output, dim);
		tessellator.beginPolygon();
		for(const auto& contour : contourList) {
			tessellator.beginContour();
			for(const Point2& p : contour)
				tessellator.vertex(p);
			tessellator.endContour();
		}
		tessellator.endPolygon();
	}

	// Convert vertex positions back from reduced coordinates to absolute coordinates.
	AffineTransformation cellMatrix = cell.matrix();
	for(Point3& p : output.vertices())
		p = cellMatrix * p;
}

/******************************************************************************
* Traces the closed contour of the surface-boundary intersection.
******************************************************************************/
std::vector<Point2> DefectSurfaceDisplay::traceContour(HalfEdgeMesh::Edge* firstEdge, const std::vector<Point3>& reducedPos, const SimulationCellData& cell, size_t dim)
{
	size_t dim1 = (dim + 1) % 3;
	size_t dim2 = (dim + 2) % 3;
	std::vector<Point2> contour;
	HalfEdgeMesh::Edge* edge = firstEdge;
	do {
		OVITO_ASSERT(!edge->face()->testFlag(1));

		// Mark face as visited.
		edge->face()->setFlag(1);

		// Compute intersection point.
		const Point3& v1 = reducedPos[edge->vertex1()->index()];
		const Point3& v2 = reducedPos[edge->vertex2()->index()];
		Vector3 delta = v2 - v1;
		OVITO_ASSERT(delta[dim] >= 0.5f);

		delta[dim] -= 1.0f;
		if(cell.pbcFlags()[dim1]) {
			FloatType& c = delta[dim1];
			while(c < FloatType(0.5)) c += FloatType(1);
			while(c > FloatType(0.5)) c -= FloatType(1);
		}
		if(cell.pbcFlags()[dim2]) {
			FloatType& c = delta[dim2];
			while(c < FloatType(0.5)) c += FloatType(1);
			while(c > FloatType(0.5)) c -= FloatType(1);
		}
		FloatType t = v1[dim] / delta[dim];
		FloatType x = v1[dim1] - delta[dim1] * t;
		FloatType y = v1[dim2] - delta[dim2] * t;
		contour.push_back({x,y});

		// Find the face edge that crosses the boundary in the reverse direction.
		for(;;) {
			edge = edge->nextFaceEdge();
			const Point3& v1 = reducedPos[edge->vertex1()->index()];
			const Point3& v2 = reducedPos[edge->vertex2()->index()];
			if(v2[dim] - v1[dim] <= -0.5f)
				break;
		}

		edge = edge->oppositeEdge();
	}
	while(edge != firstEdge);

	return contour;
}

/******************************************************************************
* Clips a 2d contour at a periodic boundary.
******************************************************************************/
void DefectSurfaceDisplay::clipContour(std::vector<Point2>& input, std::array<bool,2> pbcFlags, std::vector<std::vector<Point2>>& openContours, std::vector<std::vector<Point2>>& closedContours)
{
	if(!pbcFlags[0] && !pbcFlags[1]) {
		closedContours.push_back(std::move(input));
		return;
	}

	// Ensure all coordinates are within the primary image.
	if(pbcFlags[0]) {
		for(auto& v : input) {
			while(v.x() < FloatType(0)) v.x() += FloatType(1);
			while(v.x() > FloatType(1)) v.x() -= FloatType(1);
		}
	}
	if(pbcFlags[1]) {
		for(auto& v : input) {
			while(v.y() < FloatType(0)) v.y() += FloatType(1);
			while(v.y() > FloatType(1)) v.y() -= FloatType(1);
		}
	}

	std::vector<std::vector<Point2>> contours;
	contours.push_back({});

	auto v1 = input.cend() - 1;
	for(auto v2 = input.cbegin(); v2 != input.cend(); v1 = v2, ++v2) {
		contours.back().push_back(*v1);

		Vector2 delta = (*v2) - (*v1);
		if(std::abs(delta.x()) < 0.5f && std::abs(delta.y()) < 0.5f)
			continue;

		FloatType t[2] = { 1.0f, 1.0f };
		Vector2I crossDir(0, 0);
		for(size_t dim = 0; dim < 2; dim++) {
			if(pbcFlags[dim]) {
				if(delta[dim] >= 0.5f) {
					delta[dim] -= 1.0f;
					t[dim] = (*v1)[dim] / -delta[dim];
					crossDir[dim] = -1;
				}
				else if(delta[dim] <= -0.5f) {
					delta[dim] += 1.0f;
					t[dim] = (1.0f - (*v1)[dim]) / delta[dim];
					crossDir[dim] = +1;
				}
				OVITO_ASSERT(t[dim] >= 0 && t[dim] <= 1);
			}
		}

		Point2 base = *v1;
		if(t[0] < t[1]) {
			clipContourSegment(0, t[0], base, delta, crossDir[0], contours);
			if(crossDir[1] != 0)
				clipContourSegment(1, t[1], base, delta, crossDir[1], contours);
		}
		else if(t[1] < t[0]) {
			clipContourSegment(1, t[1], base, delta, crossDir[1], contours);
			if(crossDir[0] != 0)
				clipContourSegment(0, t[0], base, delta, crossDir[0], contours);
		}
	}

	if(contours.size() == 1) {
		closedContours.push_back(std::move(contours.back()));
	}
	else {
		auto& firstSegment = contours.front();
		auto& lastSegment = contours.back();
		firstSegment.insert(firstSegment.begin(), lastSegment.begin(), lastSegment.end());
		openContours.insert(openContours.begin(), std::make_move_iterator(contours.begin()), std::make_move_iterator(contours.end() - 1));
	}
}

void DefectSurfaceDisplay::clipContourSegment(size_t dim, FloatType t, Point2& base, Vector2& delta, int crossDir, std::vector<std::vector<Point2>>& contours)
{
	Point2 intersection = base + t * delta;
	intersection[dim] = (crossDir == -1) ? 0.0f : 1.0f;
	contours.back().push_back(intersection);
	intersection[dim] = (crossDir == +1) ? 0.0f : 1.0f;
	contours.push_back({intersection});
	base = intersection;
	delta *= (1.0f - t);
}

bool DefectSurfaceDisplay::isCornerInside2DRegion(const std::vector<std::vector<Point2>>& contours)
{
	OVITO_ASSERT(!contours.empty());

	// 2D version of the algorithm:
	//
	// J. Andreas Baerentzen and Henrik Aanaes
	// Signed Distance Computation Using the Angle Weighted Pseudonormal
	// IEEE Transactions on Visualization and Computer Graphics, Volume 11, Issue 3 (May 2005), Pages: 243 - 253

	bool isInside = true;

	// Determine which vertex is closest to the test point.
	std::vector<Point2>::const_iterator closestVertex = contours.front().end();
	FloatType closestDistanceSq = FLOATTYPE_MAX;
	for(const auto& contour : contours) {
		auto v1 = contour.end() - 1;
		for(auto v2 = contour.begin(); v2 != contour.end(); v1 = v2++) {
			Vector2 r = (*v1) - Point2::Origin();
			FloatType distanceSq = r.squaredLength();
			if(distanceSq < closestDistanceSq) {
				closestDistanceSq = distanceSq;
				closestVertex = v1;

				// Compute pseuso-normal at vertex.
				auto v0 = (v1 == contour.begin()) ? (contour.end() - 1) : (v1 - 1);
				Vector2 edgeDir = (*v2) - (*v0);
				Vector2 normal(edgeDir.y(), -edgeDir.x());
				isInside = (normal.dot(r) > 0);
			}

			// Check if any edge is closer to the test point.
			Vector2 edgeDir = (*v2) - (*v1);
			FloatType edgeLength = edgeDir.length();
			if(edgeLength <= FLOATTYPE_EPSILON) continue;
			edgeDir /= edgeLength;
			FloatType d = -edgeDir.dot(r);
			if(d <= 0 || d >= edgeLength) continue;
			Vector2 c = r + edgeDir * d;
			distanceSq = c.squaredLength();
			if(distanceSq < closestDistanceSq) {
				closestDistanceSq = distanceSq;

				// Compute normal at edge.
				Vector2 normal(edgeDir.y(), -edgeDir.x());
				isInside = (normal.dot(c) > 0);
			}
		}
	}

	return isInside;
}

bool DefectSurfaceDisplay::isCornerInside3DRegion(const HalfEdgeMesh& mesh, const std::vector<Point3>& reducedPos, const std::array<bool,3> pbcFlags)
{
	if(mesh.vertices().empty())
		return true;

	// 3D version of the algorithm:
	//
	// J. Andreas Baerentzen and Henrik Aanaes
	// Signed Distance Computation Using the Angle Weighted Pseudonormal
	// IEEE Transactions on Visualization and Computer Graphics, Volume 11, Issue 3 (May 2005), Pages: 243 - 253

	// Determine which vertex is closest to the test point.
	FloatType closestDistanceSq = FLOATTYPE_MAX;
	HalfEdgeMesh::Vertex* closestVertex = nullptr;
	Vector3 closestNormal, closestVector;
	for(HalfEdgeMesh::Vertex* v : mesh.vertices()) {
		Vector3 r = reducedPos[v->index()] - Point3::Origin();
		for(size_t k = 0; k < 3; k++) {
			if(pbcFlags[k]) {
				while(r[k] > FloatType( 0.5)) r[k] -= FloatType(1);
				while(r[k] < FloatType(-0.5)) r[k] += FloatType(1);
			}
		}
		FloatType distSq = r.squaredLength();
		if(distSq < closestDistanceSq) {
			closestDistanceSq = distSq;
			closestVertex = v;
			closestVector = r;
		}
	}

	// Check if any edge is closer to the test point than the closest vertex.
	for(HalfEdgeMesh::Vertex* v : mesh.vertices()) {
		for(HalfEdgeMesh::Edge* edge = v->edges(); edge != nullptr; edge = edge->nextVertexEdge()) {
			const Point3& p1 = reducedPos[edge->vertex1()->index()];
			const Point3& p2 = reducedPos[edge->vertex2()->index()];
			Vector3 edgeDir = p2 - p1;
			Vector3 r = p1 - Point3::Origin();
			for(size_t k = 0; k < 3; k++) {
				if(pbcFlags[k]) {
					while(r[k] > FloatType( 0.5)) r[k] -= FloatType(1);
					while(r[k] < FloatType(-0.5)) r[k] += FloatType(1);
					while(edgeDir[k] > FloatType( 0.5)) edgeDir[k] -= FloatType(1);
					while(edgeDir[k] < FloatType(-0.5)) edgeDir[k] += FloatType(1);
				}
			}
			FloatType edgeLength = edgeDir.length();
			if(edgeLength <= FLOATTYPE_EPSILON) continue;
			edgeDir /= edgeLength;
			FloatType d = -edgeDir.dot(r);
			if(d <= 0 || d >= edgeLength) continue;
			Vector3 c = r + edgeDir * d;
			FloatType distSq = c.squaredLength();
			if(distSq < closestDistanceSq) {
				closestDistanceSq = distSq;
				closestVertex = nullptr;
				closestVector = c;
				Vector3 e1 = reducedPos[edge->nextFaceEdge()->vertex2()->index()] - p1;
				Vector3 e2 = reducedPos[edge->oppositeEdge()->nextFaceEdge()->vertex2()->index()] - p1;
				for(size_t k = 0; k < 3; k++) {
					if(pbcFlags[k]) {
						while(e1[k] > FloatType( 0.5)) e1[k] -= FloatType(1);
						while(e1[k] < FloatType(-0.5)) e1[k] += FloatType(1);
						while(e2[k] > FloatType( 0.5)) e2[k] -= FloatType(1);
						while(e2[k] < FloatType(-0.5)) e2[k] += FloatType(1);
					}
				}
				closestNormal = edgeDir.cross(e1).normalized() + e2.cross(edgeDir).normalized();
			}
		}
	}

	// Check if any facet is closer to the test point than the closest vertex and the closest edge.
	HalfEdgeMesh::Face* closestFace = nullptr;
	for(HalfEdgeMesh::Face* face : mesh.faces()) {
		HalfEdgeMesh::Edge* edge1 = face->edges();
		HalfEdgeMesh::Edge* edge2 = edge1->nextFaceEdge();
		const Point3& p1 = reducedPos[edge1->vertex1()->index()];
		const Point3& p2 = reducedPos[edge1->vertex2()->index()];
		const Point3& p3 = reducedPos[edge2->vertex2()->index()];
		Vector3 edgeVectors[3];
		edgeVectors[0] = p2 - p1;
		edgeVectors[1] = p3 - p2;
		Vector3 r = p1 - Point3::Origin();
		for(size_t k = 0; k < 3; k++) {
			if(pbcFlags[k]) {
				while(r[k] > FloatType( 0.5)) r[k] -= FloatType(1);
				while(r[k] < FloatType(-0.5)) r[k] += FloatType(1);
				while(edgeVectors[0][k] > FloatType( 0.5)) edgeVectors[0][k] -= FloatType(1);
				while(edgeVectors[0][k] < FloatType(-0.5)) edgeVectors[0][k] += FloatType(1);
				while(edgeVectors[1][k] > FloatType( 0.5)) edgeVectors[1][k] -= FloatType(1);
				while(edgeVectors[1][k] < FloatType(-0.5)) edgeVectors[1][k] += FloatType(1);
			}
		}
		edgeVectors[2] = -edgeVectors[1] - edgeVectors[0];

		Vector3 normal = edgeVectors[0].cross(edgeVectors[1]);
		bool isInsideTriangle = true;
		Vector3 vertexVector = r;
		for(size_t v = 0; v < 3; v++) {
			if(vertexVector.dot(normal.cross(edgeVectors[v])) >= 0.0) {
				isInsideTriangle = false;
				break;
			}
			vertexVector += edgeVectors[v];
		}
		if(isInsideTriangle) {
			FloatType normalLengthSq = normal.squaredLength();
			if(std::abs(normalLengthSq) <= FLOATTYPE_EPSILON) continue;
			normal /= sqrt(normalLengthSq);
			FloatType planeDist = normal.dot(r);
			if(planeDist * planeDist < closestDistanceSq) {
				closestDistanceSq = planeDist * planeDist;
				closestVector = normal * planeDist;
				closestVertex = nullptr;
				closestNormal = normal;
			}
		}
	}

	// If a vertex is closest, we still have to compute the local pseudo-normal at the vertex.
	if(closestVertex != nullptr) {
		HalfEdgeMesh::Edge* edge = closestVertex->edges();
		closestNormal.setZero();
		Vector3 edge1v = reducedPos[edge->vertex2()->index()] - reducedPos[closestVertex->index()];
		for(size_t k = 0; k < 3; k++) {
			if(pbcFlags[k]) {
				while(edge1v[k] > FloatType( 0.5)) edge1v[k] -= FloatType(1);
				while(edge1v[k] < FloatType(-0.5)) edge1v[k] += FloatType(1);
			}
		}
		edge1v.normalizeSafely();
		do {
			HalfEdgeMesh::Edge* nextEdge = edge->oppositeEdge()->nextFaceEdge();
			OVITO_ASSERT(nextEdge->vertex1() == closestVertex);
			Vector3 edge2v = reducedPos[nextEdge->vertex2()->index()] - reducedPos[closestVertex->index()];
			for(size_t k = 0; k < 3; k++) {
				if(pbcFlags[k]) {
					while(edge2v[k] > FloatType( 0.5)) edge2v[k] -= FloatType(1);
					while(edge2v[k] < FloatType(-0.5)) edge2v[k] += FloatType(1);
				}
			}
			edge2v.normalizeSafely();
			FloatType angle = acos(edge1v.dot(edge2v));
			Vector3 normal = edge2v.cross(edge1v);
			if(normal != Vector3::Zero())
				closestNormal += normal.normalized() * angle;
			edge = nextEdge;
			edge1v = edge2v;
		}
		while(edge != closestVertex->edges());
	}

	return closestNormal.dot(closestVector) > 0;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void DefectSurfaceDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Surface display"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	ColorParameterUI* surfaceColorUI = new ColorParameterUI(this, PROPERTY_FIELD(DefectSurfaceDisplay::_surfaceColor));
	layout->addWidget(surfaceColorUI->label(), 0, 0);
	layout->addWidget(surfaceColorUI->colorPicker(), 0, 1);

	BooleanParameterUI* smoothShadingUI = new BooleanParameterUI(this, PROPERTY_FIELD(DefectSurfaceDisplay::_smoothShading));
	layout->addWidget(smoothShadingUI->checkBox(), 1, 0, 1, 2);

	ColorParameterUI* capColorUI = new ColorParameterUI(this, PROPERTY_FIELD(DefectSurfaceDisplay::_capColor));
	layout->addWidget(capColorUI->label(), 2, 0);
	layout->addWidget(capColorUI->colorPicker(), 2, 1);

	BooleanParameterUI* showCapUI = new BooleanParameterUI(this, PROPERTY_FIELD(DefectSurfaceDisplay::_showCap));
	layout->addWidget(showCapUI->checkBox(), 3, 0, 1, 2);
}

};
