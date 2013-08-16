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
	: QDialog(parent)
{
	setWindowTitle(tr("Input file column mapping"));

	_vectorCmpntSignalMapper = new QSignalMapper(this);
	connect(_vectorCmpntSignalMapper, SIGNAL(mapped(int)), this, SLOT(updateVectorComponentList(int)));

	// Create the table sub-widget.
	QVBoxLayout* layout = new QVBoxLayout(this);

	QLabel* captionLabel = new QLabel(
			tr("Please specify how the data columns of the input file should be mapped "
				"to Ovito's particle properties."));
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
	horizontalHeaders << tr("File columns");
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

#if 0
	class NameItemDelegate : public QItemDelegate {
	public:
		virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
			QComboBox* box = new QComboBox(parent);
			box->setEditable(true);
			box->setDuplicatesEnabled(false);
			QMapIterator<QString, ParticleProperty::Type> i(ParticleProperty::standardPropertyList());
			while(i.hasNext()) {
				i.next();
				box->addItem(i.key(), i.value());
			}
			return box;
		}
		virtual void setEditorData(QWidget* editor, const QModelIndex& index) const {
			int value = index.model()->data(index, Qt::UserRole).toInt();
			QComboBox* box = static_cast<QComboBox*>(editor);
			if(value < 0)
				box->setCurrentIndex(box->findData(value));
			else
				box->setEditText(index.model()->data(index).toString());
		}
		virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
			QComboBox* box = static_cast<QComboBox*>(editor);
			QString newValue = box->currentText().trimmed();
			model->setData(index, newValue);
			ParticleProperty::Type id = ParticleProperty::standardPropertyList().value(newValue);
			if(id != ParticleProperty::UserProperty) {
				model->setData(index, id, Qt::UserRole);
				int newType = ParticleProperty::standardPropertyDataType(id);
				model->setData(index.sibling(index.row(), 0), dataTypeToString(newType));
				model->setData(index.sibling(index.row(), 0), newType, Qt::UserRole);
				if(newType == QMetaType::Void) {
					model->setData(index.sibling(index.row(), 2), 0, Qt::UserRole);
					model->setData(index.sibling(index.row(), 2), "");
				}
				else {
					int vectorComponent = std::min(index.sibling(index.row(), 2).data(Qt::UserRole).toInt(), (int)ParticleProperty::standardPropertyComponentCount(id)-1);
					QString componentName;
					if(ParticleProperty::standardPropertyComponentNames(id).size() > vectorComponent) componentName = ParticleProperty::standardPropertyComponentNames(id)[vectorComponent];
					model->setData(index.sibling(index.row(), 2), vectorComponent, Qt::UserRole);
					model->setData(index.sibling(index.row(), 2), componentName);
				}
			}
			else {
				model->setData(index, 0, Qt::UserRole);
				if(newValue.isEmpty()) {
					model->setData(index.sibling(index.row(), 0), dataTypeToString(QMetaType::Void));
					model->setData(index.sibling(index.row(), 0), (int)QMetaType::Void, Qt::UserRole);
					model->setData(index.sibling(index.row(), 2), 0, Qt::UserRole);
					model->setData(index.sibling(index.row(), 2), "");
				}
			}
		}
	};
	static NameItemDelegate nameItemDelegate;

	class DataTypeItemDelegate : public QItemDelegate {
	public:
		virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
			QComboBox* box = new QComboBox(parent);
			box->addItem(dataTypeToString(QMetaType::Void), (int)QMetaType::Void);
			box->addItem(dataTypeToString(qMetaTypeId<int>()), qMetaTypeId<int>());
			box->addItem(dataTypeToString(qMetaTypeId<FloatType>()), qMetaTypeId<FloatType>());
			return box;
		}
		virtual void setEditorData(QWidget* editor, const QModelIndex& index) const {
			int value = index.model()->data(index, Qt::UserRole).toInt();
			QComboBox* box = static_cast<QComboBox*>(editor);
			box->setCurrentIndex(box->findData(value));
		}
		virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
			QComboBox* box = static_cast<QComboBox*>(editor);
			int newType = box->itemData(box->currentIndex()).toInt();
			model->setData(index, newType, Qt::UserRole);
			model->setData(index, dataTypeToString(newType));
			if(newType == QMetaType::Void) {
				model->setData(index.sibling(index.row(), 2), 0, Qt::UserRole);
				model->setData(index.sibling(index.row(), 2), "");
			}
			ParticleProperty::Type standardPropertyType = (ParticleProperty::Type)index.sibling(index.row(), 1).data(Qt::UserRole).toInt();
			if(standardPropertyType != ParticleProperty::UserProperty) {
				if(ParticleProperty::standardPropertyDataType(standardPropertyType) != newType) {
					model->setData(index.sibling(index.row(), 1), 0, Qt::UserRole);
					model->setData(index.sibling(index.row(), 1), QString());
					model->setData(index.sibling(index.row(), 3), 0, Qt::UserRole);
					model->setData(index.sibling(index.row(), 3), "");
				}
			}
		}
	};
	static DataTypeItemDelegate dataTypeItemDelegate;



	class VectorComponentItemDelegate : public QItemDelegate {
	public:
		virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
			QComboBox* box = new QComboBox(parent);
			return box;
		}
		virtual void setEditorData(QWidget* editor, const QModelIndex& index) const {
			int value = index.model()->data(index, Qt::UserRole).toInt();
			OVITO_ASSERT(value >= 0);
			QComboBox* box = static_cast<QComboBox*>(editor);
			int dataType = index.sibling(index.row(), 0).data(Qt::UserRole).toInt();
			ParticleProperty::Type dataChannelId = (ParticleProperty::Type)index.sibling(index.row(), 1).data(Qt::UserRole).toInt();
			box->clear();
			if(dataType != QMetaType::Void) {
				if(dataChannelId != ParticleProperty::UserProperty) {
					box->setEditable(false);
					Q_FOREACH(QString name, ParticleProperty::standardPropertyComponentNames(dataChannelId))
						box->addItem(name);
				}
				else {
					box->setEditable(true);
					for(int i=1; i<10; i++)
						box->addItem(QString::number(i));
				}
				box->setCurrentIndex(value);
				box->setEnabled(box->count() > 0);
			}
			else box->setEnabled(false);
		}
		virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
			QComboBox* box = static_cast<QComboBox*>(editor);
			if(box->currentIndex() >= 0) {
				model->setData(index, box->currentIndex(), Qt::UserRole);
				model->setData(index, box->currentText());
			}
			else {
				model->setData(index, 0, Qt::UserRole);
				model->setData(index, "");
			}
		}
	};
	static VectorComponentItemDelegate vectorComponentItemDelegate;

	_tableWidget->setItemDelegateForColumn(1, &nameItemDelegate);
	_tableWidget->setItemDelegateForColumn(2, &vectorComponentItemDelegate);
	_tableWidget->setItemDelegateForColumn(3, &dataTypeItemDelegate);
