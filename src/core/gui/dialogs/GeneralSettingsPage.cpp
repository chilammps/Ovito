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
#include <core/viewport/ViewportWindow.h>
#include "GeneralSettingsPage.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Core, GeneralSettingsPage, ApplicationSettingsDialogPage);

/******************************************************************************
* Creates the widget that contains the plugin specific setting controls.
******************************************************************************/
void GeneralSettingsPage::insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	QWidget* page = new QWidget();
	tabWidget->addTab(page, tr("General"));
	QVBoxLayout* layout1 = new QVBoxLayout(page);

	QSettings settings;

	QGroupBox* uiGroupBox = new QGroupBox(tr("User interface"), page);
	layout1->addWidget(uiGroupBox);
	QGridLayout* layout2 = new QGridLayout(uiGroupBox);

	_useQtFileDialog = new QCheckBox(tr("Use alternative file selection dialog"), uiGroupBox);
	_useQtFileDialog->setToolTip(tr(
			"<p>Use an alternative file selection dialog instead of the native dialog box provided by the operating system.</p>"));
	layout2->addWidget(_useQtFileDialog, 0, 0);
	_useQtFileDialog->setChecked(settings.value("file/use_qt_dialog", false).toBool());

	QGroupBox* openglGroupBox = new QGroupBox(tr("Display / OpenGL"), page);
	layout1->addWidget(openglGroupBox);
	layout2 = new QGridLayout(openglGroupBox);

	// OpenGL context sharing:
	_overrideGLContextSharing = new QCheckBox(tr("Override context sharing"), openglGroupBox);
	_overrideGLContextSharing->setToolTip(tr("<p>Activate this option to explicitly control the sharing of OpenGL contexts between viewport windows.</p>"));
	layout2->addWidget(_overrideGLContextSharing, 0, 0);
	_contextSharingMode = new QComboBox(openglGroupBox);
	_contextSharingMode->setEnabled(false);
	if(ViewportWindow::contextSharingEnabled(true)) {
		_contextSharingMode->addItem(tr("Enable sharing (default)"));
		_contextSharingMode->addItem(tr("Disable sharing"));
	}
	else {
		_contextSharingMode->addItem(tr("Enable sharing"));
		_contextSharingMode->addItem(tr("Disable sharing (default)"));
	}
	layout2->addWidget(_contextSharingMode, 0, 1);
	connect(_overrideGLContextSharing, &QCheckBox::toggled, _contextSharingMode, &QComboBox::setEnabled);
	_overrideGLContextSharing->setChecked(settings.contains("display/share_opengl_context"));
	_contextSharingMode->setCurrentIndex(ViewportWindow::contextSharingEnabled() ? 0 : 1);

	// OpenGL point sprites:
	_overrideUseOfPointSprites = new QCheckBox(tr("Override usage of point sprites"), openglGroupBox);
	_overrideUseOfPointSprites->setToolTip(tr("<p>Activate this option to explicitly control the usage of OpenGL point sprites for rendering of particles.</p>"));
	layout2->addWidget(_overrideUseOfPointSprites, 1, 0);
	_pointSpriteMode = new QComboBox(openglGroupBox);
	_pointSpriteMode->setEnabled(false);
	if(ViewportWindow::pointSpritesEnabled(true)) {
		_pointSpriteMode->addItem(tr("Use point sprites (default)"));
		_pointSpriteMode->addItem(tr("Don't use point sprites"));
	}
	else {
		_pointSpriteMode->addItem(tr("Use point sprites"));
		_pointSpriteMode->addItem(tr("Don't use point sprites (default)"));
	}
	layout2->addWidget(_pointSpriteMode, 1, 1);
	connect(_overrideUseOfPointSprites, &QCheckBox::toggled, _pointSpriteMode, &QComboBox::setEnabled);
	_overrideUseOfPointSprites->setChecked(settings.contains("display/use_point_sprites"));
	_pointSpriteMode->setCurrentIndex(ViewportWindow::pointSpritesEnabled() ? 0 : 1);

	// OpenGL geometry shaders:
	_overrideUseOfGeometryShaders = new QCheckBox(tr("Override usage of geometry shaders"), openglGroupBox);
	_overrideUseOfGeometryShaders->setToolTip(tr("<p>Activate this option to explicitly control the usage of OpenGL geometry shaders.</p>"));
	layout2->addWidget(_overrideUseOfGeometryShaders, 2, 0);
	_geometryShaderMode = new QComboBox(openglGroupBox);
	_geometryShaderMode->setEnabled(false);
	if(ViewportWindow::geometryShadersEnabled(true)) {
		_geometryShaderMode->addItem(tr("Use geometry shaders (default)"));
		_geometryShaderMode->addItem(tr("Don't use geometry shaders"));
	}
	else {
		_geometryShaderMode->addItem(tr("Use geometry shaders"));
		_geometryShaderMode->addItem(tr("Don't use geometry shaders (default)"));
	}
	layout2->addWidget(_geometryShaderMode, 2, 1);
	connect(_overrideUseOfGeometryShaders, &QCheckBox::toggled, _geometryShaderMode, &QComboBox::setEnabled);
	_overrideUseOfGeometryShaders->setChecked(settings.contains("display/use_geometry_shaders"));
	_geometryShaderMode->setCurrentIndex(ViewportWindow::geometryShadersEnabled() ? 0 : 1);

	layout2->addWidget(new QLabel(tr("<p style=\"font-size: small; color: #686868;\">(Restart required for changes to take effect.)</p>")), 3, 0, 1, 2);

	QGroupBox* updateGroupBox = new QGroupBox(tr("Program updates"), page);
	layout1->addWidget(updateGroupBox);
	layout2 = new QGridLayout(updateGroupBox);

	_enableUpdateChecks = new QCheckBox(tr("Auto-refresh news page from web server"), updateGroupBox);
	_enableUpdateChecks->setToolTip(tr(
			"<p>The news page is fetched from <i>www.ovito.org</i> and displayed on each program startup. "
			"It contains information about new program updates when they become available.</p>"));
	layout2->addWidget(_enableUpdateChecks, 0, 0);
	_enableUsageStatistics = new QCheckBox(tr("Send unique installation ID to web server"), updateGroupBox);
	_enableUsageStatistics->setToolTip(tr(
			"<p>Every installation of OVITO has a unique identifier, which is generated on first program start. "
			"This option enables the transmission of the anonymous identifier to the web server to help the developers collect "
			"program usage statistics.</p>"));
	layout2->addWidget(_enableUsageStatistics, 1, 0);

	_enableUpdateChecks->setChecked(settings.value("updates/check_for_updates", true).toBool());
	_enableUsageStatistics->setChecked(settings.value("updates/transmit_id", true).toBool());

	connect(_enableUpdateChecks, &QCheckBox::toggled, _enableUsageStatistics, &QCheckBox::setEnabled);
	_enableUsageStatistics->setEnabled(_enableUpdateChecks->isChecked());

	layout1->addStretch();
}

/******************************************************************************
* Lets the page save all changed settings.
******************************************************************************/
bool GeneralSettingsPage::saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	QSettings settings;
	settings.setValue("file/use_qt_dialog", _useQtFileDialog->isChecked());
	settings.setValue("updates/check_for_updates", _enableUpdateChecks->isChecked());
	settings.setValue("updates/transmit_id", _enableUsageStatistics->isChecked());
	if(_overrideGLContextSharing->isChecked())
		settings.setValue("display/share_opengl_context", _contextSharingMode->currentIndex() == 0);
	else
		settings.remove("display/share_opengl_context");
	if(_overrideUseOfPointSprites->isChecked())
		settings.setValue("display/use_point_sprites", _pointSpriteMode->currentIndex() == 0);
	else
		settings.remove("display/use_point_sprites");
	if(_overrideUseOfGeometryShaders->isChecked())
		settings.setValue("display/use_geometry_shaders", _geometryShaderMode->currentIndex() == 0);
	else
		settings.remove("display/use_geometry_shaders");
	return true;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
