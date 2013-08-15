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

/******************************************************************************
* Constructor.
******************************************************************************/
InputColumnMappingDialog::InputColumnMappingDialog(const InputColumnMapping& mapping, QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Input file column mapping"));

	// Create the table sub-widget.
	QVBoxLayout* layout = new QVBoxLayout(this);

	QGridLayout* tableWidgetLayout = new QGridLayout();
	_tableWidget = new QTableWidget(this);
	tableWidgetLayout->addWidget(_tableWidget, 0, 0);
	tableWidgetLayout->setRowMinimumHeight(0, 250);
	tableWidgetLayout->setRowStretch(0, 1);
	tableWidgetLayout->setColumnMinimumWidth(0, 450);
	tableWidgetLayout->setColumnStretch(0, 1);
	layout->addLayout(tableWidgetLayout);

	_tableWidget->setColumnCount(4);
	QStringList horizontalHeaders;
	horizontalHeaders << tr("File column");
	horizontalHeaders << tr("Particle property");
	horizontalHeaders << tr("Component");
	horizontalHeaders << tr("Data type");
	_tableWidget->setHorizontalHeaderLabels(horizontalHeaders);
	_tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);

#if 0
	_tableWidget->resizeColumnToContents(0);
	_tableWidget->resizeColumnToContents(2);

	// Calculate the optimum with of the 2. column.
	QComboBox* box = new QComboBox();
	box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	QMapIterator<QString, ParticleProperty::Type> i(ParticleProperty::standardPropertyList());
	while(i.hasNext()) {
		i.next();
		box->addItem(i.key(), i.value());
	}
	_tableWidget->setColumnWidth(1, box->sizeHint().width());
#endif

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
					model->setData(index.sibling(index.row(), 2), 0, Qt::UserRole);
					model->setData(index.sibling(index.row(), 2), "");
				}
			}
		}
	};
	static DataTypeItemDelegate dataTypeItemDelegate;

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
	_tableWidget->setItemDelegateForColumn(3, &dataTypeItemDelegate);
	_tableWidget->setItemDelegateForColumn(2, &vectorComponentItemDelegate);
	layout->addStretch(1);

	// Ok and Cancel buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout->addWidget(buttonBox);
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
		// Close dialog box.
		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}
}

};	// End of namespace
