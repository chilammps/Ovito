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

#include <plugins/particles/Particles.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/utilities/mesh/TriMesh.h>
#include <core/animation/controller/Controller.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include <plugins/particles/util/CapPolygonTessellator.h>
#include "SurfaceMeshDisplay.h"
#include "SurfaceMesh.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, SurfaceMeshDisplayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, SurfaceMeshDisplay, DisplayObject);
SET_OVITO_OBJECT_EDITOR(SurfaceMeshDisplay, SurfaceMeshDisplayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(SurfaceMeshDisplay, _surfaceColor, "SurfaceColor", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SurfaceMeshDisplay, _capColor, "CapColor", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SurfaceMeshDisplay, _showCap, "ShowCap", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(SurfaceMeshDisplay, _smoothShading, "SmoothShading");
DEFINE_REFERENCE_FIELD(SurfaceMeshDisplay, _surfaceTransparency, "SurfaceTransparency", Controller);
DEFINE_REFERENCE_FIELD(SurfaceMeshDisplay, _capTransparency, "CapTransparency", Controller);
SET_PROPERTY_FIELD_LABEL(SurfaceMeshDisplay, _surfaceColor, "Surface color");
SET_PROPERTY_FIELD_LABEL(SurfaceMeshDisplay, _capColor, "Cap color");
SET_PROPERTY_FIELD_LABEL(SurfaceMeshDisplay, _showCap, "Show cap polygons");
SET_PROPERTY_FIELD_LABEL(SurfaceMeshDisplay, _smoothShading, "Smooth shading");
SET_PROPERTY_FIELD_LABEL(SurfaceMeshDisplay, _surfaceTransparency, "Surface transparency");
SET_PROPERTY_FIELD_LABEL(SurfaceMeshDisplay, _capTransparency, "Cap transparency");
SET_PROPERTY_FIELD_UNITS(SurfaceMeshDisplay, _surfaceTransparency, PercentParameterUnit);
SET_PROPERTY_FIELD_UNITS(SurfaceMeshDisplay, _capTransparency, PercentParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
SurfaceMeshDisplay::SurfaceMeshDisplay(DataSet* dataset) : DisplayObject(dataset),
	_surfaceColor(1, 1, 1), _capColor(0.8, 0.8, 1.0), _showCap(true), _smoothShading(true)
{
	INIT_PROPERTY_FIELD(SurfaceMeshDisplay::_surfaceColor);
	INIT_PROPERTY_FIELD(SurfaceMeshDisplay::_capColor);
	INIT_PROPERTY_FIELD(SurfaceMeshDisplay::_showCap);
	INIT_PROPERTY_FIELD(SurfaceMeshDisplay::_smoothShading);
	INIT_PROPERTY_FIELD(SurfaceMeshDisplay::_surfaceTransparency);
	INIT_PROPERTY_FIELD(SurfaceMeshDisplay::_capTransparency);

	_surfaceTransparency = ControllerManager::instance().createFloatController(dataset);
	_capTransparency = ControllerManager::instance().createFloatController(dataset);
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 SurfaceMeshDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	SimulationCellObject* cellObject = flowState.findObject<SimulationCellObject>();
	if(!cellObject)
		return Box3();

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(dataObject, cellObject->data()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = Box3(Point3(0,0,0), Point3(1,1,1)).transformed(cellObject->cellMatrix());
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Lets the display object render the data object.
******************************************************************************/
void SurfaceMeshDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Get the simulation cell.
	SimulationCellObject* cellObject = flowState.findObject<SimulationCellObject>();
	if(!cellObject)
		return;

	// Do we have to re-create the geometry buffers from scratch?
	bool recreateSurfaceBuffer = !_surfaceBuffer || !_surfaceBuffer->isValid(renderer);
	bool recreateCapBuffer = _showCap && (!_capBuffer || !_capBuffer->isValid(renderer));

	// Get the rendering colors for the surface and cap meshes.
	FloatType transp_surface = 0;
	FloatType transp_cap = 0;
	TimeInterval iv;
	if(_surfaceTransparency) transp_surface = _surfaceTransparency->getFloatValue(time, iv);
	if(_capTransparency) transp_cap = _capTransparency->getFloatValue(time, iv);
	ColorA color_surface(surfaceColor(), 1.0f - transp_surface);
	ColorA color_cap(capColor(), 1.0f - transp_cap);

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			dataObject,
			cellObject->data(), color_surface, color_cap, _smoothShading)
					|| recreateSurfaceBuffer || recreateCapBuffer;

	// Re-create the geometry buffers if necessary.
	if(recreateSurfaceBuffer)
		_surfaceBuffer = renderer->createMeshPrimitive();
	if(recreateCapBuffer && _showCap)
		_capBuffer = renderer->createMeshPrimitive();

	// Update buffer contents.
	if(updateContents) {
		OORef<SurfaceMesh> defectSurfaceObj = dataObject->convertTo<SurfaceMesh>(time);
		if(defectSurfaceObj) {
			TriMesh surfaceMesh;
			TriMesh capMesh;
			if(buildSurfaceMesh(defectSurfaceObj->mesh(), cellObject->data(), surfaceMesh)) {
				// Assign smoothing group to faces to interpolate normals.
				if(_smoothShading) {
					for(auto& face : surfaceMesh.faces())
						face.setSmoothingGroups(1);
				}
				_surfaceBuffer->setMesh(surfaceMesh, color_surface);
				if(_showCap) {
					buildCapMesh(defectSurfaceObj->mesh(), cellObject->data(), defectSurfaceObj->isCompletelySolid(), capMesh);
					_capBuffer->setMesh(capMesh, color_cap);
				}
			}
			else {
				// Render empty meshes if they could not be generated.
				_surfaceBuffer->setMesh(TriMesh(), color_surface);
				if(_showCap) _capBuffer->setMesh(TriMesh(), color_cap);
			}
		}
		else {
			_surfaceBuffer->setMesh(TriMesh(), ColorA(1,1,1,1));
			if(_showCap) _capBuffer->setMesh(TriMesh(), ColorA(1,1,1,1));
		}
	}

	// Handle picking of triangles.
	renderer->beginPickObject(contextNode);
	_surfaceBuffer->render(renderer);
	if(_showCap)
		_capBuffer->render(renderer);
	else
		_capBuffer.reset();
	renderer->endPickObject();
}

/******************************************************************************
* Generates the final triangle mesh, which will be rendered.
******************************************************************************/
bool SurfaceMeshDisplay::buildSurfaceMesh(const HalfEdgeMesh& input, const SimulationCell& cell, TriMesh& output)
{
	// Convert half-edge mesh to triangle mesh.
	input.convertToTriMesh(output);

	// Convert vertex positions to reduced coordinates.
	for(Point3& p : output.vertices())
		p = cell.absoluteToReduced(p);

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
			if(!splitFace(output, output.face(findex), oldVertexCount, newVertices, newVertexLookupMap, cell, dim))
				return false;
		}

		// Insert newly created vertices into mesh.
		output.setVertexCount(oldVertexCount + (int)newVertices.size());
		std::copy(newVertices.cbegin(), newVertices.cend(), output.vertices().begin() + oldVertexCount);
	}

	// Convert vertex positions back from reduced coordinates to absolute coordinates.
	AffineTransformation cellMatrix = cell.matrix();
	for(Point3& p : output.vertices())
		p = cellMatrix * p;

	output.invalidateVertices();
	output.invalidateFaces();

	return true;
}

/******************************************************************************
* Splits a triangle face at a periodic boundary.
******************************************************************************/
bool SurfaceMeshDisplay::splitFace(TriMesh& output, TriMeshFace& face, int oldVertexCount, std::vector<Point3>& newVertices,
		std::map<std::pair<int,int>,std::pair<int,int>>& newVertexLookupMap, const SimulationCell& cell, size_t dim)
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
		return true;	// Face is not crossing the periodic boundary.

	// Create four new vertices (or use existing ones created during splitting of adjacent faces).
	int properEdge = -1;
	int newVertexIndices[3][2];
	for(int i = 0; i < 3; i++) {
		if(std::abs(zd[i]) < 0.5f) {
			if(properEdge != -1)
				return false;		// The simulation box may be too small or invalid.
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
			newVertexIndices[i][oi1] = oldVertexCount + (int)newVertices.size();
			newVertexIndices[i][oi2] = oldVertexCount + (int)newVertices.size() + 1;
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

	return true;
}

/******************************************************************************
* Generates the triangle mesh for the PBC caps.
******************************************************************************/
void SurfaceMeshDisplay::buildCapMesh(const HalfEdgeMesh& input, const SimulationCell& cell, bool isCompletelySolid, TriMesh& output)
{
	// Convert vertex positions to reduced coordinates.
	std::vector<Point3> reducedPos(input.vertexCount());
	auto inputVertex = input.vertices().begin();
	for(Point3& p : reducedPos)
		p = cell.absoluteToReduced((*inputVertex++)->pos());

	int isBoxCornerInside3DRegion = -1;

	// Create caps for each periodic boundary.
	for(size_t dim = 0; dim < 3; dim++) {
		if(cell.pbcFlags()[dim] == false) continue;

		// Make sure all vertices are located inside the periodic box.
		for(Point3& p : reducedPos) {
			FloatType& c = p[dim];
			OVITO_ASSERT(std::isfinite(c));
			if(FloatType s = floor(c)) c -= s;
			OVITO_ASSERT(std::isfinite(c));
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
					clipContour(contour, std::array<bool,2>{{ cell.pbcFlags()[(dim+1)%3], cell.pbcFlags()[(dim+2)%3] }}, openContours, closedContours);
				}
			}
		}

		// Feed contours into tessellator to create triangles.
		CapPolygonTessellator tessellator(output, dim);
		tessellator.beginPolygon();
		for(const auto& contour : closedContours) {
			tessellator.beginContour();
			for(const Point2& p : contour)
				tessellator.vertex(p);
			tessellator.endContour();
		}

		// Build the outer contour.
		if(!openContours.empty()) {
			QBitArray visitedContours(openContours.size());
			for(auto c1 = openContours.begin(); c1 != openContours.end(); ++c1) {
				if(!visitedContours.testBit(c1 - openContours.begin())) {
					tessellator.beginContour();
					auto currentContour = c1;
					do {
						for(const Point2& p : *currentContour)
							tessellator.vertex(p);
						visitedContours.setBit(currentContour - openContours.begin());

						FloatType exitSide = 0;
						if(currentContour->back().x() == 0) exitSide = currentContour->back().y();
						else if(currentContour->back().y() == 1) exitSide = currentContour->back().x() + 1.0f;
						else if(currentContour->back().x() == 1) exitSide = 3.0f - currentContour->back().y();
						else if(currentContour->back().y() == 0) exitSide = 4.0f - currentContour->back().x();

						// Find the next contour.
						FloatType entrySide;
						FloatType closestDist = FLOATTYPE_MAX;
						for(auto c = openContours.begin(); c != openContours.end(); ++c) {
							FloatType pos = 0;
							if(c->front().x() == 0) pos = c->front().y();
							else if(c->front().y() == 1) pos = c->front().x() + 1.0f;
							else if(c->front().x() == 1) pos = 3.0f - c->front().y();
							else if(c->front().y() == 0) pos = 4.0f - c->front().x();
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
								case 0: tessellator.vertex(Point2(0,0)); break;
								case 1: tessellator.vertex(Point2(0,1)); break;
								case 2: tessellator.vertex(Point2(1,1)); break;
								case 3: tessellator.vertex(Point2(1,0)); break;
								}
								corner = (corner + 3) % 4;
								if(corner == entryCorner) break;
							}
						}
					}
					while(!visitedContours.testBit(currentContour - openContours.begin()));
					tessellator.endContour();
				}
			}
		}
		else {
			if(isBoxCornerInside3DRegion == -1) {
				if(closedContours.empty())
					isBoxCornerInside3DRegion = isCornerInside3DRegion(input, reducedPos, cell.pbcFlags(), isCompletelySolid);
				else
					isBoxCornerInside3DRegion = isCornerInside2DRegion(closedContours);
			}
			if(isBoxCornerInside3DRegion) {
				tessellator.beginContour();
				tessellator.vertex(Point2(0,0));
				tessellator.vertex(Point2(1,0));
				tessellator.vertex(Point2(1,1));
				tessellator.vertex(Point2(0,1));
				tessellator.endContour();
			}
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
std::vector<Point2> SurfaceMeshDisplay::traceContour(HalfEdgeMesh::Edge* firstEdge, const std::vector<Point3>& reducedPos, const SimulationCell& cell, size_t dim)
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
			if(FloatType s = floor(c + FloatType(0.5)))
				c -= s;
		}
		if(cell.pbcFlags()[dim2]) {
			FloatType& c = delta[dim2];
			if(FloatType s = floor(c + FloatType(0.5)))
				c -= s;
		}
		FloatType t;
		if(std::abs(delta[dim]) > FloatType(1e-9f))
			t = v1[dim] / delta[dim];
		else
			t = FloatType(0.5);
		FloatType x = v1[dim1] - delta[dim1] * t;
		FloatType y = v1[dim2] - delta[dim2] * t;
		OVITO_ASSERT(std::isfinite(x) && std::isfinite(y));
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
void SurfaceMeshDisplay::clipContour(std::vector<Point2>& input, std::array<bool,2> pbcFlags, std::vector<std::vector<Point2>>& openContours, std::vector<std::vector<Point2>>& closedContours)
{
	if(!pbcFlags[0] && !pbcFlags[1]) {
		closedContours.push_back(std::move(input));
		return;
	}

	// Ensure all coordinates are within the primary image.
	if(pbcFlags[0]) {
		for(auto& v : input) {
			OVITO_ASSERT(std::isfinite(v.x()));
			if(FloatType s = floor(v.x())) v.x() -= s;
		}
	}
	if(pbcFlags[1]) {
		for(auto& v : input) {
			OVITO_ASSERT(std::isfinite(v.y()));
			if(FloatType s = floor(v.y())) v.y() -= s;
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
					if(std::abs(delta[dim]) > FLOATTYPE_EPSILON)
						t[dim] = (*v1)[dim] / -delta[dim];
					else
						t[dim] = 0.5f;
					crossDir[dim] = -1;
				}
				else if(delta[dim] <= -0.5f) {
					delta[dim] += 1.0f;
					if(std::abs(delta[dim]) > FLOATTYPE_EPSILON)
						t[dim] = (1.0f - (*v1)[dim]) / delta[dim];
					else
						t[dim] = 0.5f;
					crossDir[dim] = +1;
				}
				OVITO_ASSERT(t[dim] >= 0 && t[dim] <= 1);
			}
		}

		Point2 base = *v1;
		if(t[0] < t[1]) {
			computeContourIntersection(0, t[0], base, delta, crossDir[0], contours);
			if(crossDir[1] != 0)
				computeContourIntersection(1, t[1], base, delta, crossDir[1], contours);
		}
		else if(t[1] < t[0]) {
			computeContourIntersection(1, t[1], base, delta, crossDir[1], contours);
			if(crossDir[0] != 0)
				computeContourIntersection(0, t[0], base, delta, crossDir[0], contours);
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

/******************************************************************************
* Computes the intersection point of a 2d contour segment crossing a
* periodic boundary.
******************************************************************************/
void SurfaceMeshDisplay::computeContourIntersection(size_t dim, FloatType t, Point2& base, Vector2& delta, int crossDir, std::vector<std::vector<Point2>>& contours)
{
	OVITO_ASSERT(std::isfinite(t));
	Point2 intersection = base + t * delta;
	intersection[dim] = (crossDir == -1) ? 0.0f : 1.0f;
	contours.back().push_back(intersection);
	intersection[dim] = (crossDir == +1) ? 0.0f : 1.0f;
	contours.push_back({intersection});
	base = intersection;
	delta *= (1.0f - t);
}

/******************************************************************************
* Determines if the 2D box corner (0,0) is inside the closed region described
* by the 2d polygon.
*
* 2D version of the algorithm:
*
* J. Andreas Baerentzen and Henrik Aanaes
* Signed Distance Computation Using the Angle Weighted Pseudonormal
* IEEE Transactions on Visualization and Computer Graphics 11 (2005), Page 243
******************************************************************************/
bool SurfaceMeshDisplay::isCornerInside2DRegion(const std::vector<std::vector<Point2>>& contours)
{
	OVITO_ASSERT(!contours.empty());
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

/******************************************************************************
* Determines if the 3D box corner (0,0,0) is inside the region described by
* the half-edge polyhedron.
*
* Algorithm:
*
* J. Andreas Baerentzen and Henrik Aanaes
* Signed Distance Computation Using the Angle Weighted Pseudonormal
* IEEE Transactions on Visualization and Computer Graphics 11 (2005), Page 243
******************************************************************************/
bool SurfaceMeshDisplay::isCornerInside3DRegion(const HalfEdgeMesh& mesh, const std::vector<Point3>& reducedPos, const std::array<bool,3> pbcFlags, bool isCompletelySolid)
{
	if(mesh.vertices().empty())
		return isCompletelySolid;

	// Determine which vertex is closest to the test point.
	FloatType closestDistanceSq = FLOATTYPE_MAX;
	HalfEdgeMesh::Vertex* closestVertex = nullptr;
	Vector3 closestNormal, closestVector;
	for(HalfEdgeMesh::Vertex* v : mesh.vertices()) {
		Vector3 r = reducedPos[v->index()] - Point3::Origin();
		for(size_t k = 0; k < 3; k++) {
			if(pbcFlags[k]) {
				if(FloatType s = floor(r[k] + FloatType(0.5)))
					r[k] -= s;
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
					if(FloatType s = floor(r[k] + FloatType(0.5)))
						r[k] -= s;
					if(FloatType s = floor(edgeDir[k] + FloatType(0.5)))
						edgeDir[k] -= s;
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
						if(FloatType s = floor(e1[k] + FloatType(0.5)))
							e1[k] -= s;
						if(FloatType s = floor(e2[k] + FloatType(0.5)))
							e2[k] -= s;
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
				if(FloatType s = floor(r[k] + FloatType(0.5)))
					r[k] -= s;
				if(FloatType s = floor(edgeVectors[0][k] + FloatType(0.5)))
					edgeVectors[0][k] -= s;
				if(FloatType s = floor(edgeVectors[1][k] + FloatType(0.5)))
					edgeVectors[1][k] -= s;
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
				if(FloatType s = floor(edge1v[k] + FloatType(0.5)))
					edge1v[k] -= s;
			}
		}
		edge1v.normalizeSafely();
		do {
			HalfEdgeMesh::Edge* nextEdge = edge->oppositeEdge()->nextFaceEdge();
			OVITO_ASSERT(nextEdge->vertex1() == closestVertex);
			Vector3 edge2v = reducedPos[nextEdge->vertex2()->index()] - reducedPos[closestVertex->index()];
			for(size_t k = 0; k < 3; k++) {
				if(pbcFlags[k]) {
					if(FloatType s = floor(edge2v[k] + FloatType(0.5)))
						edge2v[k] -= s;
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

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SurfaceMeshDisplayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Surface display"), rolloutParams, "display_objects.surface_mesh.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGroupBox* surfaceGroupBox = new QGroupBox(tr("Surface"));
	QGridLayout* sublayout = new QGridLayout(surfaceGroupBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(1, 1);
	layout->addWidget(surfaceGroupBox);

	ColorParameterUI* surfaceColorUI = new ColorParameterUI(this, PROPERTY_FIELD(SurfaceMeshDisplay::_surfaceColor));
	sublayout->addWidget(surfaceColorUI->label(), 0, 0);
	sublayout->addWidget(surfaceColorUI->colorPicker(), 0, 1);

	FloatParameterUI* surfaceTransparencyUI = new FloatParameterUI(this, PROPERTY_FIELD(SurfaceMeshDisplay::_surfaceTransparency));
	sublayout->addWidget(new QLabel(tr("Transparency (%):")), 1, 0);
	sublayout->addLayout(surfaceTransparencyUI->createFieldLayout(), 1, 1);
	surfaceTransparencyUI->setMinValue(0);
	surfaceTransparencyUI->setMaxValue(1);

	BooleanParameterUI* smoothShadingUI = new BooleanParameterUI(this, PROPERTY_FIELD(SurfaceMeshDisplay::_smoothShading));
	sublayout->addWidget(smoothShadingUI->checkBox(), 2, 0, 1, 2);

	BooleanGroupBoxParameterUI* capGroupUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(SurfaceMeshDisplay::_showCap));
	capGroupUI->groupBox()->setTitle(tr("Cap polygons"));
	sublayout = new QGridLayout(capGroupUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(1, 1);
	layout->addWidget(capGroupUI->groupBox());

	ColorParameterUI* capColorUI = new ColorParameterUI(this, PROPERTY_FIELD(SurfaceMeshDisplay::_capColor));
	sublayout->addWidget(capColorUI->label(), 0, 0);
	sublayout->addWidget(capColorUI->colorPicker(), 0, 1);

	FloatParameterUI* capTransparencyUI = new FloatParameterUI(this, PROPERTY_FIELD(SurfaceMeshDisplay::_capTransparency));
	sublayout->addWidget(new QLabel(tr("Transparency (%):")), 1, 0);
	sublayout->addLayout(capTransparencyUI->createFieldLayout(), 1, 1);
	capTransparencyUI->setMinValue(0);
	capTransparencyUI->setMaxValue(1);
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
