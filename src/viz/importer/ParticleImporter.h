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

#ifndef __OVITO_PARTICLE_IMPORTER_H
#define __OVITO_PARTICLE_IMPORTER_H

#include <core/Core.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <viz/data/ParticleProperty.h>
#include "CompressedTextParserStream.h"
#include "ParticleImportData.h"

namespace Viz {

using namespace Ovito;

// These are format strings used with the sscanf() parsing function
// when parsing floating point numbers.
#ifdef FLOATTYPE_FLOAT
	#define FLOAT_SCANF_STRING_1   "%g"
	#define FLOAT_SCANF_STRING_2   "%g %g"
	#define FLOAT_SCANF_STRING_3   "%g %g %g"
#else
	#define FLOAT_SCANF_STRING_1   "%lg"
	#define FLOAT_SCANF_STRING_2   "%lg %lg"
	#define FLOAT_SCANF_STRING_3   "%lg %lg %lg"
#endif

/**
 * \brief Base class for file parsers that read particle-position data.
 */
class ParticleImporter : public LinkedFileImporter
{
public:

	/// \brief Constructs a new instance of this class.
	ParticleImporter() : _isMultiTimestepFile(false) {
		INIT_PROPERTY_FIELD(ParticleImporter::_isMultiTimestepFile);
	}

	/// \brief Returns true if the input file contains multiple timesteps.
	bool isMultiTimestepFile() const { return _isMultiTimestepFile; }

	/// \brief Tells the importer that the input file contains multiple timesteps.
	void setMultiTimestepFile(bool enable) { _isMultiTimestepFile = enable; }

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	virtual Future<QVector<LinkedFileImporter::FrameSourceInformation>> findFrames(const QUrl& sourceUrl) override;

protected:

	/// \brief Reads the data from the input file(s).
	virtual void loadImplementation(FutureInterface<ImportedDataPtr>& futureInterface, FrameSourceInformation frame) override;

	/// \brief Parses the given input file and stores the data in the given container object.
	virtual void parseFile(FutureInterfaceBase& futureInterface, ParticleImportData& container, CompressedTextParserStream& stream) = 0;

	/// \brief Scans the given input file to find all contained simulation frames.
	virtual void scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<LinkedFileImporter::FrameSourceInformation>& frames, const QUrl& sourceUrl, CompressedTextParserStream& stream);

	/// Retrieves the given file in the background and scans it for simulation timesteps.
	virtual void scanMultiTimestepFile(FutureInterface<QVector<LinkedFileImporter::FrameSourceInformation>>& futureInterface, const QUrl sourceUrl);

private:

	/// Indicates that the input file contains multiple timesteps.
	PropertyField<bool> _isMultiTimestepFile;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_isMultiTimestepFile);
};

};

#endif // __OVITO_PARTICLE_IMPORTER_H
