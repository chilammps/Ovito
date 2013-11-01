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
#include <core/utilities/concurrent/Task.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include "ParticleImporter.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ParticleImporter, LinkedFileImporter)
DEFINE_PROPERTY_FIELD(ParticleImporter, _isMultiTimestepFile, "IsMultiTimestepFile")
SET_PROPERTY_FIELD_LABEL(ParticleImporter, _isMultiTimestepFile, "File contains multiple timesteps")

/******************************************************************************
* Scans the input source (which can be a directory or a single file) to
* discover all animation frames.
******************************************************************************/
Future<QVector<LinkedFileImporter::FrameSourceInformation>> ParticleImporter::findFrames(const QUrl& sourceUrl)
{
	if(isMultiTimestepFile()) {
		return runInBackground<QVector<LinkedFileImporter::FrameSourceInformation>>(
				std::bind(&ParticleImporter::scanMultiTimestepFile, this, std::placeholders::_1, sourceUrl));
	}
	else {
		return LinkedFileImporter::findFrames(sourceUrl);
	}
}

/******************************************************************************
* Scans the input file for simulation timesteps.
******************************************************************************/
void ParticleImporter::scanMultiTimestepFile(FutureInterface<QVector<LinkedFileImporter::FrameSourceInformation>>& futureInterface, const QUrl sourceUrl)
{
	futureInterface.setProgressText(tr("Scanning file %1").arg(sourceUrl.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Fetch file.
	Future<QString> fetchFileFuture = FileManager::instance().fetchUrl(sourceUrl);
	ProgressManager::instance().addTask(fetchFileFuture);
	if(!futureInterface.waitForSubTask(fetchFileFuture))
		return;

	// Open file.
	QFile file(fetchFileFuture.result());
	CompressedTextParserStream stream(file, sourceUrl.path());

	// Scan file.
	QVector<LinkedFileImporter::FrameSourceInformation> result;
	try {
		scanFileForTimesteps(futureInterface, result, sourceUrl, stream);
	}
	catch(const Exception& ex) {
		// Silently ignore parsing and I/O errors if at least two frames have been read.
		// Keep all frames read up to where the error occurred.
		if(result.size() <= 1)
			throw;
		else
			result.pop_back();		// Remove last discovered frame because it may be corrupted.
	}

	// Return results.
	if(!futureInterface.isCanceled())
		futureInterface.setResult(result);
}

/******************************************************************************
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void ParticleImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<LinkedFileImporter::FrameSourceInformation>& frames, const QUrl& sourceUrl, CompressedTextParserStream& stream)
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

};
