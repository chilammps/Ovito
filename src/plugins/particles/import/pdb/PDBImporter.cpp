///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include "PDBImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, PDBImporter, ParticleImporter);

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool PDBImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

	// Read the first line.
	stream.readLine(100);
	if(!stream.lineStartsWith("HEADER ") || qstrlen(stream.line()) > 82)
		return false;

	// Read a second line.
	stream.readLine();
	if(qstrlen(stream.line()) < 7 || qstrlen(stream.line()) > 82 || stream.line()[7] != ' ')
		return false;

	return true;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void PDBImporter::PDBImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading PDB file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Read header line.
	stream.readLine();
	if(!stream.lineStartsWith("HEADER ") || qstrlen(stream.line()) > 82)
		throw Exception(tr("Not a Protein Data Bank (PDB) file."));

	// Parse metadata records.
	int numAtoms = 0;
	while(!stream.eof()) {
		stream.readLine();
		int lineLength = qstrlen(stream.line());
		if(lineLength < 3 || lineLength > 82)
			throw Exception(tr("Invalid line length detected in Protein Data Bank (PDB) file at line %1").arg(stream.lineNumber()));

		// Parse simulation cell.
		if(stream.lineStartsWith("CRYST1")) {
			FloatType a,b,c,alpha,beta,gamma;
			if(sscanf(stream.line(), "CRYST1 " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " "
					FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &a, &b, &c, &alpha, &beta, &gamma) != 6)
				throw Exception(tr("Invalid simulation cell in Protein Data Bank (PDB) file at line %1").arg(stream.lineNumber()));
			AffineTransformation cell = AffineTransformation::Identity();
			if(alpha == 90 && beta == 90 && gamma == 90) {
				cell(0,0) = a;
				cell(1,1) = b;
				cell(2,2) = c;
			}
			else if(alpha == 90 && beta == 90) {
				gamma *= FLOATTYPE_PI / 180.0f;
				cell(0,0) = a;
				cell(0,1) = b * cos(gamma);
				cell(1,1) = b * sin(gamma);
				cell(2,2) = c;
			}
			else {
				alpha *= FLOATTYPE_PI / 180.0f;
				beta *= FLOATTYPE_PI / 180.0f;
				gamma *= FLOATTYPE_PI / 180.0f;
				FloatType v = a*b*c*sqrt(1.0f - cos(alpha)*cos(alpha) - cos(beta)*cos(beta) - cos(gamma)*cos(gamma) + 2.0f * cos(alpha) * cos(beta) * cos(gamma));
				cell(0,0) = a;
				cell(0,1) = b * cos(gamma);
				cell(1,1) = b * sin(gamma);
				cell(0,2) = c * cos(beta);
				cell(1,2) = c * (cos(alpha) - cos(beta)*cos(gamma)) / sin(gamma);
				cell(2,2) = v / (a*b*sin(gamma));
			}
			simulationCell().setMatrix(cell);
		}
		// Count atoms.
		else if(stream.lineStartsWith("ATOM  ") || stream.lineStartsWith("HETATM")) {
			numAtoms++;
		}
	}

	setProgressRange(numAtoms);

	// Jump back to beginning of file.
	stream.seek(0);

	// Create the particle properties.
	ParticleProperty* posProperty = new ParticleProperty(numAtoms, ParticleProperty::PositionProperty, 0, true);
	addParticleProperty(posProperty);
	ParticleProperty* typeProperty = new ParticleProperty(numAtoms, ParticleProperty::ParticleTypeProperty, 0, true);
	addParticleProperty(typeProperty);

	// Parse atoms.
	int atomIndex = 0;
	Point3* p = posProperty->dataPoint3();
	int* a = typeProperty->dataInt();
	while(!stream.eof() && atomIndex < numAtoms) {
		stream.readLine();
		int lineLength = qstrlen(stream.line());
		if(lineLength < 3 || lineLength > 82)
			throw Exception(tr("Invalid line length detected in Protein Data Bank (PDB) file at line %1").arg(stream.lineNumber()));

		// Parse atom definition.
		if(stream.lineStartsWith("ATOM  ") || stream.lineStartsWith("HETATM")) {
			char atomType[3];
			int atomTypeLength = 0;
			for(const char* c = stream.line() + 76; c <= stream.line() + std::min(77, lineLength); ++c)
				if(*c != ' ') atomType[atomTypeLength++] = *c;
			if(atomTypeLength == 0) {
				for(const char* c = stream.line() + 12; c <= stream.line() + std::min(15, lineLength); ++c)
					if(*c != ' ') atomType[atomTypeLength++] = *c;
			}
			*a = addParticleTypeName(atomType, atomType + atomTypeLength);
#ifdef FLOATTYPE_FLOAT
			if(lineLength <= 30 || sscanf(stream.line() + 30, "%8g%8g%8g", &p->x(), &p->y(), &p->z()) != 3)
#else
			if(lineLength <= 30 || sscanf(stream.line() + 30, "%8lg%8lg%8lg", &p->x(), &p->y(), &p->z()) != 3)
#endif
				throw Exception(tr("Invalid atom coordinates (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
			atomIndex++;
			++p;
			++a;
		}
	}

	setStatus(tr("Number of particles: %1").arg(numAtoms));
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
