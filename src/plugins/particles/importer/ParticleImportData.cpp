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
#include <core/utilities/concurrent/ProgressManager.h>
#include <plugins/particles/data/SimulationCell.h>
#include <plugins/particles/data/SimulationCellDisplay.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/ParticleTypeProperty.h>
#include <plugins/particles/data/ParticleDisplay.h>
#include <plugins/particles/data/ParticleType.h>
#include "ParticleImportData.h"
#include "ParticleImporter.h"

namespace Particles {

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void ParticleImportTask::load(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(ParticleImporter::tr("Reading file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(frame().sourceFile);
	ProgressManager::instance().addTask(fetchFileFuture);
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
* Lets the data container insert the data it holds into the scene by creating
* appropriate scene objects.
******************************************************************************/
QSet<SceneObject*> ParticleImportTask::insertIntoScene(LinkedFileObject* destination)
{
	QSet<SceneObject*> activeObjects;

	// Adopt simulation cell.
	OORef<SimulationCell> cell = destination->findSceneObject<SimulationCell>();
	if(!cell) {
		cell = new SimulationCell(simulationCell());

		// Create a display object for the simulation cell.
		OORef<SimulationCellDisplay> cellDisplay = new SimulationCellDisplay();
		cell->addDisplayObject(cellDisplay.get());

		// Choose an appropriate simulation cell line rendering width for the given cell dimensions.
		FloatType cellDiameter = (
				simulationCell().matrix().column(0) +
				simulationCell().matrix().column(1) +
				simulationCell().matrix().column(2)).length();
		cellDisplay->setSimulationCellLineWidth(cellDiameter * 1.4e-3f);

		destination->addSceneObject(cell.get());
	}
	else {
		cell->setData(simulationCell());
	}
	activeObjects.insert(cell.get());

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
		if(propertyObj)
			propertyObj->setStorage(QSharedDataPointer<ParticleProperty>(property.release()));
		else {
			propertyObj = ParticlePropertyObject::create(QSharedDataPointer<ParticleProperty>(property.release()));
			destination->addSceneObject(propertyObj.get());
		}
		if(propertyObj->type() == ParticleProperty::ParticleTypeProperty)
			insertParticleTypes(propertyObj.get());
		activeObjects.insert(propertyObj.get());
	}

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
	for(const auto& mapitem : _particleTypes) {
		OORef<ParticleType> ptype = typeProperty->particleType(mapitem.second.id);
		if(ptype == nullptr) {
			ptype = new ParticleType();
			ptype->setId(mapitem.second.id);

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
			ptype->setColor(defaultTypeColors[std::abs(ptype->id()) % (sizeof(defaultTypeColors) / sizeof(defaultTypeColors[0]))]);

			typeProperty->insertParticleType(ptype);
		}
		activeTypes.insert(ptype.get());

		if(!mapitem.second.name.isEmpty())
			ptype->setName(mapitem.second.name);
		else if(ptype->name().isEmpty())
			ptype->setName(ParticleImporter::tr("Type %1").arg(mapitem.second.id));

		if(mapitem.second.color != Color(0,0,0))
			ptype->setColor(mapitem.second.color);

		if(mapitem.second.radius != 0)
			ptype->setRadius(mapitem.second.radius);
	}

	// Remove unused particle types.
	for(int index = typeProperty->particleTypes().size() - 1; index >= 0; index--) {
		if(!activeTypes.contains(typeProperty->particleTypes()[index]))
			typeProperty->removeParticleType(index);
	}
}

};
