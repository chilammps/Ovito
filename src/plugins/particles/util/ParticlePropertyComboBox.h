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

#ifndef __OVITO_PARTICLE_PROPERTY_COMBO_BOX_H
#define __OVITO_PARTICLE_PROPERTY_COMBO_BOX_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util)

/**
 * \brief Widget that allows the user to select a particle property from a list.
 */
class OVITO_PARTICLES_EXPORT ParticlePropertyComboBox : public QComboBox
{
	Q_OBJECT

public:

	/// \brief Default constructor.
	/// \param parent The parent widget of the combo box.
	ParticlePropertyComboBox(QWidget* parent = nullptr) : QComboBox(parent) {}

	/// \brief Adds a particle property to the end of the list.
	/// \param property The particle property to be added.
	void addItem(const ParticlePropertyReference& property, const QString& label = QString()) {
		QComboBox::addItem(label.isEmpty() ? property.name() : label, qVariantFromValue(property));
	}

	/// \brief Adds a particle property to the end of the list.
	/// \param property The particle property to be added.
	void addItem(ParticlePropertyObject* property, int vectorComponent = -1) {
		QString label = property->nameWithComponent(vectorComponent);
		ParticlePropertyReference propRef(property, vectorComponent);
		QComboBox::addItem(label, qVariantFromValue(propRef));
	}

	/// \brief Adds multiple particle properties to the combo box.
	/// \param list The list of particle properties to be added.
	void addItems(const QVector<ParticlePropertyObject*>& list) {
		for(ParticlePropertyObject* p : list)
			addItem(p);
	}

	/// \brief Returns the particle property that is currently selected in the
	///        combo box.
	/// \return The selected item. The returned reference can be null if
	///         no item is currently selected.
	ParticlePropertyReference currentProperty() const;

	/// \brief Returns the list index of the given property, or -1 if not found.
	int propertyIndex(const ParticlePropertyReference& property) const {
		for(int index = 0; index < count(); index++) {
			if(property == itemData(index).value<ParticlePropertyReference>())
				return index;
		}
		return -1;
	}

	/// \brief Returns the property at the given index.
	ParticlePropertyReference property(int index) const {
		return itemData(index).value<ParticlePropertyReference>();
	}

	/// \brief Sets the selection of the combo box to the given particle property.
	/// \param property The particle property to be selected.
	void setCurrentProperty(const ParticlePropertyReference& property) {
		for(int index = 0; index < count(); index++) {
			if(property == itemData(index).value<ParticlePropertyReference>()) {
				setCurrentIndex(index);
				return;
			}
		}
		setCurrentIndex(-1);
		if(isEditable() && !property.isNull())
			setCurrentText(property.name());
	}
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_PROPERTY_COMBO_BOX_H
