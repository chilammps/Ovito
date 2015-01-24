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

#include <plugins/particles/Particles.h>
#include <core/animation/AnimationSettings.h>
#include "ParticleExporterSettingsDialog.h"
#include "OutputColumnMapping.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export)

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleExporterSettingsDialog::ParticleExporterSettingsDialog(QWidget* parent, ParticleExporter* exporter, const PipelineFlowState& state, OutputColumnMapping* columnMapping)
	: QDialog(parent), _exporter(exporter), _columnMapping(columnMapping)
{
	setWindowTitle(tr("Export Settings"));

	_mainLayout = new QVBoxLayout(this);
	QRadioButton* radioBtn;

	QGroupBox* rangeGroupBox = new QGroupBox(tr("Export frame sequence"), this);
	_mainLayout->addWidget(rangeGroupBox);

	QGridLayout* rangeGroupLayout = new QGridLayout(rangeGroupBox);
	rangeGroupLayout->setColumnStretch(0, 5);
	rangeGroupLayout->setColumnStretch(1, 95);
	_rangeButtonGroup = new QButtonGroup(this);

	bool exportAnim = _exporter->exportAnimation();
	radioBtn = new QRadioButton(tr("Single frame"));
	_rangeButtonGroup->addButton(radioBtn, 0);
	rangeGroupLayout->addWidget(radioBtn, 0, 0, 1, 2);
	radioBtn->setChecked(!exportAnim);

	radioBtn = new QRadioButton(tr("Animation range"));
	_rangeButtonGroup->addButton(radioBtn, 1);
	rangeGroupLayout->addWidget(radioBtn, 1, 0, 1, 2);
	radioBtn->setChecked(exportAnim);
	radioBtn->setEnabled(exporter->dataset()->animationSettings()->animationInterval().duration() != 0);

	QHBoxLayout* frameRangeLayout = new QHBoxLayout();
	rangeGroupLayout->addLayout(frameRangeLayout, 2, 1, 1, 1);

	frameRangeLayout->setSpacing(0);
	frameRangeLayout->addWidget(new QLabel(tr("From:")));
	_startTimeSpinner = new SpinnerWidget();
	_startTimeSpinner->setUnit(exporter->dataset()->unitsManager().timeUnit());
	_startTimeSpinner->setIntValue(exporter->dataset()->animationSettings()->frameToTime(_exporter->startFrame()));
	_startTimeSpinner->setTextBox(new QLineEdit());
	_startTimeSpinner->setMinValue(exporter->dataset()->animationSettings()->animationInterval().start());
	_startTimeSpinner->setMaxValue(exporter->dataset()->animationSettings()->animationInterval().end());
	frameRangeLayout->addWidget(_startTimeSpinner->textBox());
	frameRangeLayout->addWidget(_startTimeSpinner);
	frameRangeLayout->addSpacing(8);
	frameRangeLayout->addWidget(new QLabel(tr("To:")));
	_endTimeSpinner = new SpinnerWidget();
	_endTimeSpinner->setUnit(exporter->dataset()->unitsManager().timeUnit());
	_endTimeSpinner->setIntValue(exporter->dataset()->animationSettings()->frameToTime(_exporter->endFrame()));
	_endTimeSpinner->setTextBox(new QLineEdit());
	_endTimeSpinner->setMinValue(exporter->dataset()->animationSettings()->animationInterval().start());
	_endTimeSpinner->setMaxValue(exporter->dataset()->animationSettings()->animationInterval().end());
	frameRangeLayout->addWidget(_endTimeSpinner->textBox());
	frameRangeLayout->addWidget(_endTimeSpinner);
	frameRangeLayout->addSpacing(8);
	frameRangeLayout->addWidget(new QLabel(tr("Every Nth frame:")));
	_nthFrameSpinner = new SpinnerWidget();
	_nthFrameSpinner->setUnit(exporter->dataset()->unitsManager().integerIdentityUnit());
	_nthFrameSpinner->setIntValue(_exporter->everyNthFrame());
	_nthFrameSpinner->setTextBox(new QLineEdit());
	_nthFrameSpinner->setMinValue(1);
	frameRangeLayout->addWidget(_nthFrameSpinner->textBox());
	frameRangeLayout->addWidget(_nthFrameSpinner);

	_startTimeSpinner->setEnabled(radioBtn->isChecked());
	_endTimeSpinner->setEnabled(radioBtn->isChecked());
	_nthFrameSpinner->setEnabled(radioBtn->isChecked());
	connect(radioBtn, &QRadioButton::toggled, _startTimeSpinner, &SpinnerWidget::setEnabled);
	connect(radioBtn, &QRadioButton::toggled, _endTimeSpinner, &SpinnerWidget::setEnabled);
	connect(radioBtn, &QRadioButton::toggled, _nthFrameSpinner, &SpinnerWidget::setEnabled);

	QGroupBox* fileGroupBox = new QGroupBox(tr("Output"), this);
	_mainLayout->addWidget(fileGroupBox);

	QGridLayout* fileGroupLayout = new QGridLayout(fileGroupBox);
	fileGroupLayout->setColumnStretch(0, 5);
	fileGroupLayout->setColumnStretch(1, 95);
	_fileGroupButtonGroup = new QButtonGroup(this);

	radioBtn = new QRadioButton(tr("Single file"));
	_fileGroupButtonGroup->addButton(radioBtn, 0);
	fileGroupLayout->addWidget(radioBtn, 0, 0, 1, 2);
	radioBtn->setChecked(!_exporter->useWildcardFilename());

	radioBtn = new QRadioButton(tr("Multiple files (wild-card pattern):"));
	_fileGroupButtonGroup->addButton(radioBtn, 1);
	fileGroupLayout->addWidget(radioBtn, 1, 0, 1, 2);
	radioBtn->setChecked(_exporter->useWildcardFilename());

	_wildcardTextbox = new QLineEdit(_exporter->wildcardFilename(), fileGroupBox);
	fileGroupLayout->addWidget(_wildcardTextbox, 2, 1, 1, 1);
	_wildcardTextbox->setEnabled(radioBtn->isChecked());
	connect(radioBtn, &QRadioButton::toggled, _wildcardTextbox, &QLineEdit::setEnabled);

	if(columnMapping) {
		QGroupBox* columnsGroupBox = new QGroupBox(tr("Particle properties"), this);
		_mainLayout->addWidget(columnsGroupBox);
		QGridLayout* columnsGroupBoxLayout = new QGridLayout(columnsGroupBox);

		_columnMappingWidget = new QListWidget();
		columnsGroupBoxLayout->addWidget(_columnMappingWidget, 0, 0, 5, 1);
		columnsGroupBoxLayout->setRowStretch(2, 1);

		bool hasParticleIdentifiers = false;
		for(DataObject* o : state.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
			if(!property) continue;
			if(property->componentCount() == 1) {
				insertPropertyItem(ParticlePropertyReference(property), property->name());
				if(property->type() == ParticleProperty::IdentifierProperty)
					hasParticleIdentifiers = true;
			}
			else {
				for(int vectorComponent = 0; vectorComponent < (int)property->componentCount(); vectorComponent++) {
					QString propertyName = property->nameWithComponent(vectorComponent);
					ParticlePropertyReference propRef(property, vectorComponent);
					insertPropertyItem(propRef, propertyName);
				}
			}
		}
		if(!hasParticleIdentifiers)
			insertPropertyItem(ParticleProperty::IdentifierProperty, tr("Particle index"));

		QPushButton* moveUpButton = new QPushButton(tr("Move up"), columnsGroupBox);
		QPushButton* moveDownButton = new QPushButton(tr("Move down"), columnsGroupBox);
		QPushButton* selectAllButton = new QPushButton(tr("Select all"), columnsGroupBox);
		QPushButton* selectNoneButton = new QPushButton(tr("Unselect all"), columnsGroupBox);
		columnsGroupBoxLayout->addWidget(moveUpButton, 0, 1, 1, 1);
		columnsGroupBoxLayout->addWidget(moveDownButton, 1, 1, 1, 1);
		columnsGroupBoxLayout->addWidget(selectAllButton, 3, 1, 1, 1);
		columnsGroupBoxLayout->addWidget(selectNoneButton, 4, 1, 1, 1);
		moveUpButton->setEnabled(_columnMappingWidget->currentRow() >= 1);
		moveDownButton->setEnabled(_columnMappingWidget->currentRow() >= 0 && _columnMappingWidget->currentRow() < _columnMappingWidget->count() - 1);

		connect(_columnMappingWidget, &QListWidget::itemSelectionChanged, [moveUpButton, moveDownButton, this]() {
			moveUpButton->setEnabled(_columnMappingWidget->currentRow() >= 1);
			moveDownButton->setEnabled(_columnMappingWidget->currentRow() >= 0 && _columnMappingWidget->currentRow() < _columnMappingWidget->count() - 1);
		});

		connect(moveUpButton, &QPushButton::clicked, [this]() {
			int currentIndex = _columnMappingWidget->currentRow();
			QListWidgetItem* currentItem = _columnMappingWidget->takeItem(currentIndex);
			_columnMappingWidget->insertItem(currentIndex - 1, currentItem);
			_columnMappingWidget->setCurrentRow(currentIndex - 1);
		});

		connect(moveDownButton, &QPushButton::clicked, [this]() {
			int currentIndex = _columnMappingWidget->currentRow();
			QListWidgetItem* currentItem = _columnMappingWidget->takeItem(currentIndex);
			_columnMappingWidget->insertItem(currentIndex + 1, currentItem);
			_columnMappingWidget->setCurrentRow(currentIndex + 1);
		});

		connect(selectAllButton, &QPushButton::clicked, [this]() {
			for(int index = 0; index < _columnMappingWidget->count(); index++)
				_columnMappingWidget->item(index)->setCheckState(Qt::Checked);
		});

		connect(selectNoneButton, &QPushButton::clicked, [this]() {
			for(int index = 0; index < _columnMappingWidget->count(); index++)
				_columnMappingWidget->item(index)->setCheckState(Qt::Unchecked);
		});
	}

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &ParticleExporterSettingsDialog::onOk);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &ParticleExporterSettingsDialog::reject);
	_mainLayout->addWidget(buttonBox);
}

