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
#include "InputColumnMappingDialog.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import)

enum {
	FILE_COLUMN_COLUMN = 0,
	PROPERTY_COLUMN,
	VECTOR_COMPNT_COLUMN
};

/******************************************************************************
* Constructor.
******************************************************************************/
InputColumnMappingDialog::InputColumnMappingDialog(const InputColumnMapping& mapping, QWidget* parent) : QDialog(parent)
{
	OVITO_CHECK_POINTER(parent);
	setWindowTitle(tr("File column mapping"));

	_vectorCmpntSignalMapper = new QSignalMapper(this);
	connect(_vectorCmpntSignalMapper, (void (QSignalMapper::*)(int))&QSignalMapper::mapped, this, &InputColumnMappingDialog::updateVectorComponentList);

	// Create the table sub-widget.
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(2);

	QLabel* captionLabel = new QLabel(
			tr("Please specify how the data columns of the input file should be mapped "
				"to OVITO's particle properties."));
	captionLabel->setWordWrap(true);
	layout->addWidget(captionLabel);
	layout->addSpacing(10);

	QGridLayout* tableWidgetLayout = new QGridLayout();
	_tableWidget = new QTableWidget(this);
	tableWidgetLayout->addWidget(_tableWidget, 0, 0);
	tableWidgetLayout->setRowMinimumHeight(0, 250);
	tableWidgetLayout->setRowStretch(0, 1);
	tableWidgetLayout->setColumnMinimumWidth(0, 450);
	tableWidgetLayout->setColumnStretch(0, 1);
	layout->addLayout(tableWidgetLayout, 4);

	_tableWidget->setColumnCount(3);
	QStringList horizontalHeaders;
	horizontalHeaders << tr("File column");
	horizontalHeaders << tr("Particle property");
	horizontalHeaders << tr("Component");
	_tableWidget->setHorizontalHeaderLabels(horizontalHeaders);
	_tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);

	_tableWidget->resizeColumnToContents(VECTOR_COMPNT_COLUMN);

	// Calculate the optimum width of the property column.
	QComboBox* box = new QComboBox();
	box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	QMapIterator<QString, ParticleProperty::Type> i(ParticleProperty::standardPropertyList());
	while(i.hasNext()) {
		i.next();
		box->addItem(i.key(), i.value());
	}
	_tableWidget->setColumnWidth(PROPERTY_COLUMN, box->sizeHint().width());
	_tableWidget->verticalHeader()->setVisible(false);
	_tableWidget->setShowGrid(false);

	layout->addSpacing(6);
	layout->addWidget(_fileExcerptLabel = new QLabel(tr("File excerpt:")));
	_fileExcerptLabel->setVisible(false);
	_fileExcerptField = new QTextEdit();
	_fileExcerptField->setLineWrapMode(QTextEdit::NoWrap);
	_fileExcerptField->setAcceptRichText(false);
	_fileExcerptField->setReadOnly(true);
	_fileExcerptField->setVisible(false);
	layout->addWidget(_fileExcerptField, 1);
	layout->addSpacing(10);

	// Dialog buttons:
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox->addButton(tr("Load preset..."), QDialogButtonBox::ActionRole), &QPushButton::clicked, this, &InputColumnMappingDialog::onLoadPreset);
	connect(buttonBox->addButton(tr("Save preset..."), QDialogButtonBox::ActionRole), &QPushButton::clicked, this, &InputColumnMappingDialog::onSavePreset);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &InputColumnMappingDialog::onOk);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &InputColumnMappingDialog::reject);
	layout->addWidget(buttonBox);

	setMapping(mapping);
}

/******************************************************************************
 * Returns the string representation of a data channel type.
 *****************************************************************************/
QString InputColumnMappingDialog::dataTypeToString(int dataType)
{
	if(dataType == qMetaTypeId<int>()) return tr("Integer");
	else if(dataType == qMetaTypeId<FloatType>()) return tr("Float");
	else return tr("None");
}

/******************************************************************************
* This is called when the user has pressed the OK button.
******************************************************************************/
void InputColumnMappingDialog::onOk()
{
	try {
		// First validate the current mapping.
		mapping().validate();

		// Close dialog box.
		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}
}

/******************************************************************************
 * Fills the editor with the given mapping.
 *****************************************************************************/
