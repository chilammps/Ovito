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
#include <core/gui/widgets/general/SpinnerWidget.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/utilities/units/UnitsManager.h>
#include "AdjustCameraDialog.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* The constructor of the viewport settings dialog.
******************************************************************************/
AdjustCameraDialog::AdjustCameraDialog(Viewport* viewport, QWidget* parent) :
	QDialog(parent), _viewport(viewport)
{
	setWindowTitle(tr("Adjust View"));
	
	_oldViewType = viewport->viewType();
	_oldCameraTM = viewport->cameraTransformation();
	_oldFOV = viewport->fieldOfView();

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QGroupBox* viewPosBox = new QGroupBox(tr("View position"));
	mainLayout->addWidget(viewPosBox);

	QGridLayout* gridLayout = new QGridLayout(viewPosBox);
	gridLayout->setColumnStretch(1,1);
	gridLayout->setColumnStretch(2,1);
	gridLayout->setColumnStretch(3,1);
	gridLayout->addWidget(new QLabel(tr("XYZ:")), 0, 0);

	QHBoxLayout* fieldLayout;
	QLineEdit* textBox;

	_camPosXSpinner = new SpinnerWidget();
	_camPosYSpinner = new SpinnerWidget();
	_camPosZSpinner = new SpinnerWidget();
	_camPosXSpinner->setUnit(viewport->dataset()->unitsManager().worldUnit());
	_camPosYSpinner->setUnit(viewport->dataset()->unitsManager().worldUnit());
	_camPosZSpinner->setUnit(viewport->dataset()->unitsManager().worldUnit());

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camPosXSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camPosXSpinner);
	gridLayout->addLayout(fieldLayout, 0, 1);
	connect(_camPosXSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camPosYSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camPosYSpinner);
	gridLayout->addLayout(fieldLayout, 0, 2);
	connect(_camPosYSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camPosZSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camPosZSpinner);
	gridLayout->addLayout(fieldLayout, 0, 3);
	connect(_camPosZSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	QGroupBox* viewDirBox = new QGroupBox(tr("View direction"));
	mainLayout->addWidget(viewDirBox);

	gridLayout = new QGridLayout(viewDirBox);
	gridLayout->setColumnStretch(1,1);
	gridLayout->setColumnStretch(2,1);
	gridLayout->setColumnStretch(3,1);
	gridLayout->addWidget(new QLabel(tr("XYZ:")), 0, 0);

	_camDirXSpinner = new SpinnerWidget();
	_camDirYSpinner = new SpinnerWidget();
	_camDirZSpinner = new SpinnerWidget();
	_camDirXSpinner->setUnit(viewport->dataset()->unitsManager().worldUnit());
	_camDirYSpinner->setUnit(viewport->dataset()->unitsManager().worldUnit());
	_camDirZSpinner->setUnit(viewport->dataset()->unitsManager().worldUnit());

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camDirXSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camDirXSpinner);
	gridLayout->addLayout(fieldLayout, 0, 1);
	connect(_camDirXSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camDirYSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camDirYSpinner);
	gridLayout->addLayout(fieldLayout, 0, 2);
	connect(_camDirYSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camDirZSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camDirZSpinner);
	gridLayout->addLayout(fieldLayout, 0, 3);
	connect(_camDirZSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	QGroupBox* projectionBox = new QGroupBox(tr("Projection type"));
	mainLayout->addWidget(projectionBox);

	gridLayout = new QGridLayout(projectionBox);
	gridLayout->setColumnMinimumWidth(0, 30);
	gridLayout->setColumnStretch(3, 1);

	_camPerspective = new QRadioButton(tr("Perspective:"));
	connect(_camPerspective, &QRadioButton::clicked, this, &AdjustCameraDialog::onAdjustCamera);
	gridLayout->addWidget(_camPerspective, 0, 0, 1, 3);

	gridLayout->addWidget(new QLabel(tr("View angle:")), 1, 1);
	_camFOVAngleSpinner = new SpinnerWidget();
	_camFOVAngleSpinner->setUnit(_viewport->dataset()->unitsManager().angleUnit());
	_camFOVAngleSpinner->setMinValue(1e-4f);
	_camFOVAngleSpinner->setMaxValue(FLOATTYPE_PI - 1e-2);
	_camFOVAngleSpinner->setFloatValue(35.0*FLOATTYPE_PI/180.0);
	_camFOVAngleSpinner->setEnabled(false);
	connect(_camPerspective, &QRadioButton::toggled, _camFOVAngleSpinner, &SpinnerWidget::setEnabled);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camFOVAngleSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camFOVAngleSpinner);
	gridLayout->addLayout(fieldLayout, 1, 2);
	connect(_camFOVAngleSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	_camParallel = new QRadioButton(tr("Parallel:"));
	connect(_camParallel, &QRadioButton::clicked, this, &AdjustCameraDialog::onAdjustCamera);
	gridLayout->addWidget(_camParallel, 2, 0, 1, 3);

	gridLayout->addWidget(new QLabel(tr("Field of view:")), 3, 1);
	_camFOVSpinner = new SpinnerWidget();
	_camFOVSpinner->setUnit(_viewport->dataset()->unitsManager().worldUnit());
	_camFOVSpinner->setMinValue(1e-4f);
	_camFOVSpinner->setFloatValue(200.0f);
	_camFOVSpinner->setEnabled(false);
	connect(_camParallel, &QRadioButton::toggled, _camFOVSpinner, &SpinnerWidget::setEnabled);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camFOVSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camFOVSpinner);
	gridLayout->addLayout(fieldLayout, 3, 2);
	connect(_camFOVSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal, this);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &AdjustCameraDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &AdjustCameraDialog::onCancel);
	connect(buttonBox, &QDialogButtonBox::helpRequested, [this]() {
		_viewport->dataset()->mainWindow()->openHelpTopic("viewports.adjust_view_dialog.html");
	});
	mainLayout->addWidget(buttonBox);

	updateGUI();
}

