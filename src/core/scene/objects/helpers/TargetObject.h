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

#ifndef __OVITO_TARGET_OBJECT_H
#define __OVITO_TARGET_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/display/DisplayObject.h>
#include <core/rendering/LineGeometryBuffer.h>

namespace Ovito {

/**
 * A simple helper object that serves as direction target for camera and light objects.
 */
class OVITO_CORE_EXPORT TargetObject : public SceneObject
{
public:

	/// Constructor.
	Q_INVOKABLE TargetObject(DataSet* dataset);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Target"); }

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A scene display object for target scene objects.
 */
class OVITO_CORE_EXPORT TargetDisplayObject : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE TargetDisplayObject(DataSet* dataset) : DisplayObject(dataset) {}

	/// \brief Lets the display object render a scene object.
	virtual void render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Computes the view-dependent bounding box of the scene object for interactive rendering in the viewports.
	virtual Box3 viewDependentBoundingBox(TimePoint time, Viewport* viewport, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Target icon"); }

protected:

	/// The buffered geometry used to render the icon.
	std::unique_ptr<LineGeometryBuffer> _icon;

	/// The icon geometry to be rendered in object picking mode.
	std::unique_ptr<LineGeometryBuffer> _pickingIcon;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		QPointer<SceneObject>, unsigned int,		// Scene object + revision number
		Color										// Display color
		> _geometryCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_TARGET_OBJECT_H
