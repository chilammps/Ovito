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
#include <core/viewport/ViewportManager.h>
#include <core/gui/app/Application.h>

/// The default field of view in world units used for orthogonal view types when the scene is empty.
#define DEFAULT_ORTHOGONAL_FIELD_OF_VIEW		200.0

/// The default field of view in radians used for perspective view types when the scene is empty.
#define DEFAULT_PERSPECTIVE_FIELD_OF_VIEW		(FLOATTYPE_PI/4.0)

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(ViewportConfiguration, RefTarget);
#if 0
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _viewNode, "ViewNode", ObjectNode, PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_NEVER_CLONE_TARGET)
#endif
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _viewType, "ViewType", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _shadingMode, "ShadingMode", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _showGrid, "ShowGrid", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _gridMatrix, "GridMatrix", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _fieldOfView, "FieldOfView", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _viewMatrix, "ViewMatrix", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _showRenderFrame, "ShowRenderFrame", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _orbitCenter, "OrbitCenter", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _useOrbitCenter, "UseOrbitCenter", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfiguration, _viewportTitle, "Title", PROPERTY_FIELD_NO_UNDO)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(ViewportConfigurationSet, RefTarget);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ViewportConfigurationSet, _viewRecords, "Records", ViewportConfiguration, PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_ALWAYS_CLONE)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfigurationSet, _activeViewport, "ActiveViewport", PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_PROPERTY_FIELD(ViewportConfigurationSet, _maximizedViewport, "MaximizedViewport", PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Default constructor.
******************************************************************************/
ViewportConfiguration::ViewportConfiguration(Viewport* owner) :
	_viewType(VIEW_NONE), _shadingMode(SHADING_WIREFRAME), _showGrid(false),
	_fieldOfView(100), _viewMatrix(AffineTransformation::Identity()), _showRenderFrame(false),
	_viewport(owner), _orbitCenter(Point3::Origin()), _useOrbitCenter(false)
{
#if 0
	INIT_PROPERTY_FIELD(ViewportConfiguration::_viewNode);
#endif
	INIT_PROPERTY_FIELD(ViewportConfiguration::_viewType);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_shadingMode);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_showGrid);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_gridMatrix);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_fieldOfView);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_viewMatrix);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_showRenderFrame);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_orbitCenter);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_useOrbitCenter);
	INIT_PROPERTY_FIELD(ViewportConfiguration::_viewportTitle);
}

/******************************************************************************
* Changes the view type.
******************************************************************************/
void ViewportConfiguration::setViewType(ViewType type)
{
	if(type == viewType())
		return;

#if 0
	// Reset camera node.
	setViewNode(NULL);
#endif

	// Setup default view matrix.
	switch(type) {
		case VIEW_TOP:
			setViewMatrix(AffineTransformation::Identity());
			setGridMatrix(AffineTransformation::Identity());
			break;
		case VIEW_BOTTOM:
			setViewMatrix(AffineTransformation(-1,0,0,0,  0,1,0,0,  0,0,-1,0));
			setGridMatrix(inverseViewMatrix());
			break;
		case VIEW_LEFT:
			setViewMatrix(AffineTransformation(0,-1,0,0,  0,0,1,0,  -1,0,0,0));
			setGridMatrix(inverseViewMatrix());
			break;
		case VIEW_RIGHT:
			setViewMatrix(AffineTransformation(0,1,0,0,  0,0,1,0,  1,0,0,0));
			setGridMatrix(inverseViewMatrix());
			break;
		case VIEW_FRONT:
			setViewMatrix(AffineTransformation(1,0,0,0,  0,0,1,0,  0,-1,0,0));
			setGridMatrix(inverseViewMatrix());
			break;
		case VIEW_BACK:
			setViewMatrix(AffineTransformation(-1,0,0,0,  0,0,1,0,  0,1,0,0));
			setGridMatrix(inverseViewMatrix());
			break;
		case VIEW_ORTHO:
			if(viewType() == VIEW_NONE)
				setViewMatrix(AffineTransformation::Identity());
			setGridMatrix(AffineTransformation::Identity());
			break;
		case VIEW_PERSPECTIVE:
			if(viewType() >= VIEW_TOP && viewType() <= VIEW_ORTHO) {
				setViewMatrix(AffineTransformation::translation(Vector3(0,0,-fieldOfView())) * viewMatrix());
			}
			else if(viewType() != VIEW_PERSPECTIVE)
				setViewMatrix(AffineTransformation::translation(Vector3(0,0,-50)));
			setGridMatrix(AffineTransformation::Identity());
			break;
		case VIEW_SCENENODE:
			setGridMatrix(AffineTransformation::Identity());
			break;
		case VIEW_NONE:
			break;
	}

	// Setup default zoom.
	if(type == VIEW_PERSPECTIVE) {
		if(viewType() != VIEW_PERSPECTIVE)
			setFieldOfView(DEFAULT_PERSPECTIVE_FIELD_OF_VIEW);
	}
	else {
		if(viewType() == VIEW_PERSPECTIVE || viewType() == VIEW_NONE)
			setFieldOfView(DEFAULT_ORTHOGONAL_FIELD_OF_VIEW);
	}

	_viewType = type;
}

