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

#ifndef __OVITO_RENDER_COMMAND_PAGE_H
#define __OVITO_RENDER_COMMAND_PAGE_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesPanel.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * The command panel page lets user render the scene.
 */
class OVITO_CORE_EXPORT RenderCommandPage : public QWidget
{
	Q_OBJECT

public:

	/// Initializes the render page.
    RenderCommandPage(MainWindow* mainWindow, QWidget* parent);

private Q_SLOTS:

	/// This is called when a new dataset has been loaded.
	void onDataSetChanged(DataSet* newDataSet);

	/// This is called when new render settings have been loaded.
	void onRenderSettingsReplaced(RenderSettings* newRenderSettings);

private:

	/// This panel shows the properties of the render settings object.
	PropertiesPanel* propertiesPanel;

	QMetaObject::Connection _renderSettingsReplacedConnection;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_RENDER_COMMAND_PAGE_H
