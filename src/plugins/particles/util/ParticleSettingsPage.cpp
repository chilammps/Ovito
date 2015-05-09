///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include "ParticleSettingsPage.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Particles, ParticleSettingsPage, ApplicationSettingsDialogPage);

class NameColumnDelegate : public QStyledItemDelegate
{
public:
	NameColumnDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override { return nullptr; }
};

class RadiusColumnDelegate : public QStyledItemDelegate
{
public:
	RadiusColumnDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    	if(!index.model()->data(index, Qt::EditRole).isValid())
    		return nullptr;
		QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
		editor->setFrame(false);
		editor->setMinimum(0);
		editor->setSingleStep(0.1);
		return editor;
	}

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    	double value = index.model()->data(index, Qt::EditRole).toDouble();
    	QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
		spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    	QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
    	spinBox->interpretText();
    	double value = spinBox->value();
    	model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    	editor->setGeometry(option.rect);
    }

    QString displayText(const QVariant& value, const QLocale& locale) const override {
    	if(value.isValid())
    		return QString::number(value.toDouble());
    	else
    		return QString();
    }
};

class ColorColumnDelegate : public QStyledItemDelegate
{
public:
	ColorColumnDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    	QColor oldColor = index.model()->data(index, Qt::EditRole).value<QColor>();
    	QString ptypeName = index.sibling(index.row(), 0).data().toString();
    	QColor newColor = QColorDialog::getColor(oldColor, parent->window(), tr("Select color for '%1'").arg(ptypeName));
    	if(newColor.isValid()) {
    		const_cast<QAbstractItemModel*>(index.model())->setData(index, QVariant::fromValue(newColor), Qt::EditRole);
    	}
		return nullptr;
	}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    	QBrush brush(index.model()->data(index, Qt::EditRole).value<QColor>());
    	painter->fillRect(option.rect, brush);
    }
};

/******************************************************************************
* Creates the widget that contains the plugin specific setting controls.
******************************************************************************/
void ParticleSettingsPage::insertSettingsDialogPage(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	QWidget* page = new QWidget();
	tabWidget->addTab(page, tr("Particles"));
	QVBoxLayout* layout1 = new QVBoxLayout(page);
	layout1->setSpacing(0);

	_particleTypesItem = new QTreeWidgetItem(QStringList() << tr("Particle types") << QString() << QString());
	_particleTypesItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	_structureTypesItem = new QTreeWidgetItem(QStringList() << tr("Structure types") << QString() << QString());
	_structureTypesItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	QStringList typeNames;
	for(int i = 0; i < ParticleTypeProperty::PredefinedParticleType::NUMBER_OF_PREDEFINED_PARTICLE_TYPES; i++)
		typeNames << ParticleTypeProperty::getPredefinedParticleTypeName((ParticleTypeProperty::PredefinedParticleType)i);
	QSettings settings;
	settings.beginGroup("particles/defaults/color");
	settings.beginGroup(QString::number((int)ParticleProperty::ParticleTypeProperty));
	typeNames.append(settings.childKeys());
	settings.endGroup();
	settings.endGroup();
	settings.beginGroup("particles/defaults/radius");
	settings.beginGroup(QString::number((int)ParticleProperty::ParticleTypeProperty));
	typeNames.append(settings.childKeys());
	settings.endGroup();
	settings.endGroup();
	typeNames.removeDuplicates();

	for(const QString& tname : typeNames) {
		QTreeWidgetItem* childItem = new QTreeWidgetItem();
		childItem->setText(0, tname);
		Color color = ParticleTypeProperty::getDefaultParticleColor(ParticleProperty::ParticleTypeProperty, tname, 0);
		FloatType radius = ParticleTypeProperty::getDefaultParticleRadius(ParticleProperty::ParticleTypeProperty, tname, 0);
		childItem->setData(1, Qt::DisplayRole, QVariant::fromValue((QColor)color));
		childItem->setData(2, Qt::DisplayRole, QVariant::fromValue(radius));
		childItem->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren));
		_particleTypesItem->addChild(childItem);
	}

	QStringList structureNames;
	for(int i = 0; i < ParticleTypeProperty::PredefinedStructureType::NUMBER_OF_PREDEFINED_STRUCTURE_TYPES; i++)
		structureNames << ParticleTypeProperty::getPredefinedStructureTypeName((ParticleTypeProperty::PredefinedStructureType)i);
	settings.beginGroup("particles/defaults/color");
	settings.beginGroup(QString::number((int)ParticleProperty::StructureTypeProperty));
	structureNames.append(settings.childKeys());
	structureNames.removeDuplicates();

	for(const QString& tname : structureNames) {
		QTreeWidgetItem* childItem = new QTreeWidgetItem();
		childItem->setText(0, tname);
		Color color = ParticleTypeProperty::getDefaultParticleColor(ParticleProperty::StructureTypeProperty, tname, 0);
		childItem->setData(1, Qt::DisplayRole, QVariant::fromValue((QColor)color));
		childItem->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren));
		_structureTypesItem->addChild(childItem);
	}

	layout1->addWidget(new QLabel(tr("Default particle colors and sizes:")));
	_predefTypesTable = new QTreeWidget();
	layout1->addWidget(_predefTypesTable, 1);
	_predefTypesTable->setColumnCount(3);
	_predefTypesTable->setHeaderLabels(QStringList() << tr("Type") << tr("Color") << tr("Radius"));
	_predefTypesTable->setRootIsDecorated(true);
	_predefTypesTable->setAllColumnsShowFocus(true);
	_predefTypesTable->addTopLevelItem(_particleTypesItem);
	_predefTypesTable->setFirstItemColumnSpanned(_particleTypesItem, true);
	_predefTypesTable->addTopLevelItem(_structureTypesItem);
	_predefTypesTable->setFirstItemColumnSpanned(_structureTypesItem, true);
	_predefTypesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	_predefTypesTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
	_predefTypesTable->setColumnWidth(0, 280);

	NameColumnDelegate* nameDelegate = new NameColumnDelegate(this);
	_predefTypesTable->setItemDelegateForColumn(0, nameDelegate);
	ColorColumnDelegate* colorDelegate = new ColorColumnDelegate(this);
	_predefTypesTable->setItemDelegateForColumn(1, colorDelegate);
	RadiusColumnDelegate* radiusDelegate = new RadiusColumnDelegate(this);
	_predefTypesTable->setItemDelegateForColumn(2, radiusDelegate);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	buttonLayout->setContentsMargins(0,0,0,0);
	QPushButton* restoreBuiltinDefaultsButton = new QPushButton(tr("Restore built-in defaults"));
	buttonLayout->addStretch(1);
	buttonLayout->addWidget(restoreBuiltinDefaultsButton);
	connect(restoreBuiltinDefaultsButton, &QPushButton::clicked, this, &ParticleSettingsPage::restoreBuiltinParticlePresets);
	layout1->addLayout(buttonLayout);
}

