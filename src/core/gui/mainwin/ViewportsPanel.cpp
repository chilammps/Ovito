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

#include <core/Core.h>
#include <core/viewport/ViewportSettings.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/input/ViewportInputMode.h>
#include <core/viewport/input/ViewportInputManager.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetContainer.h>
#include <core/gui/mainwin/MainWindow.h>
#include "ViewportsPanel.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* The constructor of the viewports panel class.
******************************************************************************/
ViewportsPanel::ViewportsPanel(MainWindow* parent) : QWidget(parent)
{
	// Activate the new viewport layout as soon as a new scene file is loaded.
	connect(&parent->datasetContainer(), &DataSetContainer::viewportConfigReplaced, this, &ViewportsPanel::onViewportConfigurationReplaced);
	connect(&parent->datasetContainer(), &DataSetContainer::animationSettingsReplaced, this, &ViewportsPanel::onAnimationSettingsReplaced);

	// Track viewport input changes.
	connect(parent->viewportInputManager(), &ViewportInputManager::inputModeChanged, this, &ViewportsPanel::onInputModeChanged);
}

/******************************************************************************
* This is called when a new viewport configuration has been loaded.
******************************************************************************/
void ViewportsPanel::onViewportConfigurationReplaced(ViewportConfiguration* newViewportConfiguration)
{
	disconnect(_activeViewportChangedConnection);
	disconnect(_maximizedViewportChangedConnection);

	// Delete all existing viewport widgets first.
	for(QWidget* widget : findChildren<QWidget*>())
		delete widget;

	_viewportConfig = newViewportConfiguration;

	if(newViewportConfiguration) {

		// Create widgets for new viewports.
		for(Viewport* vp : newViewportConfiguration->viewports()) {
			vp->createWidget(this);
		}

		// Repaint the viewport borders when another viewport has been activated.
		_activeViewportChangedConnection = connect(newViewportConfiguration, &ViewportConfiguration::activeViewportChanged, this, (void (ViewportsPanel::*)())&ViewportsPanel::update);

		// Update layout when a viewport has been maximized.
		_maximizedViewportChangedConnection = connect(newViewportConfiguration, &ViewportConfiguration::maximizedViewportChanged, this, &ViewportsPanel::layoutViewports);

		// Layout viewport widgets.
		layoutViewports();
	}
}

/******************************************************************************
* This is called when new animation settings have been loaded.
******************************************************************************/
void ViewportsPanel::onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings)
{
	disconnect(_autoKeyModeChangedConnection);
	_animSettings = newAnimationSettings;

	if(newAnimationSettings) {
		_autoKeyModeChangedConnection = connect(newAnimationSettings, &AnimationSettings::autoKeyModeChanged, this, (void (ViewportsPanel::*)())&ViewportsPanel::update);
		_timeChangeCompleteConnection = connect(newAnimationSettings, &AnimationSettings::timeChangeComplete, this, (void (ViewportsPanel::*)())&ViewportsPanel::update);
	}
}

/******************************************************************************
* This is called when the current viewport input mode has changed.
******************************************************************************/
void ViewportsPanel::onInputModeChanged(ViewportInputMode* oldMode, ViewportInputMode* newMode)
{
	disconnect(_activeModeCursorChangedConnection);
	if(newMode) {
		_activeModeCursorChangedConnection = connect(newMode, &ViewportInputMode::curserChanged, this, &ViewportsPanel::viewportModeCursorChanged);
		viewportModeCursorChanged(newMode->cursor());
	}
	else viewportModeCursorChanged(cursor());
}

/******************************************************************************
* This is called when the mouse cursor of the active input mode has changed.
******************************************************************************/
void ViewportsPanel::viewportModeCursorChanged(const QCursor& cursor)
{
	if(_viewportConfig) {
		for(Viewport* vp : _viewportConfig->viewports())
			vp->setCursor(cursor);
	}
}

/******************************************************************************
* Renders the borders of the viewports.
******************************************************************************/
void ViewportsPanel::paintEvent(QPaintEvent* event)
{
	if(!_viewportConfig || !_animSettings) return;

	// Render border around active viewport.
	Viewport* vp = _viewportConfig->activeViewport();
	if(!vp) return;
	QWidget* vpWidget = vp->widget();
	if(!vpWidget || vpWidget->isHidden()) return;

	QPainter painter(this);

	// Choose a color for the viewport border.
	Color borderColor;
	if(_animSettings->autoKeyMode())
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
* Performs the layout of the viewport windows.
******************************************************************************/
void ViewportsPanel::layoutViewports()
{
	if(!_viewportConfig) return;
	const QVector<Viewport*>& viewports = _viewportConfig->viewports();
	Viewport* maximizedViewport = _viewportConfig->maximizedViewport();
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

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
