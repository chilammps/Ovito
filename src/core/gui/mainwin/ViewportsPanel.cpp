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
#include <core/viewport/input/ViewportInputHandler.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/data/DataSetManager.h>
#include <core/scene/animation/AnimManager.h>
#include <core/actions/ActionManager.h>
#include <core/rendering/RenderSettings.h>

namespace Ovito {

/******************************************************************************
* The constructor of the viewports panel class.
******************************************************************************/
ViewportsPanel::ViewportPanel(QWidget* parent) : QWidget(parent)
{
	// The mouse cursor must updated each time when a new input mode becomes active.
	connect(&VIEWPORT_INPUT_MANAGER, SIGNAL(inputModeChanged(ViewportInputHandler*, ViewportInputHandler*)), this, SLOT(updateViewportCursor()));

	// Repaint the viewport borders if the animation mode has been activated.
	connect(&ANIM_MANAGER, SIGNAL(animationModeChanged(bool)), SLOT(update()));

	// Repaint the viewport borders if another viewport has been activated.
	connect(&ViewportManager::instance(), SIGNAL(activeViewportChanged(Viewport*)), SLOT(update()));

	// Update layout when a viewport has been maximized.
	connect(&ViewportManager::instance(), SIGNAL(maximizedViewportChanged(Viewport*)), SLOT(layoutViewports()));

	// Active the new viewport layout as soon as a new scene file is loaded.
	connect(&DATASET_MANAGER, SIGNAL(dataSetReset(DataSet*)), this, SLOT(reset(DataSet*)));
}

/******************************************************************************
* Creates a new viewport.
******************************************************************************/
Viewport* ViewportsPanel::createViewport()
{
	Viewport* vp = new Viewport(this);
	_viewports.push_back(vp);
	return vp;
}

/******************************************************************************
* Removes and destroys a viewport.
******************************************************************************/
void ViewportsPanel::removeViewport(Viewport* vp)
{
	CHECK_POINTER(vp);
	OVITO_ASSERT(_viewports.contains(vp));
	_viewports.remove(_viewports.indexOf(vp));
	delete vp;
}

/******************************************************************************
* Resets the viewport panel to the state saved in the current data set.
******************************************************************************/
void ViewportsPanel::reset(DataSet* newDataSet)
{
	if(newDataSet) {
		newDataSet->viewportConfig()->restoreConfiguration();
		layoutViewports();
	}
}

/******************************************************************************
* Updates the cursor for each viewport.
* The cursor is taken from the active viewport input handler.
******************************************************************************/
void ViewportsPanel::updateViewportCursor()
{
	ViewportInputHandler* handler = VIEWPORT_INPUT_MANAGER.currentHandler();
	if(handler && handler->temporaryNavigationMode()) handler = handler->temporaryNavigationMode();
	if(!handler)
		unsetCursor();
	else
		setCursor(handler->getCursor());
}

/******************************************************************************
* Renders the borders of the viewports.
******************************************************************************/
void ViewportsPanel::paintEvent(QPaintEvent* event)
{
	// Render border around active viewport.
	Viewport* vp = ViewportManager::instance().activeViewport();
	if(!vp || vp->isHidden()) return;

	QPainter painter(this);

	// Choose a color for the viewport border.
	ColorA borderColor;
	if(ANIM_MANAGER.animationMode())
		borderColor = Viewport::viewportColor(Viewport::COLOR_ANIMATION_MODE);
	else
		borderColor = Viewport::viewportColor(Viewport::COLOR_ACTIVE_VIEWPORT_BORDER);

	painter.setPen(borderColor);
	QRect rect = vp->geometry();
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
	ViewportManager::instance().updateViewports();
}

/******************************************************************************
* Performs the layout of the viewports.
* Does the actual calculation of its children's positions and sizes.
******************************************************************************/
void ViewportsPanel::layoutViewports()
{
	// Count the number of visible window.
	int nvisible = 0;
	for(Viewport* viewport : viewports()) {
		if(!viewport->isHidden()) nvisible++;
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
	Q_FOREACH(Viewport* viewport, viewports()) {
		if(viewport->isHidden()) continue;

		int x = count%columns;
		int y = count/columns;
		QRect rect(clientRect.topLeft(), QSize(0,0));
		rect.translate(clientRect.width() * x / columns, clientRect.height() * y / rows);
		rect.setWidth((clientRect.width() * (x+1) / columns) - rect.x());
		rect.setHeight((clientRect.height() * (y+1) / rows) - rect.y());
		rect.adjust(2,2,-2,-2);

		if(viewport->settings()->renderFrameShown()) {
			// Setup a viewport rectangle that has the same
			// aspect ratio as the rendering image.
			RenderSettings* renderSettings = DATASET_MANAGER.currentSet()->renderSettings();
			if(renderSettings && rect.width() > 0) {
				FloatType renderAspectRatio = renderSettings->outputImageAspectRatio();
				FloatType windowAspectRatio = (FloatType)rect.height() / (FloatType)rect.width();
				if(renderAspectRatio < windowAspectRatio) {
					int frameHeight = max((int)(rect.width() * renderAspectRatio), 1);
					rect = QRect(rect.x(), rect.y() + (rect.height() - frameHeight) / 2, rect.width(), frameHeight);
				}
				else {
					int frameWidth = max((int)(rect.height() / renderAspectRatio), 1);
					rect = QRect(rect.x() + (rect.width() - frameWidth) / 2, rect.y(), frameWidth, rect.height());
				}
			}
		}

		if(viewport->geometry() != rect) {
			viewport->setGeometry(rect);
			needsRepaint = true;
		}
		count++;
	}

	if(needsRepaint)
		update();
}


};
