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
#include <core/gui/actions/ActionManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>
#include <core/rendering/RenderSettings.h>
#include <core/viewport/ViewportConfiguration.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

/******************************************************************************
* Handles the ACTION_RENDER_ACTIVE_VIEWPORT command.
******************************************************************************/
void ActionManager::on_RenderActiveViewport_triggered()
{
	try {
		// Set input focus to main window.
		// This will process any pending user inputs in QLineEdit fields that haven't been processed yet.
		mainWindow()->setFocus();

		// Get the current render settings.
		RenderSettings* settings = _dataset->renderSettings();

		// Get viewport to be rendered.
		Viewport* viewport = _dataset->viewportConfig()->activeViewport();
		if(!viewport)
			throw Exception(tr("There is no active viewport to render."));

		// Call high-level rendering function, which will take care of the rest.
		_dataset->renderScene(settings, viewport);
	}
	catch(const Exception& ex) {
		ex.logError();
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
