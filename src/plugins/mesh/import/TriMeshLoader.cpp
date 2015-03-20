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
#include <core/dataset/importexport/FileSource.h>
#include <core/scene/objects/geometry/TriMeshObject.h>
#include <core/scene/objects/geometry/TriMeshDisplay.h>
#include <core/utilities/io/FileManager.h>
#include "TriMeshLoader.h"

namespace Mesh {

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void TriMeshLoader::perform()
{
	setProgressText(QString("Reading file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(datasetContainer(), frame().sourceFile);
	if(!waitForSubTask(fetchFileFuture))
		return;

	// Open file for reading.
	QFile file(fetchFileFuture.result());
	CompressedTextReader stream(file, frame().sourceFile.path());

	// Jump to requested byte offset.
	if(frame().byteOffset != 0)
		stream.seek(frame().byteOffset);

	// Parse file.
	parseFile(stream);
}

/******************************************************************************
* Inserts the data loaded by perform() into the provided container object.
* This function is called by the system from the main thread after the
* asynchronous loading task has finished.
******************************************************************************/
void TriMeshLoader::handOver(CompoundObject* container)
{
	OORef<TriMeshObject> triMeshObj = container->findDataObject<TriMeshObject>();
	if(!triMeshObj) {
		triMeshObj = new TriMeshObject(container->dataset());

		// Create a display object for the data object.
		OORef<TriMeshDisplay> triMeshDisplay = new TriMeshDisplay(container->dataset());
		triMeshObj->addDisplayObject(triMeshDisplay);

		container->addDataObject(triMeshObj);
	}
	triMeshObj->mesh() = mesh();
	triMeshObj->notifyDependents(ReferenceEvent::TargetChanged);

	container->removeInactiveObjects({ triMeshObj });
}

};
