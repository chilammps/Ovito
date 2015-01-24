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
#include <core/plugins/PluginManager.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/ObjectNode.h>
#include <core/dataset/DataSetContainer.h>
#include <core/animation/AnimationSettings.h>
#include "ModifierListBox.h"
#include "ModificationListModel.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Initializes the widget.
******************************************************************************/
ModifierListBox::ModifierListBox(QWidget* parent, ModificationListModel* modificationList) : QComboBox(parent),
		_modificationList(modificationList)
{
	ModifierCategory otherCategory;
	otherCategory.name = tr("Others");

	// Retrieve all installed modifier classes.
	Q_FOREACH(const OvitoObjectType* clazz, PluginManager::instance().listClasses(Modifier::OOType)) {
		// Sort modifiers into categories.
		if(clazz->qtMetaObject()) {
			int infoIndex = clazz->qtMetaObject()->indexOfClassInfo("ModifierCategory");
			if(infoIndex != -1) {
				QString categoryName = QString::fromLocal8Bit(clazz->qtMetaObject()->classInfo(infoIndex).value());
				// Check if category has already been created.
				bool found = false;
				for(auto& category : _modifierCategories) {
					if(category.name == categoryName) {
						category.modifierClasses.push_back(clazz);
						found = true;
						break;
					}
				}
				// Create a new category.
				if(!found) {
					ModifierCategory category;
					category.name = categoryName;
					category.modifierClasses.push_back(clazz);
					_modifierCategories.push_back(category);
				}
				continue;
			}
		}

		// Insert modifiers that don't have category information to the "Other" category.
		otherCategory.modifierClasses.push_back(clazz);
	}

	// Sort category list alphabetically.
	std::sort(_modifierCategories.begin(), _modifierCategories.end(), [](const ModifierCategory& a, const ModifierCategory& b) {
		return QString::compare(a.name, b.name, Qt::CaseInsensitive) < 0;
	} );

	// Assign modifiers that haven't been assigned to a category yet to the "Other" category.
	if(!otherCategory.modifierClasses.isEmpty())
		_modifierCategories.push_back(otherCategory);

	// Sort modifier sub-lists alphabetically.
	for(auto& category : _modifierCategories) {
		std::sort(category.modifierClasses.begin(), category.modifierClasses.end(), [](const OvitoObjectType* a, const OvitoObjectType* b) {
			return QString::compare(a->name(), b->name(), Qt::CaseInsensitive) < 0;
		} );
	}

	_categoryFont = font();
	_categoryFont.setBold(true);
	if(_categoryFont.pixelSize() < 0)
		_categoryFont.setPointSize(_categoryFont.pointSize() * 4 / 5);
	else
		_categoryFont.setPixelSize(_categoryFont.pixelSize() * 4 / 5);
	_categoryBackgroundBrush = QBrush(Qt::lightGray, Qt::Dense4Pattern);
	_categoryForegroundBrush = QBrush(Qt::blue);

	updateAvailableModifiers();
}

/******************************************************************************
* Updates the list box of modifier classes that can be applied to the current selected
* item in the modification list.
******************************************************************************/
void ModifierListBox::updateAvailableModifiers()
{
	clear();

	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(this->model());
	OVITO_CHECK_POINTER(model);
	QStandardItem* titleItem = new QStandardItem(tr("Add modification..."));
	titleItem->setFlags(Qt::ItemIsEnabled);
	model->appendRow(titleItem);
	setCurrentIndex(0);

	ModificationListItem* currentItem = _modificationList->selectedItem();
	if(currentItem == nullptr) {
		return;
	}

	// Retrieve the input state which a newly inserted modifier would be applied to.
	// This is used to filter the list of available modifiers.
	PipelineFlowState inputState;
	DataSet* dataset = _modificationList->datasetContainer().currentSet();
	if(dynamic_object_cast<Modifier>(currentItem->object())) {
		for(ModifierApplication* modApp : currentItem->modifierApplications()) {
			PipelineObject* pipelineObj = modApp->pipelineObject();
			OVITO_CHECK_OBJECT_POINTER(pipelineObj);
			inputState = pipelineObj->evaluatePipeline(dataset->animationSettings()->time(), modApp, true);
			break;
		}
	}
	else if(dynamic_object_cast<DataObject>(currentItem->object())) {
		DataObject* dataObj = static_object_cast<DataObject>(currentItem->object());
		OVITO_CHECK_OBJECT_POINTER(dataObj);
		inputState = dataObj->evaluate(dataset->animationSettings()->time());
	}
	else {
		for(RefTarget* objNode : _modificationList->selectedNodes()) {
			inputState = static_object_cast<ObjectNode>(objNode)->evalPipeline(dataset->animationSettings()->time());
			break;
		}
	}

	for(const ModifierCategory& category : _modifierCategories) {
		QList<QStandardItem*> categoryItems;
		for(const OvitoObjectType* descriptor : category.modifierClasses) {
			// Create an instance of the modifier to call its isApplicableTo() method.
			OORef<Modifier> modifier = static_object_cast<Modifier>(descriptor->createInstance(dataset));
			OVITO_CHECK_OBJECT_POINTER(modifier);
			if(modifier && modifier->isApplicableTo(inputState)) {
				QStandardItem* modifierItem = new QStandardItem("   " + descriptor->displayName());
				modifierItem->setData(qVariantFromValue((void*)descriptor), Qt::UserRole);
				categoryItems.push_back(modifierItem);
			}
		}

		if(!categoryItems.empty()) {
			QStandardItem* categoryItem = new QStandardItem(category.name);
			categoryItem->setFont(_categoryFont);
			categoryItem->setBackground(_categoryBackgroundBrush);
			categoryItem->setForeground(_categoryForegroundBrush);
			categoryItem->setFlags(Qt::ItemIsEnabled);
			categoryItem->setTextAlignment(Qt::AlignCenter);
			model->appendRow(categoryItem);
			for(QStandardItem* item : categoryItems)
				model->appendRow(item);
		}
	}

	if(count() <= 1) {
		QStandardItem* categoryItem = new QStandardItem(tr("No modifiers applicable to this object"));
		categoryItem->setFont(_categoryFont);
		categoryItem->setBackground(_categoryBackgroundBrush);
		categoryItem->setForeground(_categoryForegroundBrush);
		categoryItem->setFlags(Qt::ItemIsEnabled);
		categoryItem->setTextAlignment(Qt::AlignCenter);
		model->appendRow(categoryItem);
	}

    setMaxVisibleItems(count());
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
