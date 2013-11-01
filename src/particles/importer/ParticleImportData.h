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
#include <core/utilities/io/CompressedTextParserStream.h>
#include <particles/data/ParticleProperty.h>
#include <particles/data/ParticlePropertyObject.h>
#include <particles/data/SimulationCellData.h>

namespace Particles {

using namespace Ovito;

/**
 * Container structure for data imported by a ParticleImporter.
 */
class ParticleImportTask : public LinkedFileImporter::ImportTask
{
public:

	struct ParticleTypeDefinition {
		int id;
		QString name;
		Color color;
		FloatType radius;
	};

public:

	/// Constructor.
	ParticleImportTask(const LinkedFileImporter::FrameSourceInformation& frame) : LinkedFileImporter::ImportTask(frame) {}

	/// Is called in the background thread to perform the data file import.
	virtual void load(FutureInterfaceBase& futureInterface) override;

	/// Lets the data container insert the data it holds into the scene by creating
	/// appropriate scene objects.
	virtual void insertIntoScene(LinkedFileObject* destination) override;

	/// Returns the current simulation cell matrix.
	const SimulationCellData& simulationCell() const { return _simulationCell; }

	/// Returns a reference to the simulation cell.
	SimulationCellData& simulationCell() { return _simulationCell; }

	/// Returns the list of particle properties.
	const std::vector<std::unique_ptr<ParticleProperty>>& particleProperties() const { return _properties; }

	/// Returns a standard particle property if defined.
	ParticleProperty* particleProperty(ParticleProperty::Type which) const {
		for(const auto& prop : _properties)
			if(prop->type() == which)
				return prop.get();
		return nullptr;
	}

	/// Adds a new particle property.
	void addParticleProperty(ParticleProperty* property) { _properties.push_back(std::unique_ptr<ParticleProperty>(property)); }

	/// Removes a particle property from the list.
	void removeParticleProperty(int index) { _properties.erase(_properties.begin() + index); }

	/// Defines a new particle type with the given id.
	void addParticleType(int id) {
		if(_particleTypes.find(id) == _particleTypes.end())
			_particleTypes[id] = { id, QString(), Color(0,0,0), 0 };
	}

	/// Defines a new particle type with the given id.
	void addParticleType(int id, const QString& name, const Color& color = Color(0,0,0), FloatType radius = 0) {
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

protected:

	/// Parses the given input file and stores the data in this container object.
	virtual void parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream) = 0;

	/// Inserts the stores particle types into the given destination object.
	void insertParticleTypes(ParticlePropertyObject* propertyObj);

private:

	/// The simulation cell.
	SimulationCellData _simulationCell;

	/// Particle properties.
	std::vector<std::unique_ptr<ParticleProperty>> _properties;

	/// The list of particle types.
	std::map<int, ParticleTypeDefinition> _particleTypes;
};

};

#endif // __OVITO_PARTICLE_IMPORT_DATA_H
