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

#ifndef __OVITO_PARTICLE_IMPORT_DATA_H
#define __OVITO_PARTICLE_IMPORT_DATA_H

#include <core/Core.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <viz/data/ParticleProperty.h>
#include <viz/data/ParticlePropertyObject.h>

namespace Viz {

using namespace Ovito;

/**
 * Container structure for data imported by a ParticleImporter.
 */
class ParticleImportData : public LinkedFileImporter::ImportedData
{
public:

	struct ParticleTypeDefinition {
		int id;
		QString name;
		Color color;
		FloatType radius;
	};

public:

	/// Lets the data container insert the data it holds into the scene by creating
	/// appropriate scene objects.
	virtual void insertIntoScene(LinkedFileObject* destination) override;

	/// Returns the current simulation cell matrix.
	const AffineTransformation& simulationCell() const { return _simulationCell; }

	/// Sets the simulation cell matrix.
	void setSimulationCell(const AffineTransformation& cellMatrix) { _simulationCell = cellMatrix; }

	/// Returns the PBC flags.
	const std::array<bool,3>& pbcFlags() const { return _pbcFlags; }

	/// Sets the PBC flags.
	void setPbcFlags(const std::array<bool,3>& flags) { _pbcFlags = flags; }

	/// Sets the PBC flags.
	void setPbcFlags(bool pbcX, bool pbcY, bool pbcZ) { _pbcFlags[0] = pbcX; _pbcFlags[1] = pbcY; _pbcFlags[2] = pbcZ; }

	/// Returns the list of particle properties.
	const std::vector<QExplicitlySharedDataPointer<ParticleProperty>>& particleProperties() const { return _properties; }

	/// Adds a new particle property.
	void addParticleProperty(const QExplicitlySharedDataPointer<ParticleProperty>& property) { _properties.push_back(property); }

	/// Removes a particle property from the list.
	void removeParticleProperty(int index) { _properties.erase(_properties.begin() + index); }

	/// Defines a new particle type with the given id.
	void addParticleType(int id, const QString& name = QString(), const Color& color = Color(0,0,0), FloatType radius = 0) {
		_particleTypes[id] = { id, name, color, radius };
	}

	/// Returns the list of particle types.
	const std::map<int, ParticleTypeDefinition>& particleTypes() const { return _particleTypes; }

	/// Returns the identifier of the particle type with the given name.
	/// Returns -1 if no such type exists.
	int particleTypeFromName(const QString& name) const {
		for(const auto& type : _particleTypes)
			if(type.second.name == name)
				return type.first;
		return -1;
	}

private:

	/// Creates an instance of ParticlePropertyObject or a derived class.
	OORef<ParticlePropertyObject> createPropertyObject(ParticleProperty* input);

	/// Inserts the stores particle types into the given destination object.
	void insertParticleTypes(ParticlePropertyObject* propertyObj);

private:

	/// The geometry of the cell.
	AffineTransformation _simulationCell = AffineTransformation::Zero();

	/// PBC flags.
	std::array<bool,3> _pbcFlags = {{ true, true, true }};

	/// Particle properties.
	std::vector<QExplicitlySharedDataPointer<ParticleProperty>> _properties;

	/// The list of particle types.
	std::map<int, ParticleTypeDefinition> _particleTypes;
};

};

#endif // __OVITO_PARTICLE_IMPORT_DATA_H
