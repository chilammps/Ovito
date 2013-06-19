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

#include <core/Core.h>
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/Future.h>
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <viz/data/SimulationCell.h>
#include <viz/data/ParticleProperty.h>
#include <viz/data/ParticlePropertyObject.h>
#include "AtomsImporter.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, AtomsImporter, LinkedFileImporter)

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void AtomsImporter::loadImplementation(FutureInterface<ImportedDataPtr>& futureInterface, FrameSourceInformation frame)
{
	futureInterface.setProgressText(tr("Loading file %1").arg(frame.sourceFile.toString()));

	// Fetch file.
	Future<QString> fetchFileFuture;
	fetchFileFuture = FileManager::instance().fetchUrl(frame.sourceFile);
	ProgressManager::instance().addTask(fetchFileFuture);
	if(!futureInterface.waitForSubTask(fetchFileFuture))
		return;

	// Open file.
	QString filename = fetchFileFuture.result();
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		throw Exception(tr("Failed to open file %1 for reading: %2").arg(filename).arg(file.errorString()));

	// Jump to requested file byte offset.
	if(frame.byteOffset != 0 && file.seek(frame.byteOffset) == false)
		throw Exception(tr("Failed to seek to byte offset %1 of file %2: %3").arg(frame.byteOffset).arg(filename).arg(file.errorString()));

	// Parse file.
	std::shared_ptr<AtomsData> result(std::make_shared<AtomsData>());
	parseFile(futureInterface, *result, file);

	// Return results.
	if(!futureInterface.isCanceled())
		futureInterface.setResult(result);
}

/******************************************************************************
* Lets the data container insert the data it holds into the scene by creating
* appropriate scene objects.
******************************************************************************/
void AtomsImporter::AtomsData::insertIntoScene(LinkedFileObject* destination)
{
	QSet<SceneObject*> activeObjects;

	// Adopt simulation cell.
	OORef<SimulationCell> cell = destination->findSceneObject<SimulationCell>();
	if(!cell) {
		cell = new SimulationCell(simulationCell(), pbcFlags()[0], pbcFlags()[1], pbcFlags()[2]);
		destination->addSceneObject(cell.get());
	}
	else {
		cell->setCellMatrix(simulationCell());
		cell->setPBCFlags(pbcFlags());
	}
	activeObjects.insert(cell.get());

	// Adopt particle properties.
	for(const auto& property : particleProperties()) {
		OORef<ParticlePropertyObject> propertyObj;
		for(const auto& sceneObj : destination->sceneObjects()) {
			ParticlePropertyObject* po = dynamic_object_cast<ParticlePropertyObject>(sceneObj);
			if(po != nullptr && po->type() == property->type() && po->name() == property->name()) {
				propertyObj = po;
				break;
			}
		}
		if(propertyObj)
			propertyObj->replaceStorage(property.data());
		else {
			propertyObj = new ParticlePropertyObject(property.data());
			destination->addSceneObject(propertyObj.get());
		}
		activeObjects.insert(propertyObj.get());
	}

	destination->removeInactiveObjects(activeObjects);
}

};
