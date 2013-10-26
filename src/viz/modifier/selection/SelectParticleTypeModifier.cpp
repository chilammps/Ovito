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
#include <core/gui/undo/UndoManager.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <viz/util/ParticlePropertyComboBox.h>
#include <viz/data/ParticleTypeProperty.h>
#include "SelectParticleTypeModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, SelectParticleTypeModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, SelectParticleTypeModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(SelectParticleTypeModifier, SelectParticleTypeModifierEditor)

/******************************************************************************
* Sets the identifier of the data channel that contains the type for each atom.
******************************************************************************/
void SelectParticleTypeModifier::setSourceProperty(const ParticlePropertyReference& prop)
{
	if(_inputPropertyRef == prop) return;

	// Make this change undoable.
	qRegisterMetaType<ParticlePropertyReference>();
	if(UndoManager::instance().isRecording())
		UndoManager::instance().push(new SimplePropertyChangeOperation(this, "sourceProperty"));

	_inputPropertyRef = prop;
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Sets the list of atom type identifiers to select.
******************************************************************************/
void SelectParticleTypeModifier::setSelectedParticleTypes(const QSet<int>& types)
{
	if(_selectedParticleTypes == types)
		return;		// Nothing has changed

	class SelectParticleTypesOperation : public UndoableOperation {
	public:
		SelectParticleTypesOperation(SelectParticleTypeModifier* _mod) : mod(_mod), oldTypes(_mod->selectedParticleTypes()) {}
		virtual void undo() override {
			QSet<int> temp = mod->selectedParticleTypes();
			mod->setSelectedParticleTypes(oldTypes);
			oldTypes = temp;
		}
		virtual void redo() override { undo(); }
		virtual QString displayName() const override { return "Select Atom Type"; }
	private:
		OORef<SelectParticleTypeModifier> mod;
		QSet<int> oldTypes;
	};

	if(UndoManager::instance().isRecording())
		UndoManager::instance().push(new SelectParticleTypesOperation(this));

	_selectedParticleTypes = types;
	notifyDependents(ReferenceEvent::TargetChanged);
}


/******************************************************************************
* Retrieves the input type property from the given modifier input state.
******************************************************************************/
ParticleTypeProperty* SelectParticleTypeModifier::lookupInputProperty(const PipelineFlowState& inputState) const
{
	for(const auto& o : inputState.objects()) {
		ParticleTypeProperty* ptypeProp = dynamic_object_cast<ParticleTypeProperty>(o.get());
		if(ptypeProp) {
			if((sourceProperty().type() == ParticleProperty::UserProperty && ptypeProp->name() == sourceProperty().name()) ||
					(sourceProperty().type() != ParticleProperty::UserProperty && ptypeProp->type() == sourceProperty().type())) {
				return ptypeProp;
			}
		}
	}
	return nullptr;
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus SelectParticleTypeModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the input type property.
	ParticleTypeProperty* typeProperty = lookupInputProperty(input());
	if(!typeProperty)
		throw Exception(tr("The source property for this modifier is not present in the input."));
	OVITO_ASSERT(typeProperty->componentCount() == 1);
	OVITO_ASSERT(typeProperty->dataType() == qMetaTypeId<int>());

	// Get the deep copy of the selection property.
	ParticlePropertyObject* selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);

	// The number of selected particles.
	size_t nSelected = 0;

	OVITO_ASSERT(selProperty->size() == typeProperty->size());
	const int* t = typeProperty->constDataInt();
	for(int& s : selProperty->intRange()) {
		if(selectedParticleTypes().contains(*t++)) {
			s = 1;
			nSelected++;
		}
		else s = 0;
	}
	selProperty->changed();

	QString statusMessage = tr("%1 out of %2 particles selected (%3%)").arg(nSelected).arg(inputParticleCount()).arg(nSelected * 100 / std::max((int)inputParticleCount(), 1));
	return ObjectStatus(ObjectStatus::Success, statusMessage);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void SelectParticleTypeModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Select the first particle type property from the input with more than one particle type.
	PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
	ParticleTypeProperty* bestProperty = nullptr;
	for(const auto& o : input.objects()) {
		ParticleTypeProperty* ptypeProp = dynamic_object_cast<ParticleTypeProperty>(o.get());
		if(ptypeProp && ptypeProp->particleTypes().empty() == false && ptypeProp->componentCount() == 1) {
			bestProperty = ptypeProp;
		}
	}
	if(bestProperty)
		setSourceProperty(bestProperty);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void SelectParticleTypeModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);

	stream.beginChunk(0x01);
	stream << _inputPropertyRef;
	stream << _selectedParticleTypes;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void SelectParticleTypeModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream >> _inputPropertyRef;
	stream >> _selectedParticleTypes;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> SelectParticleTypeModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<SelectParticleTypeModifier> clone = static_object_cast<SelectParticleTypeModifier>(ParticleModifier::clone(deepCopy, cloneHelper));

	clone->_inputPropertyRef = this->_inputPropertyRef;
	clone->_selectedParticleTypes = this->_selectedParticleTypes;

	return clone;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SelectParticleTypeModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	QWidget* rollout = createRollout(tr("Select particle type"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	propertyListBox = new ParticlePropertyComboBox();
	layout->addWidget(new QLabel(tr("Property:"), rollout));
	layout->addWidget(propertyListBox);

	class MyListWidget : public QListWidget {
	public:
		MyListWidget() : QListWidget() {}
		virtual QSize sizeHint() const { return QSize(256, 192); }
	};
	particleTypesBox = new MyListWidget();
	particleTypesBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
	layout->addWidget(new QLabel(tr("Types:"), rollout));
	layout->addWidget(particleTypesBox);

	// Update property list if another modifier has been loaded into the editor.
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(updatePropertyList()));

	// Status label.
	layout->addSpacing(12);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* Updates the contents of the combo box.
******************************************************************************/
void SelectParticleTypeModifierEditor::updatePropertyList()
{
	disconnect(propertyListBox, SIGNAL(activated(int)), this, SLOT(onPropertySelected(int)));
	propertyListBox->clear();

	SelectParticleTypeModifier* mod = static_object_cast<SelectParticleTypeModifier>(editObject());
	if(!mod) {
		propertyListBox->setEnabled(false);
	}
	else {
		propertyListBox->setEnabled(true);

		// Populate type property list based on modifier input.
		PipelineFlowState inputState = mod->getModifierInput();
		for(const auto& o : inputState.objects()) {
			ParticleTypeProperty* ptypeProp = dynamic_object_cast<ParticleTypeProperty>(o.get());
			if(ptypeProp && ptypeProp->particleTypes().empty() == false && ptypeProp->componentCount() == 1) {
				propertyListBox->addItem(ptypeProp);
			}
		}

		propertyListBox->setCurrentProperty(mod->sourceProperty());
	}
	connect(propertyListBox, SIGNAL(activated(int)), this, SLOT(onPropertySelected(int)));

	updateParticleTypeList();
}

/******************************************************************************
* Updates the contents of the list box.
******************************************************************************/
void SelectParticleTypeModifierEditor::updateParticleTypeList()
{
	disconnect(particleTypesBox, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onParticleTypeSelected(QListWidgetItem*)));
	particleTypesBox->setUpdatesEnabled(false);
	particleTypesBox->clear();

	SelectParticleTypeModifier* mod = static_object_cast<SelectParticleTypeModifier>(editObject());
	if(!mod) {
		particleTypesBox->setEnabled(false);
	}
	else {
		particleTypesBox->setEnabled(true);

		// Populate atom types list based on the input type property.
		ParticleTypeProperty* inputProperty = mod->lookupInputProperty(mod->getModifierInput());
		if(inputProperty) {
			for(ParticleType* ptype : inputProperty->particleTypes()) {
				if(!ptype) continue;
				QListWidgetItem* item = new QListWidgetItem(ptype->name(), particleTypesBox);
				item->setData(Qt::UserRole, ptype->id());
				item->setData(Qt::DecorationRole, (QColor)ptype->color());
				if(mod->selectedParticleTypes().contains(ptype->id()))
					item->setCheckState(Qt::Checked);
				else
					item->setCheckState(Qt::Unchecked);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren);
			}
		}
	}

	connect(particleTypesBox, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onParticleTypeSelected(QListWidgetItem*)));
	particleTypesBox->setUpdatesEnabled(true);
}

/******************************************************************************
* This is called when the user has selected a new item in the property list.
******************************************************************************/
void SelectParticleTypeModifierEditor::onPropertySelected(int index)
{
	SelectParticleTypeModifier* mod = static_object_cast<SelectParticleTypeModifier>(editObject());
	if(!mod) return;

	UndoableTransaction::handleExceptions(tr("Select property"), [this, mod]() {
		mod->setSourceProperty(propertyListBox->currentProperty());
	});
}

/******************************************************************************
* This is called when the user has selected another atom type.
******************************************************************************/
void SelectParticleTypeModifierEditor::onParticleTypeSelected(QListWidgetItem* item)
{
	SelectParticleTypeModifier* mod = static_object_cast<SelectParticleTypeModifier>(editObject());
	if(!mod) return;

	QSet<int> types = mod->selectedParticleTypes();
	if(item->checkState() == Qt::Checked)
		types.insert(item->data(Qt::UserRole).toInt());
	else
		types.remove(item->data(Qt::UserRole).toInt());

	UndoableTransaction::handleExceptions(tr("Select type"), [mod, &types]() {
		mod->setSelectedParticleTypes(types);
	});
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool SelectParticleTypeModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		updatePropertyList();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

};	// End of namespace