/******************************************************************************
* Populates the column mapping list box with an entry.
******************************************************************************/
void ParticleExporterSettingsDialog::insertPropertyItem(ParticlePropertyReference propRef, const QString& displayName)
{
	QListWidgetItem* item = new QListWidgetItem(displayName);
	item->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren));
	item->setCheckState(Qt::Unchecked);
	item->setData(Qt::UserRole, qVariantFromValue(propRef));
	int sortKey = _columnMapping->size();

	for(int c = 0; c < (int)_columnMapping->size(); c++) {
		if((*_columnMapping)[c] == propRef) {
			item->setCheckState(Qt::Checked);
			sortKey = c;
			break;
		}
	}

	item->setData(Qt::InitialSortOrderRole, sortKey);
	if(sortKey < (int)_columnMapping->size()) {
		int insertIndex = 0;
		for(; insertIndex < _columnMappingWidget->count(); insertIndex++) {
			int k = _columnMappingWidget->item(insertIndex)->data(Qt::InitialSortOrderRole).value<int>();
			if(sortKey < k)
				break;
		}
		_columnMappingWidget->insertItem(insertIndex, item);
	}
	else {
		_columnMappingWidget->addItem(item);
	}
}

/******************************************************************************
* Extends the dialog by inserting an additional widget (usually a QGroupBox) into the layout.
******************************************************************************/
void ParticleExporterSettingsDialog::insertWidget(QWidget* widget)
{
	_mainLayout->insertWidget(_mainLayout->count() - 1, widget);
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
		_exporter->setStartFrame(_exporter->dataset()->animationSettings()->timeToFrame(_startTimeSpinner->intValue()));
		_exporter->setEndFrame(_exporter->dataset()->animationSettings()->timeToFrame(std::max(_endTimeSpinner->intValue(), _startTimeSpinner->intValue())));
		_exporter->setEveryNthFrame(_nthFrameSpinner->intValue());

		if(_columnMapping) {
			OutputColumnMapping newMapping;
			for(int index = 0; index < _columnMappingWidget->count(); index++) {
				if(_columnMappingWidget->item(index)->checkState() == Qt::Checked) {
					newMapping.push_back(_columnMappingWidget->item(index)->data(Qt::UserRole).value<ParticlePropertyReference>());
				}
			}
			*_columnMapping = newMapping;
		}

		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
