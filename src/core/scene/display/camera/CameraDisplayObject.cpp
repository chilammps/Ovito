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
	bool recreateBuffer = !_buffer || !_buffer->isValid(renderer);

	// Determine icon color depending on selection status.
	Color color = ViewportSettings::getSettings().viewportColor(contextNode->isSelected() ? ViewportSettings::COLOR_SELECTION : ViewportSettings::COLOR_CAMERAS);

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			sceneObject, sceneObject ? sceneObject->revisionNumber() : 0,
			color) || recreateBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_buffer = renderer->createLineGeometryBuffer();

	// Update buffer contents.
	if(updateContents) {

		// Initialize vertices.
		static const Point3 vertices[16] = {
			{-0.15f, -0.15f, 0.3f},
			{ 0.15f, -0.15f, 0.3f},
			{ 0.15f,  0.15f, 0.3f},
			{-0.15f,  0.15f, 0.3f},
			{-0.15f, -0.15f, -0.2f},
			{ 0.15f, -0.15f, -0.2f},
			{ 0.15f,  0.15f, -0.2f},
			{-0.15f,  0.15f, -0.2f},
			{-0.02f, -0.02f, -0.2f},
			{ 0.02f, -0.02f, -0.2f},
			{ 0.02f,  0.02f, -0.2f},
			{-0.02f,  0.02f, -0.2f},
			{-0.10f, -0.10f, -0.5f},
			{ 0.10f, -0.10f, -0.5f},
			{ 0.10f,  0.10f, -0.5f},
			{-0.10f,  0.10f, -0.5f}
		};

		// Initialize lines.
		static const Point3 linePoints[] = {
				vertices[0], vertices[1],
				vertices[1], vertices[2],
				vertices[2], vertices[3],
				vertices[3], vertices[0],
				vertices[0], vertices[4],
				vertices[1], vertices[5],
				vertices[2], vertices[6],
				vertices[3], vertices[7],
				vertices[4], vertices[5],
				vertices[5], vertices[6],
				vertices[6], vertices[7],
				vertices[7], vertices[4],

				vertices[8], vertices[9],
				vertices[9], vertices[10],
				vertices[10], vertices[11],
				vertices[11], vertices[8],
				vertices[8], vertices[12],
				vertices[9], vertices[13],
				vertices[10], vertices[14],
				vertices[11], vertices[15],
				vertices[12], vertices[13],
				vertices[13], vertices[14],
				vertices[14], vertices[15],
				vertices[15], vertices[12]
		};

		_buffer->setSize(48);
		_buffer->setVertexPositions(linePoints);
		_buffer->setVertexColor(ColorA(color));
	}

	// Setup transformation matrix.
	Point3 cameraPos = Point3::Origin() + renderer->worldTransform().translation();
	FloatType scaling = 2.0f * renderer->viewport()->nonScalingSize(cameraPos);
	renderer->setWorldTransform(renderer->worldTransform() * AffineTransformation::scaling(scaling));

	// Handle picking.
	renderer->beginPickObject(contextNode, sceneObject, this);
	_buffer->render(renderer);
	renderer->endPickObject();
}

};
