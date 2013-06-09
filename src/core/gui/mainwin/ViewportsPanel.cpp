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
#include "ViewportsPanel.h"
#include <core/viewport/ViewportManager.h>
#include <core/viewport/ViewportSettings.h>
#include <core/dataset/DataSetManager.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

/******************************************************************************
* The constructor of the viewports panel class.
******************************************************************************/
ViewportsPanel::ViewportsPanel(QWidget* parent) : QWidget(parent)
{
	// Repaint the viewport borders if the animation mode has been activated.
	connect(&AnimManager::instance(), SIGNAL(animationModeChanged(bool)), SLOT(update()));

	// Repaint the viewport borders if another viewport has been activated.
	connect(&ViewportManager::instance(), SIGNAL(activeViewportChanged(Viewport*)), SLOT(update()));

	// Update layout when a viewport has been maximized.
	connect(&ViewportManager::instance(), SIGNAL(maximizedViewportChanged(Viewport*)), SLOT(layoutViewports()));

	// Active the new viewport layout as soon as a new scene file is loaded.
	connect(&DataSetManager::instance(), SIGNAL(dataSetReset(DataSet*)), this, SLOT(onDataSetReset(DataSet*)));
}

/******************************************************************************
* This is called when a new dataset has been loaded.
******************************************************************************/
void ViewportsPanel::onDataSetReset(DataSet* newDataSet)
{
	// Delete all existing viewport widgets first.
	for(QWidget* widget : findChildren<QWidget*>()) {
		delete widget;
	}

	// Create widgets for new viewports.
	const QVector<Viewport*>& viewports = ViewportManager::instance().viewports();
	for(Viewport* vp : viewports) {
		vp->createWidget(this);
	}

	// Layout viewport widgets.
	layoutViewports();
}

/******************************************************************************
* Renders the borders of the viewports.
******************************************************************************/
void ViewportsPanel::paintEvent(QPaintEvent* event)
{
	// Render border around active viewport.
	Viewport* vp = ViewportManager::instance().activeViewport();
	if(!vp) return;
	QWidget* vpWidget = vp->widget();
	if(!vpWidget || vpWidget->isHidden()) return;

	QPainter painter(this);

	// Choose a color for the viewport border.
	Color borderColor;
	if(AnimManager::instance().animationMode())
		borderColor = Viewport::viewportColor(ViewportSettings::COLOR_ANIMATION_MODE);
	else
		borderColor = Viewport::viewportColor(ViewportSettings::COLOR_ACTIVE_VIEWPORT_BORDER);

	painter.setPen((QColor)borderColor);
	QRect rect = vpWidget->geometry();
	rect.adjust(-1, -1, 0, 0);
	painter.drawRect(rect);
	rect.adjust(-1, -1, 1, 1);
	painter.drawRect(rect);
}

/******************************************************************************
* Handles size event for the window.
* Does the actual calculation of its children's positions and sizes.
******************************************************************************/
void ViewportsPanel::resizeEvent(QResizeEvent* event)
{
	layoutViewports();
}

/******************************************************************************
* Performs the layout of the viewports.
* Does the actual calculation of its children's positions and sizes.
******************************************************************************/
void ViewportsPanel::layoutViewports()
{
	const QVector<Viewport*>& viewports = ViewportManager::instance().viewports();
	Viewport* maximizedViewport = ViewportManager::instance().maximizedViewport();
	OVITO_ASSERT(viewports.size() == findChildren<QWidget*>().size());

	// Count the number of visible window.
	int nvisible = 0;
	for(Viewport* viewport : viewports) {
		if(!viewport->widget()) continue;
		if(maximizedViewport == NULL || maximizedViewport == viewport) {
			nvisible++;
			viewport->widget()->setVisible(true);
		}
		else
			viewport->widget()->setVisible(false);
	}
	if(nvisible == 0) return;

	// Compute number of rows/columns
	int rows = (int)(sqrt((double)nvisible) + 0.5);
	int columns = (nvisible+rows-1) / rows;

	// Get client rect.
	QRect clientRect = rect();

	// Position items.
	int count = 0;
	bool needsRepaint = false;
	for(Viewport* viewport : viewports) {
		QWidget* vpWidget = viewport->widget();
		if(vpWidget->isHidden()) continue;

		int x = count%columns;
		int y = count/columns;
		QRect rect(clientRect.topLeft(), QSize(0,0));
		rect.translate(clientRect.width() * x / columns, clientRect.height() * y / rows);
		rect.setWidth((clientRect.width() * (x+1) / columns) - rect.x());
		rect.setHeight((clientRect.height() * (y+1) / rows) - rect.y());
		rect.adjust(2,2,-2,-2);

		if(vpWidget->geometry() != rect) {
			vpWidget->setGeometry(rect);
			needsRepaint = true;
		}
		count++;
	}

	if(needsRepaint)
		update();
}


};