void InputColumnMappingDialog::setMapping(const InputColumnMapping& mapping)
{
	_tableWidget->clearContents();
	_fileColumnBoxes.clear();
	_propertyBoxes.clear();
	_vectorComponentBoxes.clear();
	_propertyDataTypes.clear();

	_tableWidget->setRowCount(mapping.size());
	for(int i = 0; i < (int)mapping.size(); i++) {
		QCheckBox* fileColumnItem = new QCheckBox();
		if(mapping[i].columnName.isEmpty())
			fileColumnItem->setText(tr("Column %1").arg(i+1));
		else
			fileColumnItem->setText(mapping[i].columnName);
		fileColumnItem->setChecked(mapping[i].isMapped());
		_tableWidget->setCellWidget(i, FILE_COLUMN_COLUMN, fileColumnItem);
		_fileColumnBoxes.push_back(fileColumnItem);

		QComboBox* nameItem = new QComboBox();
		nameItem->setEditable(true);
		nameItem->setDuplicatesEnabled(false);
		QMapIterator<QString, ParticleProperty::Type> propIter(ParticleProperty::standardPropertyList());
		while(propIter.hasNext()) {
			propIter.next();
			nameItem->addItem(propIter.key(), propIter.value());
		}
		nameItem->setCurrentText(mapping[i].property.name());
		nameItem->setEnabled(mapping[i].isMapped());
		_tableWidget->setCellWidget(i, PROPERTY_COLUMN, nameItem);
		_propertyBoxes.push_back(nameItem);

		QComboBox* vectorComponentItem = new QComboBox();
		_tableWidget->setCellWidget(i, VECTOR_COMPNT_COLUMN, vectorComponentItem);
		_vectorComponentBoxes.push_back(vectorComponentItem);
		updateVectorComponentList(i);
		if(vectorComponentItem->count() != 0)
			vectorComponentItem->setCurrentIndex(std::max(0,mapping[i].property.vectorComponent()));

		connect(fileColumnItem, &QCheckBox::clicked, nameItem, &QComboBox::setEnabled);
		_vectorCmpntSignalMapper->setMapping(fileColumnItem, i);
		_vectorCmpntSignalMapper->setMapping(nameItem, i);
		connect(fileColumnItem, &QCheckBox::clicked, _vectorCmpntSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);
		connect(nameItem, &QComboBox::currentTextChanged, _vectorCmpntSignalMapper, (void (QSignalMapper::*)())&QSignalMapper::map);

		_propertyDataTypes.push_back(mapping[i].dataType != QMetaType::Void ? mapping[i].dataType : qMetaTypeId<FloatType>());
	}

	_tableWidget->resizeRowsToContents();

	if(!mapping.fileExcerpt().isEmpty()) {
		_fileExcerptField->setPlainText(mapping.fileExcerpt());
		_fileExcerptField->setVisible(true);
		_fileExcerptLabel->setVisible(true);
	}
	else {
		_fileExcerptField->setVisible(false);
		_fileExcerptLabel->setVisible(false);
	}
}

/******************************************************************************
 * Updates the list of vector components for the given file column.
 *****************************************************************************/
void InputColumnMappingDialog::updateVectorComponentList(int columnIndex)
{
	OVITO_ASSERT(columnIndex < _vectorComponentBoxes.size());
	QComboBox* vecBox = _vectorComponentBoxes[columnIndex];

	QString propertyName = _propertyBoxes[columnIndex]->currentText();
	ParticleProperty::Type standardProperty = ParticleProperty::standardPropertyList().value(propertyName);
	if(standardProperty != ParticleProperty::UserProperty) {
		int oldIndex = vecBox->currentIndex();
		_vectorComponentBoxes[columnIndex]->clear();
		Q_FOREACH(QString name, ParticleProperty::standardPropertyComponentNames(standardProperty))
			vecBox->addItem(name);
		vecBox->setEnabled(_fileColumnBoxes[columnIndex]->isChecked() && vecBox->count() != 0);
		if(oldIndex >= 0)
			vecBox->setCurrentIndex(std::min(oldIndex, vecBox->count()-1));
	}
	else {
		vecBox->clear();
		vecBox->setEnabled(false);
	}
}

/******************************************************************************
 * Returns the current contents of the editor.
 *****************************************************************************/
