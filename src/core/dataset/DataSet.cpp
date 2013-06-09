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
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/rendering/RenderSettings.h>

#include <viz/data/SimulationCell.h>
#include <core/scene/ObjectNode.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(DataSet, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _viewportConfig, "ViewportConfiguration", ViewportConfiguration, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _animSettings, "AnimationSettings", AnimationSettings, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _sceneRoot, "SceneRoot", SceneRoot, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _selection, "CurrentSelection", SelectionSet, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _renderSettings, "RenderSettings", RenderSettings, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY)
SET_PROPERTY_FIELD_LABEL(DataSet, _viewportConfig, "Viewport Configuration")
SET_PROPERTY_FIELD_LABEL(DataSet, _animSettings, "Animation Settings")
SET_PROPERTY_FIELD_LABEL(DataSet, _sceneRoot, "Scene")
SET_PROPERTY_FIELD_LABEL(DataSet, _selection, "Selection")
SET_PROPERTY_FIELD_LABEL(DataSet, _renderSettings, "Render Settings")

/******************************************************************************
* Constructor.
******************************************************************************/
DataSet::DataSet()
{
	INIT_PROPERTY_FIELD(DataSet::_viewportConfig);
	INIT_PROPERTY_FIELD(DataSet::_animSettings);
	INIT_PROPERTY_FIELD(DataSet::_sceneRoot);
	INIT_PROPERTY_FIELD(DataSet::_selection);
	INIT_PROPERTY_FIELD(DataSet::_renderSettings);

	// Create a new viewport configuration by copying the default template.
	_viewportConfig = CloneHelper().cloneObject(DataSetManager::instance().defaultViewportConfiguration(), true);

	_animSettings = new AnimationSettings();
	_sceneRoot = new SceneRoot();
	_selection = new SelectionSet();
	_renderSettings = new RenderSettings();

	ObjectNode* node = new ObjectNode();
	_sceneRoot->addChild(node);
	Viz::SimulationCell* simCell = new Viz::SimulationCell(Box3(Point3::Origin(), 30));
	node->setSceneObject(simCell);
}

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool DataSet::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged) {
		// Update all viewports when something has changed in the current data set.
		if(this == DataSetManager::instance().currentSet() && source != viewportConfig()) {
			ViewportManager::instance().updateViewports();
		}
	}
	return RefTarget::referenceEvent(source, event);
}

};
