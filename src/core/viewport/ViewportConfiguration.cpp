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

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(ViewportConfiguration, RefTarget);
DEFINE_FLAGS_VECTOR_REFERENCE_FIELD(ViewportConfiguration, _viewports, "Viewports", Viewport, PROPERTY_FIELD_NO_UNDO|PROPERTY_FIELD_ALWAYS_CLONE)
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _activeViewport, "ActiveViewport", Viewport, PROPERTY_FIELD_NO_UNDO)
DEFINE_FLAGS_REFERENCE_FIELD(ViewportConfiguration, _maximizedViewport, "MaximizedViewport", Viewport, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* This gets a viewport configuration that should be
* used as template for new scene files.
******************************************************************************/
OORef<ViewportConfiguration> ViewportConfiguration::defaultConfiguration()
{
	/// This holds the default configuration of viewports to use.
	static OORef<ViewportConfiguration> defaultConfig;

	// Make sure the default configuration is initialized.
	if(!defaultConfig) {
		defaultConfig = new ViewportConfiguration();

		OORef<Viewport> topView = new Viewport();
		topView->setViewType(Viewport::VIEW_TOP);
		defaultConfig->addViewport(topView);

		OORef<Viewport> frontView = new Viewport();
		frontView->setViewType(Viewport::VIEW_FRONT);
		defaultConfig->addViewport(frontView);

		OORef<Viewport> leftView = new Viewport();
		leftView->setViewType(Viewport::VIEW_LEFT);
		defaultConfig->addViewport(leftView);

		OORef<Viewport> perspectiveView = new Viewport();
		perspectiveView->setViewType(Viewport::VIEW_PERSPECTIVE);
		defaultConfig->addViewport(perspectiveView);

		defaultConfig->setActiveViewport(topView.get());
		defaultConfig->setMaximizedViewport(NULL);
	}

	return defaultConfig;
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void ViewportConfiguration::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(ViewportConfiguration::_activeViewport)) {
		activeViewportChanged(_activeViewport);
	}
	else if(field == PROPERTY_FIELD(ViewportConfiguration::_maximizedViewport)) {
		maximizedViewportChanged(_maximizedViewport);
	}
	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}


};