InputColumnMapping InputColumnMappingDialog::mapping() const
{
	InputColumnMapping mapping;
	mapping.resize(_tableWidget->rowCount());
	for(int index = 0; index < (int)mapping.size(); index++) {
		mapping[index].columnName = _fileColumnBoxes[index]->text();
		if(_fileColumnBoxes[index]->isChecked()) {
			QString propertyName = _propertyBoxes[index]->currentText().trimmed();
			ParticleProperty::Type type = ParticleProperty::standardPropertyList().value(propertyName);
			if(type != ParticleProperty::UserProperty) {
				int vectorCompnt = std::max(0, _vectorComponentBoxes[index]->currentIndex());
				mapping[index].mapStandardColumn(type, vectorCompnt);
				continue;
			}
			else if(!propertyName.isEmpty()) {
				mapping[index].mapCustomColumn(propertyName, _propertyDataTypes[index]);
				continue;
			}
		}
	}
	if(!_fileExcerptField->isHidden()) {
		mapping.setFileExcerpt(_fileExcerptField->toPlainText());
	}
	return mapping;
}

/******************************************************************************
 * Saves the current mapping as a preset.
 *****************************************************************************/
void InputColumnMappingDialog::onSavePreset()
{
	try {
		// Get current mapping.
		InputColumnMapping m = mapping();

		// Load existing mappings.
		QSettings settings;
		settings.beginGroup("inputcolumnmapping");
		int size = settings.beginReadArray("presets");
		QStringList presetNames;
		QList<QByteArray> presetData;
		for(int i = 0; i < size; ++i) {
		    settings.setArrayIndex(i);
		    presetNames.push_back(settings.value("name").toString());
		    presetData.push_back(settings.value("data").toByteArray());
		}
		settings.endArray();

		// Let the user give a name.
		QString name = QInputDialog::getItem(this, tr("Save Column Mapping"),
			tr("Please enter a name for the column mapping:"), presetNames, -1, true);
		if(name.isEmpty()) return;

		// Serialize mapping and add it to the list.
		int index = presetNames.indexOf(name);
		if(index >= 0) {
			// Overwrite existing preset with the same name.
			presetData[index] = m.toByteArray();
		}
		else {
			// Add a new preset. Sort alphabetically.
			index = std::lower_bound(presetNames.begin(), presetNames.end(), name) - presetNames.begin();
			presetNames.insert(index, name);
			presetData.insert(index, m.toByteArray());
		}

		// Write mappings to settings store.
		settings.beginWriteArray("presets");
		for(int i = 0; i < presetNames.size(); ++i) {
		    settings.setArrayIndex(i);
		    settings.setValue("name", presetNames[i]);
		    settings.setValue("data", presetData[i]);
		}
		settings.endArray();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
 * Loads a preset mapping.
 *****************************************************************************/
void InputColumnMappingDialog::onLoadPreset()
{
	try {
		// Load list of presets.
		QSettings settings;
		settings.beginGroup("inputcolumnmapping");
		int size = settings.beginReadArray("presets");
		QStringList presetNames;
		QList<QByteArray> presetData;
		for(int i = 0; i < size; ++i) {
		    settings.setArrayIndex(i);
		    presetNames.push_back(settings.value("name").toString());
		    presetData.push_back(settings.value("data").toByteArray());
		}
		settings.endArray();

		if(size == 0)
			throw Exception(tr("There are no saved presets so far."));

		// Let the user pick a preset.
		QString name = QInputDialog::getItem(this, tr("Load Column Mapping"),
			tr("Select the column mapping to load:"), presetNames, 0, false);
		if(name.isEmpty()) return;

		// Load preset.
		InputColumnMapping mapping;
		mapping.fromByteArray(presetData[presetNames.indexOf(name)]);

		for(int index = 0; index < (int)mapping.size() && index < _tableWidget->rowCount(); index++) {
			_fileColumnBoxes[index]->setChecked(mapping[index].isMapped());
			_propertyBoxes[index]->setCurrentText(mapping[index].property.name());
			_propertyBoxes[index]->setEnabled(mapping[index].isMapped());
			updateVectorComponentList(index);
			if(_vectorComponentBoxes[index]->count() != 0)
				_vectorComponentBoxes[index]->setCurrentIndex(std::max(0,mapping[index].property.vectorComponent()));
		}
		for(int index = mapping.size(); index < _tableWidget->rowCount(); index++) {
			_fileColumnBoxes[index]->setChecked(false);
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
