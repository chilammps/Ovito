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

#ifndef __OVITO_PARTICLE_TYPE_PROPERTY_H
#define __OVITO_PARTICLE_TYPE_PROPERTY_H

#include <plugins/particles/Particles.h>
#include "ParticlePropertyObject.h"
#include "ParticleType.h"

namespace Ovito { namespace Particles {

/**
 * \brief A particle property that stores the particle types.
 */
class OVITO_PARTICLES_EXPORT ParticleTypeProperty : public ParticlePropertyObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE ParticleTypeProperty(DataSet* dataset, ParticleProperty* storage = nullptr);

	//////////////////////////////////// Specific methods //////////////////////////////////

	/// Inserts a particle type into the list of types.
	void insertParticleType(ParticleType* ptype);

	/// Returns the list of particle types.
	const QVector<ParticleType*>& particleTypes() const { return _particleTypes; }

	/// Replaces the list of particle types.
	void setParticleTypes(const QVector<ParticleType*>& types) { _particleTypes = types; }

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
	void removeParticleType(int index) {
		_particleTypes.remove(index);
	}

	/// Removes all particle types from this object.
	void clearParticleTypes() {
		_particleTypes.clear();
	}

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

	//////////////////////////////////// from RefTarget //////////////////////////////////

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	virtual bool isSubObjectEditable() const override { return true; }

	//////////////////////////////////// Default settings ////////////////////////////////

	/// Returns the default color for the particle type with the given ID.
	static Color getDefaultParticleColorFromId(int particleTypeId);

	/// Returns the default color for a named particle type.
	static Color getDefaultParticleColorFromName(const QString& particleTypeName, int particleTypeId);

protected:

	/// Contains the particle types.
	VectorReferenceField<ParticleType> _particleTypes;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(_particleTypes);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

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

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_TYPE_PROPERTY_H
