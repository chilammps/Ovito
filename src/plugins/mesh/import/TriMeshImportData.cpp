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

#include <plugins/mesh/Mesh.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/scene/objects/geometry/TriMeshObject.h>
#include <core/scene/display/geometry/TriMeshDisplay.h>
#include <core/utilities/io/FileManager.h>
#include "TriMeshImportData.h"

namespace Mesh {

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void TriMeshImportData::load(DataSetContainer& container, FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(QString("Reading file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(container, frame().sourceFile);
	if(!futureInterface.waitForSubTask(fetchFileFuture))
		return;

	// Open file for reading.
	QFile file(fetchFileFuture.result());
	CompressedTextParserStream stream(file, frame().sourceFile.path());

	// Jump to requested byte offset.
	if(frame().byteOffset != 0)
		stream.seek(frame().byteOffset);

	// Parse file.
	parseFile(futureInterface, stream);
}

/******************************************************************************
* Lets the data container insert the data it holds into the scene by creating
* appropriate scene objects.
******************************************************************************/
QSet<SceneObject*> TriMeshImportData::insertIntoScene(LinkedFileObject* destination)
{
	OORef<TriMeshObject> triMeshObj = destination->findSceneObject<TriMeshObject>();
	if(!triMeshObj) {
		triMeshObj = new TriMeshObject(destination->dataSet());

		// Create a display object for the scene object.
		OORef<TriMeshDisplay> triMeshDisplay = new TriMeshDisplay(destination->dataSet());
		triMeshObj->addDisplayObject(triMeshDisplay.get());

		destination->addSceneObject(triMeshObj.get());
	}
	triMeshObj->mesh() = mesh();
	triMeshObj->notifyDependents(ReferenceEvent::TargetChanged);

	return { triMeshObj.get() };
}

};