/******************************************************************************
* Lets the page save all changed settings.
******************************************************************************/
bool ParticleSettingsPage::saveValues(ApplicationSettingsDialog* settingsDialog, QTabWidget* tabWidget)
{
	// Clear all existing user-defined settings first.
	QSettings settings;
	settings.beginGroup("particles/defaults/color");
	settings.beginGroup(QString::number((int)ParticleProperty::ParticleTypeProperty));
	settings.remove(QString());
	settings.endGroup();
	settings.endGroup();
	settings.beginGroup("particles/defaults/radius");
	settings.beginGroup(QString::number((int)ParticleProperty::ParticleTypeProperty));
	settings.remove(QString());

	for(int i = 0; i < _particleTypesItem->childCount(); i++) {
		QTreeWidgetItem* item = _particleTypesItem->child(i);
		QColor color = item->data(1, Qt::DisplayRole).value<QColor>();
		FloatType radius = item->data(2, Qt::DisplayRole).value<FloatType>();
		ParticleTypeProperty::setDefaultParticleColor(ParticleProperty::ParticleTypeProperty, item->text(0), color);
		ParticleTypeProperty::setDefaultParticleRadius(ParticleProperty::ParticleTypeProperty, item->text(0), radius);
	}

	for(int i = 0; i < _structureTypesItem->childCount(); i++) {
		QTreeWidgetItem* item = _structureTypesItem->child(i);
		QColor color = item->data(1, Qt::DisplayRole).value<QColor>();
		ParticleTypeProperty::setDefaultParticleColor(ParticleProperty::StructureTypeProperty, item->text(0), color);
	}

	return true;
}

/******************************************************************************
* Restores the built-in default particle colors and sizes.
******************************************************************************/
void ParticleSettingsPage::restoreBuiltinParticlePresets()
{
	for(int i = 0; i < ParticleTypeProperty::PredefinedParticleType::NUMBER_OF_PREDEFINED_PARTICLE_TYPES; i++) {
		QTreeWidgetItem* item = _particleTypesItem->child(i);
		Color color = ParticleTypeProperty::getDefaultParticleColor(ParticleProperty::ParticleTypeProperty, item->text(0), 0, false);
		FloatType radius = ParticleTypeProperty::getDefaultParticleRadius(ParticleProperty::ParticleTypeProperty, item->text(0), 0, false);
		item->setData(1, Qt::DisplayRole, QVariant::fromValue((QColor)color));
		item->setData(2, Qt::DisplayRole, QVariant::fromValue(radius));
	}
	for(int i = _particleTypesItem->childCount() - 1; i >= ParticleTypeProperty::PredefinedParticleType::NUMBER_OF_PREDEFINED_PARTICLE_TYPES; i--) {
		delete _particleTypesItem->takeChild(i);
	}

	for(int i = 0; i < ParticleTypeProperty::PredefinedStructureType::NUMBER_OF_PREDEFINED_STRUCTURE_TYPES; i++) {
		QTreeWidgetItem* item = _structureTypesItem->child(i);
		Color color = ParticleTypeProperty::getDefaultParticleColor(ParticleProperty::StructureTypeProperty, item->text(0), 0, false);
		item->setData(1, Qt::DisplayRole, QVariant::fromValue((QColor)color));
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
