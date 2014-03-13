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

#ifndef __OVITO_PARTICLE_IMPORT_TASK_H
#define __OVITO_PARTICLE_IMPORT_TASK_H

#include <plugins/particles/Particles.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/utilities/io/CompressedTextParserStream.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/SimulationCellData.h>

namespace Particles {

using namespace Ovito;

/**
 * Background loading task and data container used by a ParticleImporter derived class.
 */
class OVITO_PARTICLES_EXPORT ParticleImportTask : public LinkedFileImporter::ImportTask
{
public:

	struct ParticleTypeDefinition {
		int id;
		QString name;
		std::string name8bit;
		Color color;
		FloatType radius;
	};

public:

	/// Constructor.
	ParticleImportTask(const LinkedFileImporter::FrameSourceInformation& frame) : LinkedFileImporter::ImportTask(frame), _datasetContainer(nullptr), _timestep(-1) {}

	/// Is called in the background thread to perform the data file import.
	virtual void load(DataSetContainer& container, FutureInterfaceBase& futureInterface) override;

	/// Returns the current dataset container.
	DataSetContainer& datasetContainer() const { OVITO_CHECK_POINTER(_datasetContainer); return *_datasetContainer; }

	/// Lets the data container insert the data it holds into the scene by creating
	/// appropriate scene objects.
	virtual QSet<SceneObject*> insertIntoScene(LinkedFileObject* destination) override;

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
	void addParticleTypeId(int id) {
		for(const auto& type : _particleTypes) {
			if(type.id == id)
				return;
		}
		_particleTypes.push_back({ id, QString(), std::string(), Color(0,0,0), 0 });
	}

	/// Defines a new particle type with the given id.
	void addParticleTypeId(int id, const QString& name, const Color& color = Color(0,0,0), FloatType radius = 0) {
		for(const auto& type : _particleTypes) {
			if(type.id == id)
				return;
		}
		_particleTypes.push_back({ id, name, name.toLocal8Bit().constData(), color, radius });
	}

	/// Defines a new particle type with the given id.
	inline int addParticleTypeName(const char* name) {
		for(const auto& type : _particleTypes) {
			if(type.name8bit == name)
				return type.id;
		}
		int id = _particleTypes.size() + 1;
		_particleTypes.push_back({ id, QString::fromLocal8Bit(name), name, Color(0,0,0), 0.0f });
		return id;
	}

	/// Defines a new particle type with the given id.
	int addParticleTypeName(const char* name, const Color& color, FloatType radius = 0) {
		for(const auto& type : _particleTypes) {
			if(type.name8bit == name)
				return type.id;
		}
		int id = _particleTypes.size() + 1;
		_particleTypes.push_back({ id, QString::fromLocal8Bit(name), name, color, radius });
		return id;
	}

	/// Returns the list of particle types.
	const std::vector<ParticleTypeDefinition>& particleTypes() const { return _particleTypes; }

#if 0
	/// Returns the identifier of the particle type with the given name.
	/// Returns -1 if no such type exists.
	int particleTypeFromName(const QString& name) const {
		int index = 0;
		for(const auto& type : _particleTypes) {
			if(type.second.name == name)
				return index;
			index++;
		}
		return -1;
	}
#endif

	/// Sorts the particle types w.r.t. their name. Reassigns the per-particle type IDs.
	/// This method is used by file parsers that create particle types on the go while the read the particle data.
	/// In such a case, the assignment of IDs to types depends on the storage order of particles in the file, which is not desirable.
	void sortParticleTypesByName();

	/// Sorts particle types with ascending identifier.
	void sortParticleTypesById();

	/// Returns the simulation timestep number, or -1 if undefined.
	int timestep() const { return _timestep; }

	/// Sets the simulation timestep number.
	void setTimestep(int timestep) { _timestep = timestep; }

	/// Returns true if the loaded file format contained information on the simulation timestep.
	bool hasTimestep() const { return _timestep != -1; }

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
	std::vector<ParticleTypeDefinition> _particleTypes;

	/// The simulation timestep number.
	int _timestep;

	/// The current dataset container.
	DataSetContainer* _datasetContainer;
};

};

#endif // __OVITO_PARTICLE_IMPORT_TASK_H
