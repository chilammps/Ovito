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
#include "ParticleExporterSettingsDialog.h"

namespace Viz {

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleExporterSettingsDialog::ParticleExporterSettingsDialog(ParticleExporter* exporter, DataSet* dataset, QWidget* parent)
	: QDialog(parent), _exporter(exporter)
{
	setWindowTitle(tr("Export Settings"));

	QVBoxLayout* layout1 = new QVBoxLayout(this);
	QRadioButton* radioBtn;

	QGroupBox* rangeGroupBox = new QGroupBox(tr("Animation"), this);
	layout1->addWidget(rangeGroupBox);

	QGridLayout* rangeGroupLayout = new QGridLayout(rangeGroupBox);
	rangeGroupLayout->setColumnStretch(0, 5);
	rangeGroupLayout->setColumnStretch(1, 95);
	_rangeButtonGroup = new QButtonGroup(this);

	bool exportAnim = _exporter->exportAnimation();
	radioBtn = new QRadioButton(tr("Export current frame"));
	_rangeButtonGroup->addButton(radioBtn, 0);
	rangeGroupLayout->addWidget(radioBtn, 0, 0, 1, 2);
	radioBtn->setChecked(!exportAnim);

	radioBtn = new QRadioButton(tr("Export animation range"));
	_rangeButtonGroup->addButton(radioBtn, 1);
	rangeGroupLayout->addWidget(radioBtn, 1, 0, 1, 2);
	radioBtn->setChecked(exportAnim);
	radioBtn->setEnabled(dataset->animationSettings()->animationInterval().duration() != 0);

	QHBoxLayout* frameRangeLayout = new QHBoxLayout();
	rangeGroupLayout->addLayout(frameRangeLayout, 2, 1, 1, 1);

	frameRangeLayout->setSpacing(0);
	frameRangeLayout->addWidget(new QLabel(tr("From:")));
	_startTimeSpinner = new SpinnerWidget();
	_startTimeSpinner->setUnit(UnitsManager::instance().integerIdentityUnit());
	_startTimeSpinner->setIntValue(_exporter->startFrame());
	_startTimeSpinner->setTextBox(new QLineEdit());
	_startTimeSpinner->setMinValue(dataset->animationSettings()->animationInterval().start() / dataset->animationSettings()->ticksPerFrame());
	_startTimeSpinner->setMaxValue(dataset->animationSettings()->animationInterval().end() / dataset->animationSettings()->ticksPerFrame());
	frameRangeLayout->addWidget(_startTimeSpinner->textBox());
	frameRangeLayout->addWidget(_startTimeSpinner);
	frameRangeLayout->addSpacing(8);
	frameRangeLayout->addWidget(new QLabel(tr("To:")));
	_endTimeSpinner = new SpinnerWidget();
	_endTimeSpinner->setUnit(UnitsManager::instance().integerIdentityUnit());
	_endTimeSpinner->setIntValue(_exporter->endFrame());
	_endTimeSpinner->setTextBox(new QLineEdit());
	_endTimeSpinner->setMinValue(dataset->animationSettings()->animationInterval().start() / dataset->animationSettings()->ticksPerFrame());
	_endTimeSpinner->setMaxValue(dataset->animationSettings()->animationInterval().end() / dataset->animationSettings()->ticksPerFrame());
	frameRangeLayout->addWidget(_endTimeSpinner->textBox());
	frameRangeLayout->addWidget(_endTimeSpinner);
	frameRangeLayout->addSpacing(8);
	frameRangeLayout->addWidget(new QLabel(tr("Every Nth frame:")));
	_nthFrameSpinner = new SpinnerWidget();
	_nthFrameSpinner->setUnit(UnitsManager::instance().integerIdentityUnit());
	_nthFrameSpinner->setIntValue(_exporter->everyNthFrame());
	_nthFrameSpinner->setTextBox(new QLineEdit());
	_nthFrameSpinner->setMinValue(1);
	frameRangeLayout->addWidget(_nthFrameSpinner->textBox());
	frameRangeLayout->addWidget(_nthFrameSpinner);

	_startTimeSpinner->setEnabled(radioBtn->isChecked());
	_endTimeSpinner->setEnabled(radioBtn->isChecked());
	_nthFrameSpinner->setEnabled(radioBtn->isChecked());
	connect(radioBtn, SIGNAL(toggled(bool)), _startTimeSpinner, SLOT(setEnabled(bool)));
	connect(radioBtn, SIGNAL(toggled(bool)), _endTimeSpinner, SLOT(setEnabled(bool)));
	connect(radioBtn, SIGNAL(toggled(bool)), _nthFrameSpinner, SLOT(setEnabled(bool)));

	QGroupBox* fileGroupBox = new QGroupBox(tr("Output"), this);
	layout1->addWidget(fileGroupBox);

	QGridLayout* fileGroupLayout = new QGridLayout(fileGroupBox);
	fileGroupLayout->setColumnStretch(0, 5);
	fileGroupLayout->setColumnStretch(1, 95);
	_fileGroupButtonGroup = new QButtonGroup(this);

	radioBtn = new QRadioButton(tr("Single file"));
	_fileGroupButtonGroup->addButton(radioBtn, 0);
	fileGroupLayout->addWidget(radioBtn, 0, 0, 1, 2);
	radioBtn->setChecked(!_exporter->useWildcardFilename());

	radioBtn = new QRadioButton(tr("Multiple files using wild-card pattern:"));
	_fileGroupButtonGroup->addButton(radioBtn, 1);
	fileGroupLayout->addWidget(radioBtn, 1, 0, 1, 2);
	radioBtn->setChecked(_exporter->useWildcardFilename());

	_wildcardTextbox = new QLineEdit(_exporter->wildcardFilename(), fileGroupBox);
	fileGroupLayout->addWidget(_wildcardTextbox, 2, 1, 1, 1);
	_wildcardTextbox->setEnabled(radioBtn->isChecked());
	connect(radioBtn, SIGNAL(toggled(bool)), _wildcardTextbox, SLOT(setEnabled(bool)));

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* This is called when the user has pressed the OK button.
******************************************************************************/
void ParticleExporterSettingsDialog::onOk()
{
	try {

		_exporter->setExportAnimation(_rangeButtonGroup->checkedId() == 1);
		_exporter->setUseWildcardFilename(_fileGroupButtonGroup->checkedId() == 1);
		_exporter->setWildcardFilename(_wildcardTextbox->text());
		_exporter->setStartFrame(_startTimeSpinner->intValue());
		_exporter->setEndFrame(std::max(_endTimeSpinner->intValue(), _startTimeSpinner->intValue()));
		_exporter->setEveryNthFrame(_nthFrameSpinner->intValue());

		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

};	// End of namespace
