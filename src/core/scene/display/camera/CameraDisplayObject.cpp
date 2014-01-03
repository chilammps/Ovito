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
#include <core/scene/objects/geometry/TriMeshObject.h>
#include "CameraDisplayObject.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CameraDisplayObject, DisplayObject)

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 CameraDisplayObject::boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	// This is not a physical object. It doesn't have a size.
	return Box3(Point3::Origin(), Point3::Origin());
}

/******************************************************************************
* Computes the view-dependent bounding box of the object.
******************************************************************************/
Box3 CameraDisplayObject::viewDependentBoundingBox(TimePoint time, Viewport* viewport, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	TimeInterval iv;
	Point3 cameraPos = Point3::Origin() + contextNode->getWorldTransform(time, iv).translation();
	FloatType size = 1.0f * viewport->nonScalingSize(cameraPos);
	return Box3(Point3::Origin(), size);
}

/******************************************************************************
* Lets the display object render a scene object.
******************************************************************************/
void CameraDisplayObject::render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Camera objects are only visible in the viewports.
	if(renderer->isInteractive() == false || renderer->viewport() == nullptr)
		return;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_cameraIcon || !_cameraIcon->isValid(renderer)
			|| !_pickingCameraIcon || !_pickingCameraIcon->isValid(renderer);

	// Determine icon color depending on selection status.
	Color color = ViewportSettings::getSettings().viewportColor(contextNode->isSelected() ? ViewportSettings::COLOR_SELECTION : ViewportSettings::COLOR_CAMERAS);

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			sceneObject, sceneObject ? sceneObject->revisionNumber() : 0,
			color) || recreateBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer) {
		_cameraIcon = renderer->createLineGeometryBuffer();
		_pickingCameraIcon = renderer->createLineGeometryBuffer();
	}

	// Update buffer contents.
	if(updateContents) {

		// Initialize lines.
		static std::vector<Point3> linePoints;
		if(linePoints.empty()) {
			// Load and parse PLY file that contains the camera icon.
			QFile meshFile(QStringLiteral(":/core/3dicons/camera.ply"));
			meshFile.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream stream(&meshFile);
			for(int i = 0; i < 3; i++) stream.readLine();
			int numVertices = stream.readLine().section(' ', 2, 2).toInt();
			OVITO_ASSERT(numVertices > 0);
			for(int i = 0; i < 3; i++) stream.readLine();
			int numFaces = stream.readLine().section(' ', 2, 2).toInt();
			for(int i = 0; i < 2; i++) stream.readLine();
			std::vector<Point3> vertices(numVertices);
			for(int i = 0; i < numVertices; i++)
				stream >> vertices[i].x() >> vertices[i].y() >> vertices[i].z();
			for(int i = 0; i < numFaces; i++) {
				int numEdges, vindex, lastvindex, firstvindex;
				stream >> numEdges;
				for(int j = 0; j < numEdges; j++) {
					stream >> vindex;
					if(j != 0) {
						linePoints.push_back(vertices[lastvindex]);
						linePoints.push_back(vertices[vindex]);
					}
					else firstvindex = vindex;
					lastvindex = vindex;
				}
				linePoints.push_back(vertices[lastvindex]);
				linePoints.push_back(vertices[firstvindex]);
			}
		}

		_cameraIcon->setVertexCount(linePoints.size());
		_cameraIcon->setVertexPositions(linePoints.data());
		_cameraIcon->setLineColor(ColorA(color));

		_pickingCameraIcon->setVertexCount(linePoints.size(), renderer->defaultLinePickingWidth());
		_pickingCameraIcon->setVertexPositions(linePoints.data());
		_pickingCameraIcon->setLineColor(ColorA(color));
	}

	// Setup transformation matrix to always show the camera at the same size.
	Point3 cameraPos = Point3::Origin() + renderer->worldTransform().translation();
	FloatType scaling = 0.3f * renderer->viewport()->nonScalingSize(cameraPos);
	renderer->setWorldTransform(renderer->worldTransform() * AffineTransformation::scaling(scaling));

	renderer->beginPickObject(contextNode, sceneObject, this);
	if(!renderer->isPicking())
		_cameraIcon->render(renderer);
	else
		_pickingCameraIcon->render(renderer);
	renderer->endPickObject();
}

};