#if 0

/******************************************************************************
* Returns a description the viewport's view at the given animation time.
******************************************************************************/
CameraViewDescription ViewportConfiguration::getViewDescription(TimeTicks time, FloatType aspectRatio, const Box3& bb)
{
	Box3 sceneBoundingBox = bb;
	if(sceneBoundingBox.isEmpty())
		sceneBoundingBox = DATASET_MANAGER.currentSet()->sceneRoot()->worldBoundingBox(time);

	CameraViewDescription d;
	d.validityInterval = TimeForever;
	d.aspectRatio = aspectRatio;

	// Get transformation from view scene node.
	if(viewType() == VIEW_SCENENODE && viewNode()) {
		PipelineFlowState state = viewNode()->evalPipeline(time);
		AbstractCameraObject* camera = dynamic_object_cast<AbstractCameraObject>(state.result());
		if(camera) {
			// Get camera transformation.
			d.inverseViewMatrix = viewNode()->getWorldTransform(time, d.validityInterval);
			d.viewMatrix = d.inverseViewMatrix.inverse();
			// Calculate znear and zfar clipping plane distances.
			Box3 bb = sceneBoundingBox.transformed(d.viewMatrix);
			d.zfar = -bb.minc.Z;
			d.znear = max(-bb.maxc.Z, -bb.minc.Z * (FloatType)1e-5);
			camera->getCameraDescription(time, d);
		}
	}
	else if(viewType() == VIEW_PERSPECTIVE) {
        // Get camera position.
		//Point3 camera = ORIGIN + inverseViewMatrix().getTranslation();
		// Enlarge bounding box to include camera.
		//Box3 bb = sceneBoundingBox;
		//bb.addPoint(camera);

		//d.zfar = Length(bb.size());
		//d.zfar = max(d.zfar, (FloatType)1e-5);
		//d.znear = d.zfar * 1e-5;

		Box3 bb = sceneBoundingBox.transformed(viewMatrix());
		if(bb.minc.Z < -1e-5) {
			d.zfar = -bb.minc.Z;
			d.znear = max(-bb.maxc.Z, -bb.minc.Z * (FloatType)1e-5);
		}
		else {
			d.zfar = 1000.0f;
			d.znear = 0.1f;
		}

		d.fieldOfView = fieldOfView();
		d.viewMatrix = viewMatrix();
		d.inverseViewMatrix = inverseViewMatrix();
		d.isPerspective = true;
		d.projectionMatrix = Matrix4::perspective(d.fieldOfView, 1.0/d.aspectRatio, d.znear, d.zfar);
		d.inverseProjectionMatrix = d.projectionMatrix.inverse();
	}
	else {
		// Transform scene to camera space.
		Box3 bb = sceneBoundingBox.transformed(viewMatrix());
		if(!bb.isEmpty()) {
			d.znear = -bb.maxc.Z;
			d.zfar  = max(-bb.minc.Z, d.znear + 1.0f);
		}
		else {
			d.znear = 1;
			d.zfar = 100;
		}
		d.viewMatrix = viewMatrix();
		d.inverseViewMatrix = inverseViewMatrix();
		d.isPerspective = false;
		d.fieldOfView = fieldOfView();
		d.projectionMatrix = Matrix4::ortho(-d.fieldOfView, d.fieldOfView,
							-d.fieldOfView*d.aspectRatio, d.fieldOfView*d.aspectRatio,
							d.znear, d.zfar);
		d.inverseProjectionMatrix = d.projectionMatrix.inverse();
	}
	return d;
}

