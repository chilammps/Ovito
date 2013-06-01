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
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetManager.h>
#include <core/viewport/ViewportManager.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/reference/CloneHelper.h>
#include <core/animation/AnimationSettings.h>
#if 0
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/rendering/RenderSettings.h>
#endif

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(DataSet, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _viewportConfig, "ViewportConfiguration", ViewportConfigurationSet, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _animSettings, "AnimationSettings", AnimationSettings, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
#if 0
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _sceneRoot, "SceneRoot", SceneRoot, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _selection, "CurrentSelection", SelectionSet, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _renderSettings, "RenderSettings", RenderSettings, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
#endif
SET_PROPERTY_FIELD_LABEL(DataSet, _viewportConfig, "Viewport Configuration")
SET_PROPERTY_FIELD_LABEL(DataSet, _animSettings, "Animation Settings")
#if 0
SET_PROPERTY_FIELD_LABEL(DataSet, _sceneRoot, "Scene")
SET_PROPERTY_FIELD_LABEL(DataSet, _selection, "Selection")
SET_PROPERTY_FIELD_LABEL(DataSet, _renderSettings, "Render Settings")
#endif

/******************************************************************************
* Constructor.
******************************************************************************/
DataSet::DataSet() : _hasBeenChanged(false)
{
	INIT_PROPERTY_FIELD(DataSet::_viewportConfig);
	INIT_PROPERTY_FIELD(DataSet::_animSettings);
#if 0
	INIT_PROPERTY_FIELD(DataSet::_sceneRoot);
	INIT_PROPERTY_FIELD(DataSet::_selection);
	INIT_PROPERTY_FIELD(DataSet::_renderSettings);
#endif

	// Create a new viewport configuration by copying the default template.
	_viewportConfig = CloneHelper().cloneObject(ViewportManager::instance().defaultConfiguration(), true);

	_animSettings = new AnimationSettings();
#if 0
	_sceneRoot = new SceneRoot();
	_selection = new SelectionSet();
	_renderSettings = new RenderSettings();
#endif
}


/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool DataSet::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged) {
		// Set dirty flag.
		setDirty();

		// Update all viewports when something has changed in the current data set.
		if(this == DataSetManager::instance().currentSet()) {
			ViewportManager::instance().updateViewports();
		}
	}
	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* ISaves the class' contents to the given stream.
******************************************************************************/
void DataSet::saveToStream(ObjectSaveStream& stream)
{
	// Acquire current viewport configuration.
	if(DataSetManager::instance().currentSet() == this)
		viewportConfig()->saveConfiguration();

	// Write everything into the file stream.
	RefTarget::saveToStream(stream);
}


};
