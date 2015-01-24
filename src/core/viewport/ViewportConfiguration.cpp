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
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/SceneRoot.h>
#include <core/animation/AnimationSettings.h>
#include <core/rendering/viewport/ViewportSceneRenderer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ViewportConfiguration, RefTarget);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ViewportConfiguration, _viewports, "Viewports", Viewport, PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_ALWAYS_CLONE);
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _activeViewport, "ActiveViewport", Viewport, PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _maximizedViewport, "MaximizedViewport", Viewport, PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _orbitCenterMode, "OrbitCenterMode", PROPERTY_FIELD_NO_UNDO);
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _userOrbitCenter, "UserOrbitCenter", PROPERTY_FIELD_NO_UNDO);

/******************************************************************************
* Constructor.
******************************************************************************/
ViewportConfiguration::ViewportConfiguration(DataSet* dataset) : RefTarget(dataset),
	_orbitCenterMode(ORBIT_SELECTION_CENTER), _userOrbitCenter(Point3::Origin()), _viewportSuspendCount(0)
{
	INIT_PROPERTY_FIELD(ViewportConfiguration::_viewports);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_activeViewport);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_maximizedViewport);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_orbitCenterMode);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_userOrbitCenter);

	// Repaint viewports when the camera orbit center changed.
	connect(this, &ViewportConfiguration::cameraOrbitCenterChanged, this, &ViewportConfiguration::updateViewports);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void ViewportConfiguration::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(ViewportConfiguration::_activeViewport)) {
		Q_EMIT activeViewportChanged(_activeViewport);
	}
	else if(field == PROPERTY_FIELD(ViewportConfiguration::_maximizedViewport)) {
		Q_EMIT maximizedViewportChanged(_maximizedViewport);
	}
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void ViewportConfiguration::propertyChanged(const PropertyFieldDescriptor& field)
{
	if(field == PROPERTY_FIELD(ViewportConfiguration::_orbitCenterMode) || field == PROPERTY_FIELD(ViewportConfiguration::_userOrbitCenter)) {
		Q_EMIT cameraOrbitCenterChanged();
	}
	RefTarget::propertyChanged(field);
}

/******************************************************************************
* This flags all viewports for redrawing.
******************************************************************************/
void ViewportConfiguration::updateViewports()
{
	// Check if viewport updates are suppressed.
	if(_viewportSuspendCount > 0) {
		_viewportsNeedUpdate = true;
		return;
	}
	_viewportsNeedUpdate = false;

	for(Viewport* vp : viewports())
		vp->updateViewport();
}

/******************************************************************************
* This immediately redraws the viewports reflecting all
* changes made to the scene.
******************************************************************************/
void ViewportConfiguration::processViewportUpdates()
{
	if(isSuspended())
		return;

	for(Viewport* vp : viewports())
		vp->processUpdateRequest();
}

/******************************************************************************
* Return true if there is currently a rendering operation going on.
* No new windows or dialogs should be shown during this phase
* to prevent an infinite update loop.
******************************************************************************/
bool ViewportConfiguration::isRendering() const
{
	// Check if any of the viewport windows is rendering.
	for(Viewport* vp : viewports())
		if(vp->isRendering()) return true;

	return false;
}

/******************************************************************************
* This will resume redrawing of the viewports after a call to suspendViewportUpdates().
******************************************************************************/
void ViewportConfiguration::resumeViewportUpdates()
{
	OVITO_ASSERT(_viewportSuspendCount > 0);
	_viewportSuspendCount--;
	if(_viewportSuspendCount == 0 && _viewportsNeedUpdate)
		updateViewports();
}

/******************************************************************************
* Returns the renderer to be used for rendering the interactive viewports.
******************************************************************************/
ViewportSceneRenderer* ViewportConfiguration::viewportRenderer()
{
	if(!_viewportRenderer)
		_viewportRenderer = new ViewportSceneRenderer(dataset());
	return _viewportRenderer;
}

/******************************************************************************
* Returns the world space point around which the viewport camera orbits.
******************************************************************************/
Point3 ViewportConfiguration::orbitCenter()
{
	// Update orbiting center.
	if(orbitCenterMode() == ORBIT_SELECTION_CENTER) {
		Box3 selectionBoundingBox;
		for(SceneNode* node : dataset()->selection()->nodes()) {
			selectionBoundingBox.addBox(node->worldBoundingBox(dataset()->animationSettings()->time()));
		}
		if(!selectionBoundingBox.isEmpty())
			return selectionBoundingBox.center();
		else {
			Box3 sceneBoundingBox = dataset()->sceneRoot()->worldBoundingBox(dataset()->animationSettings()->time());
			if(!sceneBoundingBox.isEmpty())
				return sceneBoundingBox.center();
		}
	}
	else if(orbitCenterMode() == ORBIT_USER_DEFINED) {
		return _userOrbitCenter;
	}
	return Point3::Origin();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