#endif

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool ViewportConfiguration::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
#if 0
	if(source == _viewNode && event->type() == ReferenceEvent::TargetChanged) {
		// Update viewport when camera node has moved.
		if(_viewport)
			_viewport->updateViewport();
		return false;
	}
	else if(source == _viewNode && event->type() == ReferenceEvent::TitleChanged) {
		// Update viewport title when camera node has been renamed.
		if(_viewport) {
			updateViewportTitle();
			_viewport->updateViewport();
		}
		return false;
	}
#endif
	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void ViewportConfiguration::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
#if 0
	if(field == PROPERTY_FIELD(ViewportConfiguration::_viewNode)) {
		// Switch to perspective mode when camera node has been deleted.
		if(viewType() == VIEW_SCENENODE && newTarget == NULL) {
			setViewType(VIEW_PERSPECTIVE);
		}
		else {
			// Update viewport when the camera has been replaced by another scene node.
			if(_viewport) {
				updateViewportTitle();
				_viewport->updateViewport();
			}
		}
	}
#endif
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Is called when the value of a property field of this object has changed.
******************************************************************************/
void ViewportConfiguration::propertyChanged(const PropertyFieldDescriptor& field)
{
	RefTarget::propertyChanged(field);
	if(_viewport) {
		if(field == PROPERTY_FIELD(ViewportConfiguration::_viewType)) {
			updateViewportTitle();
		}
		_viewport->updateViewport();
	}
}

/******************************************************************************
* Updates the title text of the viewport based on the current view type.
******************************************************************************/
void ViewportConfiguration::updateViewportTitle()
{
	// Load viewport caption string.
	switch(viewType()) {
		case VIEW_TOP: _viewportTitle = tr("Top"); break;
		case VIEW_BOTTOM: _viewportTitle = tr("Bottom"); break;
		case VIEW_FRONT: _viewportTitle = tr("Front"); break;
		case VIEW_BACK: _viewportTitle = tr("Back"); break;
		case VIEW_LEFT: _viewportTitle = tr("Left"); break;
		case VIEW_RIGHT: _viewportTitle = tr("Right"); break;
		case VIEW_ORTHO: _viewportTitle = tr("Ortho"); break;
		case VIEW_PERSPECTIVE: _viewportTitle = tr("Perspective"); break;
		case VIEW_SCENENODE:
#if 0
			if(settings()->viewNode() != NULL)
				_viewportTitle = settings()->viewNode()->name();
			else
#endif
				_viewportTitle = tr("No view node");
		break;
		default: OVITO_ASSERT(false); _viewportTitle = QString(); // unknown viewport type
	}
}

/******************************************************************************
* The takes the current configuration of the viewports and
* saves it into this object.
******************************************************************************/
void ViewportConfigurationSet::saveConfiguration()
{
	if(!Application::instance().guiMode())
		return;

	UndoSuspender noUndo;

	// Reset first.
	_viewRecords.clear();
	_activeViewport = -1;
	_maximizedViewport = -1;

	CloneHelper cloneHelper;

	int index = 0;
	for(Viewport* vp : ViewportManager::instance().viewports()) {

		addViewport(cloneHelper.cloneObject(vp->settings(), false));
		if(vp == ViewportManager::instance().activeViewport())
			_activeViewport = index;
		if(vp == ViewportManager::instance().maximizedViewport())
			_maximizedViewport = index;

		index++;
	}
}

/******************************************************************************
* This applies the saved configuration to the viewports in the viewport panel.
******************************************************************************/
void ViewportConfigurationSet::restoreConfiguration()
{
	if(!Application::instance().guiMode())
		return;
#if 0
	// Resize viewports array if necessary.
	while(viewports.size() < records().size())
		vpPanel->addViewport();

	// Dispose viewports if there are too many.
	while(viewports.size() > records().size())
		vpPanel->removeViewport(vpPanel->viewports().back());

	ViewportManager::instance().setMaximizedViewport(NULL);
	ViewportManager::instance().setActiveViewport(NULL);

	QVector<Viewport*> viewports = ViewportManager::instance().viewports();
	OVITO_ASSERT(viewports.size() == records().size());
	CloneHelper cloneHelper;
	for(int i = 0; i < records().size(); i++) {
		Viewport* vp = viewports[i];
		vp->replaceSettings(cloneHelper.cloneObject(records()[i], false));
		if(i == _activeViewport)
			ViewportManager::instance().setActiveViewport(vp);
		if(i == _maximizedViewport)
			ViewportManager::instance().setMaximizedViewport(vp);
	}
#endif
}

};
