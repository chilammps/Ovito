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
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/Future.h>
#include <core/utilities/concurrent/Task.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include "ParticleImporter.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ParticleImporter, LinkedFileImporter);
DEFINE_PROPERTY_FIELD(ParticleImporter, _isMultiTimestepFile, "IsMultiTimestepFile");
SET_PROPERTY_FIELD_LABEL(ParticleImporter, _isMultiTimestepFile, "File contains multiple timesteps");

/******************************************************************************
* Scans the input source (which can be a directory or a single file) to
* discover all animation frames.
******************************************************************************/
Future<QVector<LinkedFileImporter::FrameSourceInformation>> ParticleImporter::findFrames(const QUrl& sourceUrl)
{
	if(isMultiTimestepFile()) {
		DataSetContainer& datasetContainer = *dataset()->container();
		return datasetContainer.taskManager().runInBackground<QVector<LinkedFileImporter::FrameSourceInformation>>(
				[this, sourceUrl](FutureInterface<QVector<LinkedFileImporter::FrameSourceInformation>>& futureInterface) {
					futureInterface.setResult(scanMultiTimestepFile(futureInterface, sourceUrl));
				}
		);
	}
	else {
		return LinkedFileImporter::findFrames(sourceUrl);
	}
}

/******************************************************************************
* Scans the input file for simulation timesteps.
******************************************************************************/
QVector<LinkedFileImporter::FrameSourceInformation> ParticleImporter::scanMultiTimestepFile(FutureInterfaceBase& futureInterface, const QUrl sourceUrl)
{
	QVector<LinkedFileImporter::FrameSourceInformation> result;

	// Check if filename is a wildcard pattern.
	// If yes, find all matching files and scan each one of them.
	QFileInfo fileInfo(sourceUrl.path());
	if(fileInfo.fileName().contains('*') || fileInfo.fileName().contains('?')) {
		auto findFilesFuture = LinkedFileImporter::findWildcardMatches(sourceUrl, dataset()->container());
		if(!futureInterface.waitForSubTask(findFilesFuture))
			return result;
		for(auto item : findFilesFuture.result()) {
			result += scanMultiTimestepFile(futureInterface, item.sourceFile);
		}
		return result;
	}

	futureInterface.setProgressText(tr("Scanning file %1").arg(sourceUrl.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(*dataset()->container(), sourceUrl);
	if(!futureInterface.waitForSubTask(fetchFileFuture))
		return result;

	// Open file.
	QFile file(fetchFileFuture.result());
	CompressedTextReader stream(file, sourceUrl.path());

	// Scan file.
	try {
		scanFileForTimesteps(futureInterface, result, sourceUrl, stream);
	}
	catch(const Exception& ex) {
		// Silently ignore parsing and I/O errors if at least two frames have been read.
		// Keep all frames read up to where the error occurred.
		if(result.size() <= 1)
			throw;
		else
			result.pop_back();		// Remove last discovered frame because it may be corrupted or only partially written.
	}

	return result;
}

/******************************************************************************
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void ParticleImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<LinkedFileImporter::FrameSourceInformation>& frames, const QUrl& sourceUrl, CompressedTextReader& stream)
{
	// By default, register a single frame.
	QFileInfo fileInfo(stream.filename());
	frames.push_back({ sourceUrl, 0, 0, fileInfo.lastModified(), fileInfo.fileName() });
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void ParticleImporter::propertyChanged(const PropertyFieldDescriptor& field)
{
	if(field == PROPERTY_FIELD(ParticleImporter::_isMultiTimestepFile)) {
		// Automatically rescan input file for animation frames when this option has been activated.
		requestFramesUpdate();
	}
	LinkedFileImporter::propertyChanged(field);
}

/******************************************************************************
* This method is called by the LinkedFileObject each time a new source
* file has been selected by the user.
******************************************************************************/
bool ParticleImporter::inspectNewFile(LinkedFileObject* obj)
{
	if(!LinkedFileImporter::inspectNewFile(obj))
		return false;

	// Set flag that this file is new.
	_isNewFile = true;

	return true;
}

};