/******************************************************************************
* Updates the values displayed in the dialog.
******************************************************************************/
void AdjustCameraDialog::updateGUI()
{
	Point3 cameraPos = _viewport->cameraPosition();
	Vector3 cameraDir = _viewport->cameraDirection();
	_camPosXSpinner->setFloatValue(cameraPos.x());
	_camPosYSpinner->setFloatValue(cameraPos.y());
	_camPosZSpinner->setFloatValue(cameraPos.z());
	_camDirXSpinner->setFloatValue(cameraDir.x());
	_camDirYSpinner->setFloatValue(cameraDir.y());
	_camDirZSpinner->setFloatValue(cameraDir.z());

	if(_viewport->isPerspectiveProjection()) {
		_camPerspective->setChecked(true);
		_camFOVAngleSpinner->setFloatValue(_viewport->fieldOfView());
	}
	else {
		_camParallel->setChecked(true);
		_camFOVSpinner->setFloatValue(_viewport->fieldOfView());
	}
}

/******************************************************************************
* Is called when the user has changed the camera settings.
******************************************************************************/
void AdjustCameraDialog::onAdjustCamera()
{
	if(_camPerspective->isChecked()) {
		_viewport->setViewType(Viewport::VIEW_PERSPECTIVE);
		_viewport->setFieldOfView(_camFOVAngleSpinner->floatValue());
	}
	else {
		_viewport->setViewType(Viewport::VIEW_ORTHO);
		_viewport->setFieldOfView(_camFOVSpinner->floatValue());
	}

	_viewport->setCameraPosition(Point3(_camPosXSpinner->floatValue(), _camPosYSpinner->floatValue(), _camPosZSpinner->floatValue()));
	_viewport->setCameraDirection(Vector3(_camDirXSpinner->floatValue(), _camDirYSpinner->floatValue(), _camDirZSpinner->floatValue()));
}

/******************************************************************************
* Event handler for the Cancel button.
******************************************************************************/
void AdjustCameraDialog::onCancel()
{
	_viewport->setViewType(_oldViewType);
	_viewport->setCameraTransformation(_oldCameraTM);
	_viewport->setFieldOfView(_oldFOV);

	reject();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
