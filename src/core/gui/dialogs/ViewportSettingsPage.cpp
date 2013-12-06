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
#include "ViewportSettingsPage.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, ViewportSettingsPage, ApplicationSettingsPage)

/******************************************************************************
* Creates the widget that contains the plugin specific setting controls.
******************************************************************************/
void ViewportSettingsPage::insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	// Retrieve current settings.
	_settings.assign(ViewportSettings::getSettings());

	QWidget* page = new QWidget();
	tabWidget->addTab(page, tr("Viewports"));
	QGridLayout* layout1 = new QGridLayout(page);

	QGroupBox* upDirectionGroupBox = new QGroupBox(tr("Up direction"), page);
	layout1->addWidget(upDirectionGroupBox, 0, 0);
	QGridLayout* layout2 = new QGridLayout(upDirectionGroupBox);

	QLabel* label1 = new QLabel(tr("<html><p>Selects the direction to stay vertical when rotating the camera:</p></html>"));
	label1->setWordWrap(true);
	layout2->addWidget(label1, 0, 0, 1, 4);

	_upDirectionGroup = new QButtonGroup(page);
	_upDirectionGroup->addButton(new QRadioButton(tr("&X-axis"), upDirectionGroupBox), ViewportSettings::X_AXIS);
	_upDirectionGroup->addButton(new QRadioButton(tr("&Y-axis"), upDirectionGroupBox), ViewportSettings::Y_AXIS);
	_upDirectionGroup->addButton(new QRadioButton(tr("&Z-axis (default)"), upDirectionGroupBox), ViewportSettings::Z_AXIS);
	layout2->addWidget(_upDirectionGroup->button(ViewportSettings::X_AXIS), 1, 0, 1, 1);
	layout2->addWidget(_upDirectionGroup->button(ViewportSettings::Y_AXIS), 1, 1, 1, 1);
	layout2->addWidget(_upDirectionGroup->button(ViewportSettings::Z_AXIS), 1, 2, 1, 1);
	_upDirectionGroup->button(_settings.upDirection())->setChecked(true);
	layout2->setColumnStretch(3, 1);

	_restrictVerticalRotationBox = new QCheckBox(tr("Restrict rotation to keep axis pointing upward"));
	_restrictVerticalRotationBox->setChecked(_settings.restrictVerticalRotation());
	layout2->addWidget(_restrictVerticalRotationBox, 2, 0, 1, 3);

	QGroupBox* colorsGroupBox = new QGroupBox(tr("Colors"), page);
	layout1->addWidget(colorsGroupBox, 1, 0);
	layout2 = new QGridLayout(colorsGroupBox);

	_colorList = new QListWidget(colorsGroupBox);
	_colorList->addItem(tr("Background"));
	_colorList->addItem(tr("Grid lines (minor)"));
	_colorList->addItem(tr("Grid lines (major)"));
	_colorList->addItem(tr("Grid lines (axes)"));
	_colorList->addItem(tr("Viewport caption"));
	_colorList->addItem(tr("Viewport caption (active)"));
	_colorList->addItem(tr("Selection box"));
	_colorList->addItem(tr("Axis (active)"));
	_colorList->addItem(tr("Axis (inactive)"));
	_colorList->addItem(tr("Viewport border (inactive)"));
	_colorList->addItem(tr("Viewport border (active)"));
	_colorList->addItem(tr("Snapping marker"));
	_colorList->addItem(tr("Animation mode"));
	_colorList->addItem(tr("Render frame"));
	_colorList->addItem(tr("Cameras"));
	_colorList->addItem(tr("Lights"));
	layout2->addWidget(_colorList, 0, 0, 2, 1);
	connect(_colorList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(onColorListItemChanged(QListWidgetItem*, QListWidgetItem*)));

	_colorPicker = new ColorPickerWidget(colorsGroupBox);
	connect(_colorPicker, SIGNAL(colorChanged()), this, SLOT(onColorChanged()));
	QHBoxLayout* boxLayout = new QHBoxLayout();
	layout2->addLayout(boxLayout, 0, 1, Qt::AlignLeft | Qt::AlignTop);
	boxLayout->addWidget(new QLabel(tr("Color:")));
	boxLayout->addWidget(_colorPicker, 1);
	layout2->setColumnStretch(1, 1);
	layout2->setRowStretch(1, 1);
	_colorList->setCurrentRow(0);

	QPushButton* restoreDefaultButton = new QPushButton(tr("Restore default colors"));
	layout2->addWidget(restoreDefaultButton, 1, 1, Qt::AlignLeft | Qt::AlignBottom);
	connect(restoreDefaultButton, SIGNAL(clicked(bool)), this, SLOT(onRestoreDefaultColors()));

	layout1->setRowStretch(2, 1);
}

/******************************************************************************
* Is called when the user selects another entry in the color list.
******************************************************************************/
void ViewportSettingsPage::onColorListItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	_colorPicker->setColor(_settings.viewportColor((ViewportSettings::ViewportColor)_colorList->currentRow()));
}

/******************************************************************************
* Is called when the user has changed a viewport element color.
******************************************************************************/
void ViewportSettingsPage::onColorChanged()
{
	_settings.setViewportColor((ViewportSettings::ViewportColor)_colorList->currentRow(), _colorPicker->color());
}

/******************************************************************************
* Is called when the user clicks the "Restore defaults" button in the color section.
******************************************************************************/
void ViewportSettingsPage::onRestoreDefaultColors()
{
	_settings.restoreDefaultViewportColors();
	_colorPicker->setColor(_settings.viewportColor((ViewportSettings::ViewportColor)_colorList->currentRow()));
}

/******************************************************************************
* Lets the page save all changed settings.
******************************************************************************/
bool ViewportSettingsPage::saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	// Update settings.
	_settings.setUpDirection((ViewportSettings::UpDirection)_upDirectionGroup->checkedId());
	_settings.setRestrictVerticalRotation(_restrictVerticalRotationBox->isChecked());

	// Store current settings.
	ViewportSettings::setSettings(_settings);

	return true;
}

};
