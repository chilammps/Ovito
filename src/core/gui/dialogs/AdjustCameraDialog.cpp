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
#include <core/utilities/units/UnitsManager.h>
#include "AdjustCameraDialog.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* The constructor of the viewport settings dialog.
******************************************************************************/
AdjustCameraDialog::AdjustCameraDialog(Viewport* viewport, QWidget* parent) :
	QDialog(parent), _viewport(viewport)
{
	setWindowTitle(tr("Adjust Camera"));
	
	_oldViewType = viewport->viewType();
	_oldCameraTM = viewport->cameraTransformation();
	_oldFOV = viewport->fieldOfView();

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	QGridLayout* gridLayout = new QGridLayout();
	gridLayout->setColumnStretch(1,1);
	gridLayout->setColumnStretch(2,1);
	gridLayout->setColumnStretch(3,1);

	_camPerspective = new QCheckBox(tr("Perspective projection"));
	connect(_camPerspective, &QCheckBox::clicked, this, &AdjustCameraDialog::onAdjustCamera);
	connect(_camPerspective, &QCheckBox::clicked, this, &AdjustCameraDialog::updateGUI);
	mainLayout->addWidget(_camPerspective);

	QHBoxLayout* fieldLayout;
	QLineEdit* textBox;

	gridLayout->addWidget(new QLabel(tr("Camera position:")), 0, 0);

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

	gridLayout->addWidget(new QLabel(tr("Camera direction:")), 1, 0);

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
	gridLayout->addLayout(fieldLayout, 1, 1);
	connect(_camDirXSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camDirYSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camDirYSpinner);
	gridLayout->addLayout(fieldLayout, 1, 2);
	connect(_camDirYSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camDirZSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camDirZSpinner);
	gridLayout->addLayout(fieldLayout, 1, 3);
	connect(_camDirZSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	_camFOVLabel = new QLabel(tr("Field of view:"));
	gridLayout->addWidget(_camFOVLabel, 2, 0);
	_camFOVSpinner = new SpinnerWidget();
	_camFOVSpinner->setMinValue(1e-4f);

	fieldLayout = new QHBoxLayout();
	fieldLayout->setContentsMargins(0,0,0,0);
	fieldLayout->setSpacing(0);
	textBox = new QLineEdit();
	_camFOVSpinner->setTextBox(textBox);
	fieldLayout->addWidget(textBox);
	fieldLayout->addWidget(_camFOVSpinner);
	gridLayout->addLayout(fieldLayout, 2, 1);
	connect(_camFOVSpinner, &SpinnerWidget::spinnerValueChanged, this, &AdjustCameraDialog::onAdjustCamera);

	mainLayout->addLayout(gridLayout);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &AdjustCameraDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &AdjustCameraDialog::onCancel);
	mainLayout->addWidget(buttonBox);

	updateGUI();
}

/******************************************************************************
* Updates the values displayed in the dialog.
******************************************************************************/
void AdjustCameraDialog::updateGUI()
{
	_camPerspective->setChecked(_viewport->isPerspectiveProjection());
	Point3 cameraPos = _viewport->cameraPosition();
	Vector3 cameraDir = _viewport->cameraDirection();
	_camPosXSpinner->setFloatValue(cameraPos.x());
	_camPosYSpinner->setFloatValue(cameraPos.y());
	_camPosZSpinner->setFloatValue(cameraPos.z());
	_camDirXSpinner->setFloatValue(cameraDir.x());
	_camDirYSpinner->setFloatValue(cameraDir.y());
	_camDirZSpinner->setFloatValue(cameraDir.z());

	if(_viewport->isPerspectiveProjection()) {
		_camFOVSpinner->setUnit(_viewport->dataset()->unitsManager().angleUnit());
		_camFOVLabel->setText(tr("View angle:"));
		_camFOVSpinner->setMaxValue(FLOATTYPE_PI - 1e-2);
	}
	else {
		_camFOVSpinner->setUnit(_viewport->dataset()->unitsManager().worldUnit());
		_camFOVLabel->setText(tr("Field of view:"));
		_camFOVSpinner->setMaxValue(FLOATTYPE_MAX);
	}
	_camFOVSpinner->setFloatValue(_viewport->fieldOfView());
}

/******************************************************************************
* Is called when the user has changed the camera settings.
******************************************************************************/
void AdjustCameraDialog::onAdjustCamera()
{
	if(_camPerspective->isChecked()) {
		if(!_viewport->isPerspectiveProjection())
			_camFOVSpinner->setFloatValue(35.0*FLOATTYPE_PI/180.0);
		_viewport->setViewType(Viewport::VIEW_PERSPECTIVE);
	}
	else {
		if(_viewport->isPerspectiveProjection()) {
			_camFOVSpinner->setMaxValue(FLOATTYPE_MAX);
			_camFOVSpinner->setFloatValue(200.0f);
		}
		_viewport->setViewType(Viewport::VIEW_ORTHO);
	}

	_viewport->setCameraPosition(Point3(_camPosXSpinner->floatValue(), _camPosYSpinner->floatValue(), _camPosZSpinner->floatValue()));
	_viewport->setCameraDirection(Vector3(_camDirXSpinner->floatValue(), _camDirYSpinner->floatValue(), _camDirZSpinner->floatValue()));
	_viewport->setFieldOfView(_camFOVSpinner->floatValue());
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
