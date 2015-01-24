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
#include <core/scene/ObjectNode.h>
#include <core/rendering/SceneRenderer.h>
#include "TargetObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene) OVITO_BEGIN_INLINE_NAMESPACE(StdObj)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, TargetObject, DataObject);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, TargetDisplayObject, DisplayObject);

/******************************************************************************
* Constructs a target object.
******************************************************************************/
TargetObject::TargetObject(DataSet* dataset) : DataObject(dataset)
{
	addDisplayObject(new TargetDisplayObject(dataset));
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 TargetDisplayObject::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	// This is not a physical object. It doesn't have a size.
	return Box3(Point3::Origin(), Point3::Origin());
}

/******************************************************************************
* Computes the view-dependent bounding box of the object.
******************************************************************************/
Box3 TargetDisplayObject::viewDependentBoundingBox(TimePoint time, Viewport* viewport, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	TimeInterval iv;
	Point3 objectPos = Point3::Origin() + contextNode->getWorldTransform(time, iv).translation();
	FloatType size = 0.2f * viewport->nonScalingSize(objectPos);
	return Box3(Point3::Origin(), size);
}

/******************************************************************************
* Lets the display object render a data object.
******************************************************************************/
void TargetDisplayObject::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Target objects are only visible in the viewports.
	if(renderer->isInteractive() == false || renderer->viewport() == nullptr)
		return;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_icon || !_icon->isValid(renderer)
			|| !_pickingIcon || !_pickingIcon->isValid(renderer);

	// Determine icon color depending on selection state.
	Color color = ViewportSettings::getSettings().viewportColor(contextNode->isSelected() ? ViewportSettings::COLOR_SELECTION : ViewportSettings::COLOR_CAMERAS);

	// Do we have to update contents of the geometry buffers?
	bool updateContents = _geometryCacheHelper.updateState(dataObject, color) || recreateBuffer;

	// Re-create the geometry buffers if necessary.
	if(recreateBuffer) {
		_icon = renderer->createLinePrimitive();
		_pickingIcon = renderer->createLinePrimitive();
	}

	// Update geometry buffers.
	if(updateContents) {

		// Initialize lines.
		static const Point3 linePoints[] = {
				{-1, -1, -1}, { 1,-1,-1},
				{-1, -1,  1}, { 1,-1, 1},
				{-1, -1, -1}, {-1,-1, 1},
				{ 1, -1, -1}, { 1,-1, 1},
				{-1,  1, -1}, { 1, 1,-1},
				{-1,  1,  1}, { 1, 1, 1},
				{-1,  1, -1}, {-1, 1, 1},
				{ 1,  1, -1}, { 1, 1, 1},
				{-1, -1, -1}, {-1, 1,-1},
				{ 1, -1, -1}, { 1, 1,-1},
				{ 1, -1,  1}, { 1, 1, 1},
				{-1, -1,  1}, {-1, 1, 1}
		};

		_icon->setVertexCount(24);
		_icon->setVertexPositions(linePoints);
		_icon->setLineColor(ColorA(color));

		_pickingIcon->setVertexCount(24, renderer->defaultLinePickingWidth());
		_pickingIcon->setVertexPositions(linePoints);
		_pickingIcon->setLineColor(ColorA(color));
	}

	// Setup transformation matrix to always show the icon at the same size.
	Point3 objectPos = Point3::Origin() + renderer->worldTransform().translation();
	FloatType scaling = 0.2f * renderer->viewport()->nonScalingSize(objectPos);
	renderer->setWorldTransform(renderer->worldTransform() * AffineTransformation::scaling(scaling));

	renderer->beginPickObject(contextNode);
	if(!renderer->isPicking())
		_icon->render(renderer);
	else
		_pickingIcon->render(renderer);
	renderer->endPickObject();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
