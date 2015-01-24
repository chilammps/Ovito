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

#ifndef __OVITO_PARTICLE_FRAME_LOADER_H
#define __OVITO_PARTICLE_FRAME_LOADER_H

#include <plugins/particles/Particles.h>
#include <core/dataset/importexport/FileSourceImporter.h>
#include <core/utilities/io/CompressedTextReader.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/BondsStorage.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/data/SimulationCell.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import)

/**
 * \brief Background loading task and data container used by a ParticleImporter.
 */
class OVITO_PARTICLES_EXPORT ParticleFrameLoader : public FileSourceImporter::FrameLoader
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
	ParticleFrameLoader(DataSetContainer* container, const FileSourceImporter::Frame& frame, bool isNewFile)
		: FileSourceImporter::FrameLoader(container, frame),
		  _timestep(-1),
		  _isNewFile(isNewFile) {}

	/// Loads the requested frame data from the external file.
	virtual void perform() override;

	/// Inserts the data loaded by perform() into the provided container object. This function is
	/// called by the system from the main thread after the asynchronous loading task has finished.
	virtual void handOver(CompoundObject* container) override;

	/// Returns the current simulation cell matrix.
	const SimulationCell& simulationCell() const { return _simulationCell; }

	/// Returns a reference to the simulation cell.
	SimulationCell& simulationCell() { return _simulationCell; }

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

	/// Changes the name of an existing particle type.
	void setParticleTypeName(int id, const QString& name) {
		for(auto& type : _particleTypes) {
			if(type.id == id) {
				type.name = name;
				type.name8bit = name.toLocal8Bit().constData();
				break;
			}
		}
	}

	/// Defines a new particle type with the given id.
	inline int addParticleTypeName(const char* name, const char* name_end) {
		size_t nameLen = name_end - name;
		for(const auto& type : _particleTypes) {
			if(type.name8bit.compare(0, type.name8bit.size(), name, nameLen) == 0)
				return type.id;
		}
		int id = _particleTypes.size() + 1;
		_particleTypes.push_back({ id, QString::fromLocal8Bit(name, nameLen), std::string(name, nameLen), Color(0,0,0), 0.0f });
		return id;
	}

	/// Defines a new particle type with the given id.
	int addParticleTypeName(const char* name, const char* name_end, const Color& color, FloatType radius = 0) {
		size_t nameLen = name_end - name;
		for(const auto& type : _particleTypes) {
			if(type.name8bit.compare(0, type.name8bit.size(), name, nameLen) == 0)
				return type.id;
		}
		int id = _particleTypes.size() + 1;
		_particleTypes.push_back({ id, QString::fromLocal8Bit(name, nameLen), std::string(name, nameLen), color, radius });
		return id;
	}

	/// Returns the list of particle types.
	const std::vector<ParticleTypeDefinition>& particleTypes() const { return _particleTypes; }

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

	/// Sets the bonds between particles.
	void setBonds(BondsStorage* bonds) { _bonds.reset(bonds); }

	/// Returns the bonds between particles (if present).
	BondsStorage* bonds() const { return _bonds.get(); }

protected:

	/// Parses the given input file and stores the data in this container object.
	virtual void parseFile(CompressedTextReader& stream) = 0;

	// Updates the progress indicator.
	bool reportProgress(int particleIndex) {
		if((particleIndex % 4096) == 0) {
			if(isCanceled()) return false;
			setProgressValue(particleIndex);
		}
		return true;
	}

	/// Inserts the stores particle types into the given destination object.
	void insertParticleTypes(ParticlePropertyObject* propertyObj);

private:

	/// The simulation cell.
	SimulationCell _simulationCell;

	/// Particle properties.
	std::vector<std::unique_ptr<ParticleProperty>> _properties;

	/// The list of particle types.
	std::vector<ParticleTypeDefinition> _particleTypes;

	/// The list of bonds between particles (if present).
	std::unique_ptr<BondsStorage> _bonds;

	/// The simulation timestep number.
	int _timestep;

	/// Flag indicating that the file currently being loaded has been newly selected by the user.
	/// If not, then the file being loaded is just another frame from the existing sequence.
	/// In this case we don't want to overwrite any settings like the periodic boundary flags that
	/// might have been changed by the user.
	bool _isNewFile;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_FRAME_LOADER_H
