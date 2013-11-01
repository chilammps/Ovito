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
#include <core/gui/mainwin/MainWindow.h>
#include "InputColumnMappingDialog.h"

namespace Viz {

enum {
	FILE_COLUMN_COLUMN = 0,
	PROPERTY_COLUMN,
	VECTOR_COMPNT_COLUMN
};

/******************************************************************************
* Constructor.
******************************************************************************/
InputColumnMappingDialog::InputColumnMappingDialog(const InputColumnMapping& mapping, QWidget* parent)
	: QDialog(parent ? parent : &MainWindow::instance())
{
	setWindowTitle(tr("File column mapping"));

	_vectorCmpntSignalMapper = new QSignalMapper(this);
	connect(_vectorCmpntSignalMapper, SIGNAL(mapped(int)), this, SLOT(updateVectorComponentList(int)));

	// Create the table sub-widget.
	QVBoxLayout* layout = new QVBoxLayout(this);

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
	layout->addLayout(tableWidgetLayout);

	_tableWidget->setColumnCount(3);
	QStringList horizontalHeaders;
	horizontalHeaders << tr("File column");
	horizontalHeaders << tr("Particle property");
	horizontalHeaders << tr("Component");
	_tableWidget->setHorizontalHeaderLabels(horizontalHeaders);
	_tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);

	_tableWidget->resizeColumnToContents(VECTOR_COMPNT_COLUMN);

	// Calculate the optimum with of the property column.
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
	layout->addStretch(1);

	// Ok and Cancel buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
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

	_tableWidget->setRowCount(mapping.columnCount());
	for(int i = 0; i < mapping.columnCount(); i++) {
		QCheckBox* fileColumnItem = new QCheckBox();
		if(mapping.columnName(i).isEmpty())
			fileColumnItem->setText(tr("Column %1").arg(i+1));
		else
			fileColumnItem->setText(mapping.columnName(i));
		fileColumnItem->setChecked(mapping.isMapped(i));
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
		nameItem->setCurrentText(mapping.propertyName(i));
		nameItem->setEnabled(mapping.isMapped(i));
		_tableWidget->setCellWidget(i, PROPERTY_COLUMN, nameItem);
		_propertyBoxes.push_back(nameItem);

		QComboBox* vectorComponentItem = new QComboBox();
		_tableWidget->setCellWidget(i, VECTOR_COMPNT_COLUMN, vectorComponentItem);
		_vectorComponentBoxes.push_back(vectorComponentItem);
		updateVectorComponentList(i);
		if(vectorComponentItem->count() != 0)
			vectorComponentItem->setCurrentIndex(mapping.vectorComponent(i));

		connect(fileColumnItem, SIGNAL(clicked(bool)), nameItem, SLOT(setEnabled(bool)));
		_vectorCmpntSignalMapper->setMapping(fileColumnItem, i);
		_vectorCmpntSignalMapper->setMapping(nameItem, i);
		connect(fileColumnItem, SIGNAL(clicked(bool)), _vectorCmpntSignalMapper, SLOT(map()));
		connect(nameItem, SIGNAL(currentTextChanged(const QString&)), _vectorCmpntSignalMapper, SLOT(map()));
	}

	_tableWidget->resizeRowsToContents();
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
	mapping.setColumnCount(_tableWidget->rowCount());
	for(int index = 0; index < mapping.columnCount(); index++) {
		if(_fileColumnBoxes[index]->isChecked()) {
			QString propertyName = _propertyBoxes[index]->currentText().trimmed();
			ParticleProperty::Type type = ParticleProperty::standardPropertyList().value(propertyName);
			if(type != ParticleProperty::UserProperty) {
				int vectorCompnt = std::max(0, _vectorComponentBoxes[index]->currentIndex());
				mapping.mapStandardColumn(index, type, vectorCompnt, _fileColumnBoxes[index]->text());
				continue;
			}
			else if(!propertyName.isEmpty()) {
				mapping.mapCustomColumn(index, propertyName, qMetaTypeId<FloatType>(), 0, ParticleProperty::UserProperty, _fileColumnBoxes[index]->text());
				continue;
			}
		}
		mapping.unmapColumn(index, _fileColumnBoxes[index]->text());
	}
	return mapping;
}

};	// End of namespace
