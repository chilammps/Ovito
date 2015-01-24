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
#include <core/gui/properties/RefTargetListParameterUI.h>
#include "ParticleTypeProperty.h"

namespace Ovito { namespace Particles {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ParticleTypePropertyEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ParticleTypeProperty, ParticlePropertyObject);
SET_OVITO_OBJECT_EDITOR(ParticleTypeProperty, ParticleTypePropertyEditor);
DEFINE_VECTOR_REFERENCE_FIELD(ParticleTypeProperty, _particleTypes, "ParticleTypes", ParticleType);
SET_PROPERTY_FIELD_LABEL(ParticleTypeProperty, _particleTypes, "Particle Types");

/******************************************************************************
* Constructor.
******************************************************************************/
ParticleTypeProperty::ParticleTypeProperty(DataSet* dataset, ParticleProperty* storage)
	: ParticlePropertyObject(dataset, storage)
{
	INIT_PROPERTY_FIELD(ParticleTypeProperty::_particleTypes);
}

/******************************************************************************
* Inserts a particle type.
******************************************************************************/
void ParticleTypeProperty::insertParticleType(ParticleType* ptype)
{
	OVITO_ASSERT_MSG(dataType() == qMetaTypeId<int>(), "ParticleTypeProperty::insertParticleType()", "The particle property should be have the data type integer.");

	// Insert into array.
	_particleTypes.push_back(ptype);
}

/******************************************************************************
* Returns the default color for a particle type ID.
******************************************************************************/
Color ParticleTypeProperty::getDefaultParticleColorFromId(int particleTypeId)
{
	// Assign initial standard color to new particle types.
	static const Color defaultTypeColors[] = {
		Color(0.4f,1.0f,0.4f),
		Color(1.0f,0.4f,0.4f),
		Color(0.4f,0.4f,1.0f),
		Color(1.0f,1.0f,0.7f),
		Color(0.97f,0.97f,0.97f),
		Color(1.0f,1.0f,0.0f),
		Color(1.0f,0.4f,1.0f),
		Color(0.7f,0.0f,1.0f),
		Color(0.2f,1.0f,1.0f),
	};
	return defaultTypeColors[std::abs(particleTypeId) % (sizeof(defaultTypeColors) / sizeof(defaultTypeColors[0]))];
}

/******************************************************************************
* Returns the default color for a particle type name.
******************************************************************************/
Color ParticleTypeProperty::getDefaultParticleColorFromName(const QString& particleTypeName, int particleTypeId)
{
	return getDefaultParticleColorFromId(particleTypeId);
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ParticleTypePropertyEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(QString(), rolloutParams);

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

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace
