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
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/viewport/ViewportConfiguration.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Viewport, RefMaker);
DEFINE_FLAGS_REFERENCE_FIELD(Viewport, _settings, "Settings", ViewportConfiguration, PROPERTY_FIELD_NO_UNDO)

/******************************************************************************
* Constructor.
******************************************************************************/
Viewport::Viewport() :
		_widget(NULL),
		_mouseOverCaption(false)
{
	INIT_PROPERTY_FIELD(Viewport::_settings);

	// Create internal settings object.
	_settings = new ViewportConfiguration(this);
}

/******************************************************************************
* Destructor.
******************************************************************************/
Viewport::~Viewport()
{
}

/******************************************************************************
* Displays the context menu for this viewport.
******************************************************************************/
void Viewport::showViewportMenu(const QPoint& pos)
{
#if 0
	/// The context menu of the viewport.
	ViewportMenu contextMenu(this);

	/// Show menu.
	contextMenu.exec(mapToGlobal(pos));
#endif
}

};
