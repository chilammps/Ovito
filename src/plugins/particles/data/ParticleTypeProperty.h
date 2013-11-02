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

/**
 * \file ParticleTypeProperty.h
 * \brief Contains the definition of the Particles::ParticleTypeProperty class.
 */

#ifndef __OVITO_PARTICLE_TYPE_PROPERTY_H
#define __OVITO_PARTICLE_TYPE_PROPERTY_H

#include <core/Core.h>
#include "ParticlePropertyObject.h"
#include "ParticleType.h"

namespace Particles {

/**
 * \brief A particle property that stores the particle types.
 */
class ParticleTypeProperty : public ParticlePropertyObject
{
public:

	/// \brief Standard constructor.
	Q_INVOKABLE ParticleTypeProperty(ParticleProperty* storage = nullptr);

	//////////////////////////////////// Specific methods //////////////////////////////////

	/// Inserts a particle type into the list of types.
	void insertParticleType(const OORef<ParticleType>& ptype);

	/// Returns the list of particle types.
	const ParticleTypeList& particleTypes() const { return _particleTypes; }

	/// Replaces the list of particle types.
	void setParticleTypes(const ParticleTypeList& types) { _particleTypes = types; }

	/// Returns the particle type with the given ID, or NULL if no such type exists.
	ParticleType* particleType(int id) const {
		for(ParticleType* ptype : particleTypes())
			if(ptype->id() == id)
				return ptype;
		return nullptr;
	}

	/// Returns the particle type with the given name, or NULL if no such type exists.
	ParticleType* particleType(const QString& name) const {
		for(ParticleType* ptype : particleTypes())
			if(ptype->name() == name)
				return ptype;
		return nullptr;
	}

	/// Removes a single particle type from this object.
	void removeParticleType(int index);

	/// Returns a map from type identifier to color.
	std::map<int,Color> colorMap() const {
		std::map<int,Color> m;
		for(ParticleType* ptype : particleTypes())
			m.insert({ptype->id(), ptype->color()});
		return m;
	}

	/// Returns a map from type identifier to particle radius.
	std::map<int,FloatType> radiusMap() const {
		std::map<int,FloatType> m;
		for(ParticleType* ptype : particleTypes())
			m.insert({ptype->id(), ptype->radius()});
		return m;
	}

	/////////////////////////////////////// from RefTarget //////////////////////////////

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	virtual bool isSubObjectEditable() const override { return true; }

protected:

	/// Contains the particle types.
	VectorReferenceField<ParticleType> _particleTypes;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_particleTypes);
};


/**
 * \brief A properties editor for the ParticleTypeProperty class.
 */
class ParticleTypePropertyEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ParticleTypePropertyEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_PARTICLE_TYPE_PROPERTY_H
