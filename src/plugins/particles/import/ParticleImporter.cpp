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
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/FileSource.h>
#include "ParticleImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ParticleImporter, FileSourceImporter);
DEFINE_PROPERTY_FIELD(ParticleImporter, _isMultiTimestepFile, "IsMultiTimestepFile");
SET_PROPERTY_FIELD_LABEL(ParticleImporter, _isMultiTimestepFile, "File contains multiple timesteps");

/******************************************************************************
* Scans the given external path (which may be a directory and a wild-card pattern,
* or a single file containing multiple frames) to find all available animation frames.
******************************************************************************/
Future<QVector<FileSourceImporter::Frame>> ParticleImporter::discoverFrames(const QUrl& sourceUrl)
{
	if(isMultiTimestepFile()) {
		return dataset()->container()->taskManager().execAsync(
				std::bind(&ParticleImporter::discoverFramesInFile, this, sourceUrl, std::placeholders::_1));
	}
	else {
		return FileSourceImporter::discoverFrames(sourceUrl);
	}
}

/******************************************************************************
* Scans the input file for simulation timesteps.
******************************************************************************/
QVector<FileSourceImporter::Frame> ParticleImporter::discoverFramesInFile(const QUrl sourceUrl, FutureInterfaceBase& futureInterface)
{
	QVector<FileSourceImporter::Frame> result;

	// Check if filename is a wildcard pattern.
	// If yes, find all matching files and scan each of them.
	QFileInfo fileInfo(sourceUrl.path());
	if(fileInfo.fileName().contains('*') || fileInfo.fileName().contains('?')) {
		auto findFilesFuture = FileSourceImporter::findWildcardMatches(sourceUrl, dataset()->container());
		if(!futureInterface.waitForSubTask(findFilesFuture))
			return result;
		for(auto item : findFilesFuture.result()) {
			result += discoverFramesInFile(item.sourceFile, futureInterface);
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
	catch(const Exception&) {
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
void ParticleImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<FileSourceImporter::Frame>& frames, const QUrl& sourceUrl, CompressedTextReader& stream)
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
	FileSourceImporter::propertyChanged(field);
}

/******************************************************************************
* This method is called by the FileSource each time a new source
* file has been selected by the user.
******************************************************************************/
bool ParticleImporter::inspectNewFile(FileSource* obj)
{
	if(!FileSourceImporter::inspectNewFile(obj))
		return false;

	// Set flag that this file is new.
	_isNewFile = true;

	return true;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
