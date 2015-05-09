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

// Define default names, colors, and radii for some predefined particle types.
std::array<ParticleTypeProperty::PredefinedTypeInfo, ParticleTypeProperty::NUMBER_OF_PREDEFINED_PARTICLE_TYPES> ParticleTypeProperty::_predefinedParticleTypes{{
	ParticleTypeProperty::PredefinedTypeInfo{ QString("H"), Color(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f), 0.46f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("He"), Color(217.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f), 1.22f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Li"), Color(204.0f/255.0f, 128.0f/255.0f, 255.0f/255.0f), 1.57f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("C"), Color(144.0f/255.0f, 144.0f/255.0f, 144.0f/255.0f), 0.77f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("N"), Color(48.0f/255.0f, 80.0f/255.0f, 248.0f/255.0f), 0.74f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("O"), Color(255.0f/255.0f, 13.0f/255.0f, 13.0f/255.0f), 0.74f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Na"), Color(171.0f/255.0f, 92.0f/255.0f, 242.0f/255.0f), 1.91f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Mg"), Color(138.0f/255.0f, 255.0f/255.0f, 0.0f/255.0f), 1.60f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Al"), Color(191.0f/255.0f, 166.0f/255.0f, 166.0f/255.0f), 1.43f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Si"), Color(240.0f/255.0f, 200.0f/255.0f, 160.0f/255.0f), 1.18f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("K"), Color(143.0f/255.0f, 64.0f/255.0f, 212.0f/255.0f), 2.35f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Ca"), Color(61.0f/255.0f, 255.0f/255.0f, 0.0f/255.0f), 1.97f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Ti"), Color(191.0f/255.0f, 194.0f/255.0f, 199.0f/255.0f), 1.47f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Cr"), Color(138.0f/255.0f, 153.0f/255.0f, 199.0f/255.0f), 1.29f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Fe"), Color(224.0f/255.0f, 102.0f/255.0f, 51.0f/255.0f), 1.26f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Co"), Color(240.0f/255.0f, 144.0f/255.0f, 160.0f/255.0f), 1.25f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Ni"), Color(80.0f/255.0f, 208.0f/255.0f, 80.0f/255.0f), 1.25f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Cu"), Color(200.0f/255.0f, 128.0f/255.0f, 51.0f/255.0f), 1.28f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Zn"), Color(125.0f/255.0f, 128.0f/255.0f, 176.0f/255.0f), 1.37f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Ga"), Color(194.0f/255.0f, 143.0f/255.0f, 143.0f/255.0f), 1.53f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Ge"), Color(102.0f/255.0f, 143.0f/255.0f, 143.0f/255.0f), 1.22f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Kr"), Color(92.0f/255.0f, 184.0f/255.0f, 209.0f/255.0f), 1.98f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Sr"), Color(0.0f, 1.0f, 0.15259f), 2.15f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Y"), Color(0.40259f, 0.59739f, 0.55813), 1.82f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Zr"), Color(0.0f, 1.0f, 0.0f), 1.60f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Nb"), Color(0.29992f, 0.7f, 0.46459f), 1.47f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Pd"), Color(0.0f/255.0f, 105.0f/255.0f, 133.0f/255.0f), 1.37f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Pt"), Color(0.79997f, 0.77511f, 0.75068f), 1.39f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("W"), Color(0.55616f, 0.54257f, 0.50178f), 1.41f },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Au"), Color(255.0f/255.0f, 209.0f/255.0f, 35.0f/255.0f), 1.44f }
}};

