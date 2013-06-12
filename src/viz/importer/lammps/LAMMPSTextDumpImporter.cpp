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
#include <viz/data/SimulationCell.h>
#include "LAMMPSTextDumpImporter.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(LAMMPSTextDumpImporter, LinkedFileImporter)

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void LAMMPSTextDumpImporter::loadImplementation(FutureInterface<OORef<SceneObject>>& futureInterface, FrameSourceInformation frame, bool suppressDialogs)
{
	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(frame.sourceFile);
	if(!futureInterface.waitForSubTask(fetchFileFuture))
		return;

	// Open file.
	QString filename = fetchFileFuture.result();
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		throw Exception(tr("Failed to open file %1 for reading: %2").arg(filename).arg(file.errorString()));

	// Create simulation cell.
	OORef<SimulationCell> cell(new SimulationCell());

	// Return results.
	futureInterface.setResult(cell);
}

};
