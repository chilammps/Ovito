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
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/utilities/io/FileManager.h>
#include <plugins/particles/data/SimulationCell.h>
#include <plugins/particles/data/SimulationCellDisplay.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/ParticleTypeProperty.h>
#include <plugins/particles/data/ParticleDisplay.h>
#include <plugins/particles/data/ParticleType.h>
#include "ParticleImportTask.h"
#include "ParticleImporter.h"

namespace Particles {

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void ParticleImportTask::load(DataSetContainer& container, FutureInterfaceBase& futureInterface)
{
	_datasetContainer = &container;
	futureInterface.setProgressText(ParticleImporter::tr("Reading file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(container, frame().sourceFile);
	if(!futureInterface.waitForSubTask(fetchFileFuture)) {
		return;
	}
	OVITO_ASSERT(fetchFileFuture.isCanceled() == false);

	// Open file.
	QFile file(fetchFileFuture.result());
	CompressedTextParserStream stream(file, frame().sourceFile.path());

	// Jump to requested file byte offset.
	if(frame().byteOffset != 0)
		stream.seek(frame().byteOffset);

	// Parse file.
	parseFile(futureInterface, stream);
}

/******************************************************************************
* Sorts the particle types w.r.t. their name. Reassigns the per-particle type IDs.
* This method is used by file parsers that create particle types on the go while the read the particle data.
* In such a case, the assignment of IDs to types depends on the storage order of particles in the file, which is not desirable.
******************************************************************************/
void ParticleImportTask::sortParticleTypesByName()
{
	// Check if type IDs form a consecutive sequence starting at 1.
	for(int index = 0; index < _particleTypes.size(); index++) {
		if(_particleTypes[index].id != index + 1)
			return;
	}

	// Check if types are already in the correct order.
	auto compare = [](const ParticleTypeDefinition& a, const ParticleTypeDefinition& b) -> bool { return a.name.compare(b.name) < 0; };
	if(std::is_sorted(_particleTypes.begin(), _particleTypes.end(), compare))
		return;

	// Reorder types.
	std::sort(_particleTypes.begin(), _particleTypes.end(), compare);

	// Build map of IDs.
	std::vector<int> mapping(_particleTypes.size() + 1);
	for(int index = 0; index < _particleTypes.size(); index++) {
		mapping[_particleTypes[index].id] = index + 1;
		_particleTypes[index].id = index + 1;
	}

	// Remap particle type IDs.
	ParticleProperty* typeProperty = particleProperty(ParticleProperty::ParticleTypeProperty);
	if(typeProperty) {
		for(int& t : typeProperty->intRange()) {
			OVITO_ASSERT(t >= 1 && t < mapping.size());
			t = mapping[t];
		}
	}
}

/******************************************************************************
* Sorts particle types with ascending identifier.
******************************************************************************/
void ParticleImportTask::sortParticleTypesById()
{
	auto compare = [](const ParticleTypeDefinition& a, const ParticleTypeDefinition& b) -> bool { return a.id < b.id; };
	std::sort(_particleTypes.begin(), _particleTypes.end(), compare);
}

/******************************************************************************
* Lets the data container insert the data it holds into the scene by creating
* appropriate scene objects.
******************************************************************************/
QSet<SceneObject*> ParticleImportTask::insertIntoScene(LinkedFileObject* destination)
{
	QSet<SceneObject*> activeObjects;

	// Adopt simulation cell.
	OORef<SimulationCell> cell = destination->findSceneObject<SimulationCell>();
	if(!cell) {
		cell = new SimulationCell(destination->dataset(), simulationCell());

		// Create a display object for the simulation cell.
		OORef<SimulationCellDisplay> cellDisplay = new SimulationCellDisplay(destination->dataset());
		cellDisplay->loadUserDefaults();
		cell->addDisplayObject(cellDisplay);

		// Choose an appropriate line width for the cell size.
		FloatType cellDiameter = (
				simulationCell().matrix().column(0) +
				simulationCell().matrix().column(1) +
				simulationCell().matrix().column(2)).length();
		cellDisplay->setSimulationCellLineWidth(cellDiameter * 1.4e-3f);

		destination->addSceneObject(cell);
	}
	else {
		// Adopt pbc flags from input file only if it is a new file.
		// This gives the user the option to change the pbc flags without them
		// being overwritten when a new frame from a simulation sequence is loaded.
		cell->setData(simulationCell(), _isNewFile);
	}
	activeObjects.insert(cell);

	// Adopt particle properties.
	for(auto& property : _properties) {
		OORef<ParticlePropertyObject> propertyObj;
		for(const auto& sceneObj : destination->sceneObjects()) {
			ParticlePropertyObject* po = dynamic_object_cast<ParticlePropertyObject>(sceneObj);
			if(po != nullptr && po->type() == property->type() && po->name() == property->name()) {
				propertyObj = po;
				break;
			}
		}

		if(propertyObj) {
			propertyObj->setStorage(QSharedDataPointer<ParticleProperty>(property.release()));
		}
		else {
			propertyObj = ParticlePropertyObject::createFromStorage(destination->dataset(), QSharedDataPointer<ParticleProperty>(property.release()));
			destination->addSceneObject(propertyObj);
		}

		if(propertyObj->type() == ParticleProperty::ParticleTypeProperty) {
			insertParticleTypes(propertyObj);
		}
		activeObjects.insert(propertyObj);
	}

	// Pass timestep number to modification pipeline system.
	if(hasTimestep())
		destination->setAttributes({{ QStringLiteral("Timestep"), QVariant::fromValue(timestep()) }});
	else
		destination->clearAttributes();

	return activeObjects;
}

/******************************************************************************
* Inserts the stores particle types into the given destination object.
******************************************************************************/
void ParticleImportTask::insertParticleTypes(ParticlePropertyObject* propertyObj)
{
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(propertyObj);
	if(!typeProperty)
		return;

	QSet<ParticleType*> activeTypes;
	for(const auto& item : _particleTypes) {
		OORef<ParticleType> ptype = typeProperty->particleType(item.id);
		if(ptype == nullptr) {
			ptype = new ParticleType(typeProperty->dataset());
			ptype->setId(item.id);

			// Assign initial standard color to new particle types.
			if(item.color != Color(0,0,0))
				ptype->setColor(item.color);
			else if(item.name.isEmpty())
				ptype->setColor(ParticleTypeProperty::getDefaultParticleColorFromId(ptype->id()));
			else
				ptype->setColor(ParticleTypeProperty::getDefaultParticleColorFromName(item.name, ptype->id()));

			typeProperty->insertParticleType(ptype);
		}
		activeTypes.insert(ptype);

		if(!item.name.isEmpty())
			ptype->setName(item.name);
		else if(ptype->name().isEmpty())
			ptype->setName(ParticleImporter::tr("Type %1").arg(item.id));

		if(item.color != Color(0,0,0))
			ptype->setColor(item.color);

		if(item.radius != 0)
			ptype->setRadius(item.radius);
	}

	// Remove unused particle types.
	for(int index = typeProperty->particleTypes().size() - 1; index >= 0; index--) {
		if(!activeTypes.contains(typeProperty->particleTypes()[index]))
			typeProperty->removeParticleType(index);
	}
}

};
