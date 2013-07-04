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
#include <core/gui/properties/RefTargetListParameterUI.h>
#include "ParticleTypeProperty.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticleTypeProperty, ParticlePropertyObject)
IMPLEMENT_OVITO_OBJECT(Viz, ParticleTypePropertyEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(ParticleTypeProperty, ParticleTypePropertyEditor)
DEFINE_VECTOR_REFERENCE_FIELD(ParticleTypeProperty, _particleTypes, "ParticleTypes", ParticleType)
SET_PROPERTY_FIELD_LABEL(ParticleTypeProperty, _particleTypes, "Particle Types")

/******************************************************************************
* Deserialization constructor.
******************************************************************************/
ParticleTypeProperty::ParticleTypeProperty()
{
	INIT_PROPERTY_FIELD(ParticleTypeProperty::_particleTypes);
}

/******************************************************************************
* Special constructor to create a standard data property.
******************************************************************************/
ParticleTypeProperty::ParticleTypeProperty(ParticleProperty::Type which) : ParticlePropertyObject(which)
{
	INIT_PROPERTY_FIELD(ParticleTypeProperty::_particleTypes);
}

/******************************************************************************
* Constructor that creates a property object from an existing property storage.
******************************************************************************/
ParticleTypeProperty::ParticleTypeProperty(ParticleProperty* storage)
	: ParticlePropertyObject(storage)
{
	INIT_PROPERTY_FIELD(ParticleTypeProperty::_particleTypes);
}

/******************************************************************************
* Inserts a particle type.
******************************************************************************/
void ParticleTypeProperty::insertParticleType(const OORef<ParticleType>& ptype)
{
	OVITO_ASSERT_MSG(dataType() == qMetaTypeId<int>(), "ParticleTypeProperty::insertParticleType()", "The particle property should be have the data type integer.");

	// Insert into array.
	_particleTypes.push_back(ptype);
}

/******************************************************************************
* Removes a particle type from this object.
******************************************************************************/
void ParticleTypeProperty::removeParticleType(int index)
{
	_particleTypes.remove(index);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ParticleTypePropertyEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Particle types"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);

	// Atom types

	// Derive a custom class from the list parameter UI to display the particle type colors.
	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:
		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField, const RolloutInsertionParameters& rolloutParams)
			: RefTargetListParameterUI(parentEditor, refField, rolloutParams, &ParticleTypeEditor::OOType) {}
	protected:
		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			if(role == Qt::DecorationRole && target != NULL) {
				return (QColor)static_object_cast<ParticleType>(target)->color();
			}
			else return RefTargetListParameterUI::getItemData(target, index, role);
		}
	};

	QWidget* subEditorContainer = new QWidget(rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(subEditorContainer);
	sublayout->setContentsMargins(0,0,0,0);
	layout->addWidget(subEditorContainer);

	RefTargetListParameterUI* particleTypesListUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(ParticleTypeProperty::_particleTypes), RolloutInsertionParameters().insertInto(subEditorContainer));
	layout->insertWidget(0, particleTypesListUI->listWidget());
}

};	// End of namespace