// Define default names, colors, and radii for predefined structure types.
std::array<ParticleTypeProperty::PredefinedTypeInfo, ParticleTypeProperty::NUMBER_OF_PREDEFINED_STRUCTURE_TYPES> ParticleTypeProperty::_predefinedStructureTypes{{
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Other"), Color(0.95f, 0.95f, 0.95f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("FCC"), Color(0.4f, 1.0f, 0.4f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("HCP"), Color(1.0f, 0.4f, 0.4f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("BCC"), Color(0.4f, 0.4f, 1.0f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("ICO"), Color(0.95f, 0.8f, 0.2f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Cubic diamond"), Color(19.0f/255.0f, 160.0f/255.0f, 254.0f/255.0f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Cubic diamond (1st neighbor)"), Color(0.0f/255.0f, 254.0f/255.0f, 245.0f/255.0f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Cubic diamond (2nd neighbor)"), Color(126.0f/255.0f, 254.0f/255.0f, 181.0f/255.0f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Hexagonal diamond"), Color(254.0f/255.0f, 137.0f/255.0f, 0.0f/255.0f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Hexagonal diamond (1st neighbor)"), Color(254.0f/255.0f, 220.0f/255.0f, 0.0f/255.0f), 0 },
	ParticleTypeProperty::PredefinedTypeInfo{ QString("Hexagonal diamond (2nd neighbor)"), Color(204.0f/255.0f, 229.0f/255.0f, 81.0f/255.0f), 0 }
}};

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
Color ParticleTypeProperty::getDefaultParticleColorFromId(ParticleProperty::Type typeClass, int particleTypeId)
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
Color ParticleTypeProperty::getDefaultParticleColor(ParticleProperty::Type typeClass, const QString& particleTypeName, int particleTypeId, bool userDefaults)
{
	if(userDefaults) {
		QSettings settings;
		settings.beginGroup("particles/defaults/color");
		settings.beginGroup(QString::number((int)typeClass));
		QVariant v = settings.value(particleTypeName);
		if(v.isValid() && v.canConvert<Color>())
			return v.value<Color>();
	}

	if(typeClass == ParticleProperty::StructureTypeProperty) {
		for(const PredefinedTypeInfo& predefType : _predefinedStructureTypes) {
			if(std::get<0>(predefType) == particleTypeName)
				return std::get<1>(predefType);
		}
		return Color(1,1,1);
	}
	else {
		for(const PredefinedTypeInfo& predefType : _predefinedParticleTypes) {
			if(std::get<0>(predefType) == particleTypeName)
				return std::get<1>(predefType);
		}
		return getDefaultParticleColorFromId(typeClass, particleTypeId);
	}
}

/******************************************************************************
* Changes the default color for a particle type name.
******************************************************************************/
void ParticleTypeProperty::setDefaultParticleColor(ParticleProperty::Type typeClass, const QString& particleTypeName, const Color& color)
{
	QSettings settings;
	settings.beginGroup("particles/defaults/color");
	settings.beginGroup(QString::number((int)typeClass));

	if(getDefaultParticleColor(typeClass, particleTypeName, 0, false) != color)
		settings.setValue(particleTypeName, QVariant::fromValue(color));
	else
		settings.remove(particleTypeName);
}

/******************************************************************************
* Returns the default radius for a particle type name.
******************************************************************************/
FloatType ParticleTypeProperty::getDefaultParticleRadius(ParticleProperty::Type typeClass, const QString& particleTypeName, int particleTypeId, bool userDefaults)
{
	if(userDefaults) {
		QSettings settings;
		settings.beginGroup("particles/defaults/radius");
		settings.beginGroup(QString::number((int)typeClass));
		QVariant v = settings.value(particleTypeName);
		if(v.isValid() && v.canConvert<FloatType>())
			return v.value<FloatType>();
	}

	if(typeClass == ParticleProperty::ParticleTypeProperty) {
		for(const PredefinedTypeInfo& predefType : _predefinedParticleTypes) {
			if(std::get<0>(predefType) == particleTypeName)
				return std::get<2>(predefType);
		}
	}
	return 0.0f;
}

/******************************************************************************
* Changes the default radius for a particle type name.
******************************************************************************/
void ParticleTypeProperty::setDefaultParticleRadius(ParticleProperty::Type typeClass, const QString& particleTypeName, FloatType radius)
{
	QSettings settings;
	settings.beginGroup("particles/defaults/radius");
	settings.beginGroup(QString::number((int)typeClass));

	if(getDefaultParticleRadius(typeClass, particleTypeName, 0, false) != radius)
		settings.setValue(particleTypeName, QVariant::fromValue(radius));
	else
		settings.remove(particleTypeName);
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
