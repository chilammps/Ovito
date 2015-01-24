///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_OVERLAY_COMMAND_PAGE_H
#define __OVITO_OVERLAY_COMMAND_PAGE_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesPanel.h>
#include <core/reference/RefTargetListener.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/overlay/ViewportOverlay.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * The command panel tab lets the user edit the viewport overlays.
 */
class OVITO_CORE_EXPORT OverlayCommandPage : public QWidget
{
	Q_OBJECT

public:

	/// Initializes the modify page.
    OverlayCommandPage(MainWindow* mainWindow, QWidget* parent);

protected Q_SLOTS:

	/// This is called whenever the current viewport configuration of current dataset has been replaced by a new one.
	void onViewportConfigReplaced(ViewportConfiguration* newViewportConfiguration);

	/// This is called when another viewport became active.
	void onActiveViewportChanged(Viewport* activeViewport);

	/// This is called when the viewport generates a reference event.
	void viewportEvent(ReferenceEvent* event);

	/// Is called when a new overlay has been selected in the list box.
	void onItemSelectionChanged();

	/// This inserts a new overlay.
	void onNewOverlay(int index);

	/// This deletes the selected overlay.
	void onDeleteOverlay();

private:

	/// Returns the active viewport.
	Viewport* activeViewport() const { return _viewportListener.target(); }

	/// Returns the selected overlay.
	ViewportOverlay* selectedOverlay() const;

	/// The container of the current dataset being edited.
	DataSetContainer& _datasetContainer;

	/// Receives reference events from the current viewport.
	RefTargetListener<Viewport> _viewportListener;

	/// Contains the list of available overlay types.
	QComboBox* _newOverlayBox;

	/// This list box shows the overlays of the active viewport.
	QListWidget* _overlayListWidget;

	/// This panel shows the properties of the selected overlay.
	PropertiesPanel* _propertiesPanel;

	/// This label displays the selected viewport.
	QLabel* _activeViewportLabel;

	QMetaObject::Connection _activeViewportChangedConnection;
	QAction* _deleteOverlayAction;

	class OverlayListItem : public RefTargetListener<ViewportOverlay>, public QListWidgetItem
	{
	public:
		/// Constructor.
		OverlayListItem(ViewportOverlay* overlay);
	};
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OVERLAY_COMMAND_PAGE_H
