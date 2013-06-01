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

#ifndef __OVITO_VIEWPORTS_PANEL_H
#define __OVITO_VIEWPORTS_PANEL_H

#include <core/Core.h>
#include <core/reference/RefTargetListener.h>
#include "Viewport.h"
#include "input/ViewportInputManager.h"

namespace Ovito {

class DataSet;		// defined in DataSet.h

/**
 * \brief The container for the viewports in the application's main window.
 */
class ViewportsPanel : public QWidget
{
	Q_OBJECT

public:

	/// \brief Constructs the viewport panel.
	ViewportPanel(QWidget* parent);

	/// \brief Returns the list of viewports in the panel.
	/// \return A list of viewports.
	const QVector<Viewport*>& viewports() const { return _viewports; }

	/// \brief Creates a new viewport in the panel.
	/// \return The new viewport instance.
	Viewport* createViewport();

	/// \brief Removes and destroys a viewport.
	/// \param vp The viewport to be removed.
	void removeViewport(Viewport* vp);

public Q_SLOTS:

	/// \brief Performs the layout of the viewports in the panel.
	void layoutViewports();

	/// \brief Updates the cursor for each viewport.
	///
	/// This method is called when the active ViewportInputHandler has changed.
	void updateViewportCursor();

protected:

	/// \brief Renders the borders around the viewports.
	virtual void paintEvent(QPaintEvent* event) override;

	/// Handles size event for the window.
	virtual void resizeEvent(QResizeEvent* event) override;

private:

	/// The list of viewports.
	QVector<Viewport*> _viewports;

private Q_SLOTS:

	/// Updates the viewport panel to reflect the viewport configuration saved in the current DataSet.
	/// This method is called each time a new DataSet has become active.
	void reset(DataSet* newDataSet);
};


};

#endif // __OVITO_VIEWPORTS_PANEL_H
