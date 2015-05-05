///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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
#include <core/dataset/importexport/FileSource.h>
#include "FHIAimsImporter.h"

#include <boost/algorithm/string.hpp>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, FHIAimsImporter, ParticleImporter);

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool FHIAimsImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

	// Skip comment line
	stream.readLine();

	// Read global scaling factor
	double scaling_factor;
	stream.readLine();
	if(stream.eof() || sscanf(stream.line(), "%lg", &scaling_factor) != 1 || scaling_factor <= 0)
		return false;

	// Read cell matrix
	for(int i = 0; i < 3; i++) {
		stream.readLine();
		if(stream.lineString().split(ws_re, QString::SkipEmptyParts).size() != 3)
			return false;
		double x,y,z;
		if(sscanf(stream.line(), "%lg %lg %lg", &x, &y, &z) != 3 || stream.eof())
			return false;
	}

	// Parse number of atoms per type.
	int nAtomTypes = 0;
	for(int i = 0; i < 2; i++) {
		stream.readLine();
		QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
		if(i == 0) nAtomTypes = tokens.size();
		else if(nAtomTypes != tokens.size())
			return false;
		int n = 0;
		Q_FOREACH(const QString& token, tokens) {
			bool ok;
			n += token.toInt(&ok);
		}
		if(n > 0)
			return true;
	}

	return false;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void FHIAimsImporter::FHIAimsImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading FHI-aims file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// First pass: determine the cell geometry and number of atoms.
	AffineTransformation cell = AffineTransformation::Identity();
	int lattVecCount = 0;
	int totalAtomCount = 0;
	while(!stream.eof()) {
		std::string line = stream.readLine();

		// Trim anything from '#' onward.
		size_t commentStart = line.find_first_of('#');
		if(commentStart != std::string::npos) line.resize(commentStart);

		// Trim leading whitespace.
		boost::algorithm::trim_left(line);

		if(boost::algorithm::starts_with(line, "lattice_vector")) {
			if(lattVecCount >= 3)
				throw Exception(tr("More than three lattice vectors (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
			if(scanf(line.c_str(), "lattice_vector " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &cell(0,lattVecCount), &cell(1,lattVecCount), &cell(2,lattVecCount)) != 3 || cell.column(lattVecCount) == Vector3::Zero())
				throw Exception(tr("Invalid cell vector (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
			lattVecCount++;
		}
		else if(boost::algorithm::starts_with(line, "atom")) {
			totalAtomCount++;
		}
	}

	// Create the particle properties.
	ParticleProperty* posProperty = new ParticleProperty(totalAtomCount, ParticleProperty::PositionProperty, 0, false);
	addParticleProperty(posProperty);
	ParticleProperty* typeProperty = new ParticleProperty(totalAtomCount, ParticleProperty::ParticleTypeProperty, 0, false);
	addParticleProperty(typeProperty);

	// Return to file beginning.
	stream.seek(0);

	// Second pass: read atom coordinates and types.

	setStatus(tr("%1 atoms").arg(totalAtomCount));
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