#endif

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

#if 0
		OVITO_ASSERT(mapping.vectorComponent(i) >= 0);
		int vectorComponent = mapping.vectorComponent(i);
		QString componentName;
		if(mapping.propertyType(i) != ParticleProperty::UserProperty) {
			ParticleProperty::Type stdId = mapping.propertyType(i);
			vectorComponent = std::min(vectorComponent, (int)ParticleProperty::standardPropertyComponentCount(stdId)-1);
			if(ParticleProperty::standardPropertyComponentNames(stdId).size() > vectorComponent)
				componentName = ParticleProperty::standardPropertyComponentNames(stdId)[vectorComponent];
		}
		else if(mapping.dataType(i) != QMetaType::Void) componentName = QString::number(vectorComponent+1);

		QTableWidgetItem* vectorComponentItem = new QTableWidgetItem(componentName);
		vectorComponentItem->setData(Qt::UserRole, vectorComponent);
		vectorComponentItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		_tableWidget->setItem(i, VECTOR_COMPNT_COLUMN, vectorComponentItem);
		_tableWidget->openPersistentEditor(vectorComponentItem);

		QTableWidgetItem* dataTypeItem = new QTableWidgetItem(dataTypeToString(mapping.dataType(i)));
		dataTypeItem->setData(Qt::UserRole, mapping.dataType(i));
		dataTypeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		_tableWidget->setItem(i, DATA_TYPE_COLUMN, dataTypeItem);
		_tableWidget->openPersistentEditor(dataTypeItem);
#endif
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
