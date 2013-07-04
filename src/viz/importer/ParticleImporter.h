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
	ParticleImporter() {}

protected:

	/// \brief Reads the data from the input file(s).
	virtual void loadImplementation(FutureInterface<ImportedDataPtr>& futureInterface, FrameSourceInformation frame) override;

	/// \brief Parses the given input file and stores the data in the given container object.
	virtual void parseFile(FutureInterface<ImportedDataPtr>& futureInterface, ParticleImportData& container, CompressedTextParserStream& stream) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_PARTICLE_IMPORTER_H
