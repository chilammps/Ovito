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
#include <core/scene/objects/DataObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/rendering/LinePrimitive.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene) OVITO_BEGIN_INLINE_NAMESPACE(StdObj)

/**
 * A simple helper object that serves as direction target for camera and light objects.
 */
class OVITO_CORE_EXPORT TargetObject : public DataObject
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
 * \brief A scene display object for target objects.
 */
class OVITO_CORE_EXPORT TargetDisplayObject : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE TargetDisplayObject(DataSet* dataset) : DisplayObject(dataset) {}

	/// \brief Lets the display object render a data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Computes the view-dependent bounding box of the data object for interactive rendering in the viewports.
	virtual Box3 viewDependentBoundingBox(TimePoint time, Viewport* viewport, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Target icon"); }

protected:

	/// The buffered geometry used to render the icon.
	std::shared_ptr<LinePrimitive> _icon;

	/// The icon geometry to be rendered in object picking mode.
	std::shared_ptr<LinePrimitive> _pickingIcon;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<DataObject>,		// Scene object + revision number
		Color									// Display color
		> _geometryCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_TARGET_OBJECT_H
