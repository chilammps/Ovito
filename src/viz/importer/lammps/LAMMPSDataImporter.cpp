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
#include <core/dataset/importexport/LinkedFileObject.h>
#include "LAMMPSDataImporter.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, LAMMPSDataImporter, ParticleImporter)

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool LAMMPSDataImporter::checkFileFormat(QIODevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextParserStream stream(input, sourceLocation.path());

	// Read first comment line.
	stream.readLine(1024);

	// Read some lines until we encounter the "atoms" keyword.
	for(int i = 0; i < 20; i++) {
		if(stream.eof())
			return false;
		std::string line(stream.readLine(1024).constData());
		// Trim anything from '#' onward.
		size_t commentStart = line.find('#');
		if(commentStart != std::string::npos) line.resize(commentStart);
		// If line is blank, continue.
		if(line.find_first_not_of(" \t\n\r") == std::string::npos) continue;
		if(line.find("atoms") != std::string::npos) {
			int natoms;
			if(sscanf(line.c_str(), "%u", &natoms) != 1 || natoms < 0)
				return false;
			return true;
		}
	}

	return false;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void LAMMPSDataImporter::LAMMPSDataImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	using namespace std;

	futureInterface.setProgressText(tr("Reading LAMMPS data file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Read comment line
	stream.readLine();

	// Read header
	int natoms = 0;
	int natomtypes = 0;
	FloatType xlo = 0, xhi = 0;
	FloatType ylo = 0, yhi = 0;
	FloatType zlo = 0, zhi = 0;
	FloatType xy = 0, xz = 0, yz = 0;

	while(true) {

		string line(stream.readLine().constData());

		// Trim anything from '#' onward.
		size_t commentStart = line.find('#');
		if(commentStart != string::npos) line.resize(commentStart);

    	// If line is blank, continue.
		if(line.find_first_not_of(" \t\n\r") == string::npos) continue;

    	if(line.find("atoms") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &natoms) != 1)
    			throw Exception(tr("Invalid number of atoms (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));

			futureInterface.setProgressRange(natoms);
		}
    	else if(line.find("atom types") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &natomtypes) != 1)
    			throw Exception(tr("Invalid number of atom types (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("xlo xhi") != string::npos) {
    		if(sscanf(line.c_str(), FLOAT_SCANF_STRING_2, &xlo, &xhi) != 2)
    			throw Exception(tr("Invalid xlo/xhi values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("ylo yhi") != string::npos) {
    		if(sscanf(line.c_str(), FLOAT_SCANF_STRING_2, &ylo, &yhi) != 2)
    			throw Exception(tr("Invalid ylo/yhi values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("zlo zhi") != string::npos) {
    		if(sscanf(line.c_str(), FLOAT_SCANF_STRING_2, &zlo, &zhi) != 2)
    			throw Exception(tr("Invalid zlo/zhi values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("xy xz yz") != string::npos) {
    		if(sscanf(line.c_str(), FLOAT_SCANF_STRING_3, &xy, &xz, &yz) != 3)
    			throw Exception(tr("Invalid xy/xz/yz values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("bonds") != string::npos) {}
    	else if(line.find("angles") != string::npos) {}
    	else if(line.find("dihedrals") != string::npos) {}
    	else if(line.find("impropers") != string::npos) {}
    	else if(line.find("bond types") != string::npos) {}
    	else if(line.find("angle types") != string::npos) {}
    	else if(line.find("dihedral types") != string::npos) {}
    	else if(line.find("improper types") != string::npos) {}
    	else if(line.find("extra bond per atom") != string::npos) {}
    	else if(line.find("triangles") != string::npos) {}
    	else if(line.find("ellipsoids") != string::npos) {}
    	else if(line.find("lines") != string::npos) {}
    	else if(line.find("bodies") != string::npos) {}
    	else break;
	}

	if(xhi < xlo || yhi < ylo || zhi < zlo)
		throw Exception(tr("Invalid simulation cell size in header of LAMMPS data file."));

	// Define the simulation cell geometry.
	simulationCell().setMatrix(AffineTransformation(
			Vector3(xhi - xlo, 0, 0),
			Vector3(xy, yhi - ylo, 0),
			Vector3(xz, yz, zhi - zlo),
			Vector3(xlo, ylo, zlo)));

	// Create atom types.
	for(int i = 1; i <= natomtypes; i++)
		addParticleType(i);

	// Read up to non-blank line plus 1 following line.
	while(!stream.eof() && string(stream.line().constData()).find_first_not_of(" \t\n\r") == string::npos) {
		stream.readLine();
	}

	// This flag is set to true if the atomic coordinates have been parsed.
	bool foundAtomsSection = false;
	if(natoms == 0)
		foundAtomsSection = true;

	// Read identifier strings one by one in free-form part of data file.
	QByteArray keyword = stream.line();
	for(;;) {

	    // Skip blank line after keyword.
		if(stream.eof()) break;
		stream.readLine();
		if(stream.eof()) break;

		if(keyword.startsWith("Atoms")) {

			ParticleProperty* posProperty = new ParticleProperty(natoms, ParticleProperty::PositionProperty);
			addParticleProperty(posProperty);
			ParticleProperty* typeProperty = new ParticleProperty(natoms, ParticleProperty::ParticleTypeProperty);
			addParticleProperty(typeProperty);

			for(int i = 0; i < natoms; i++) {
				stream.readLine();

				// Update progress indicator.
				if((i % 4096) == 0) {
					if(futureInterface.isCanceled())
						return;	// Abort!
					futureInterface.setProgressValue(i);
				}

				Point3 pos;
				int tag, atypeindex;

    			if(sscanf(stream.line().constData(), "%u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &tag, &atypeindex, &pos.x(), &pos.y(), &pos.z()) != 5)
					throw Exception(tr("Invalid atom specification (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));

				if(atypeindex < 1 || atypeindex > natomtypes)
					throw Exception(tr("Atom type index out of range (line %i).").arg(stream.lineNumber()));

				posProperty->setPoint3(i, pos);
				typeProperty->setInt(i, atypeindex);
			}

			foundAtomsSection = true;
		}
		else if(keyword.startsWith("Masses")) {
			for(int i = 0; i < natomtypes; i++) {
				stream.readLine();
			}
		}
		else if(keyword.startsWith("Velocities")) {

			// Create the velocity property.
			ParticleProperty* velocityProperty = new ParticleProperty(natoms, ParticleProperty::VelocityProperty);
			addParticleProperty(velocityProperty);

			for(int i = 0; i < natoms; i++) {
				stream.readLine();

				// Update progress indicator.
				if((i % 4096) == 0) {
					if(futureInterface.isCanceled())
						return;	// Abort!
					futureInterface.setProgressValue(i);
				}

				Vector3 v;
				int id;

    			if(sscanf(stream.line().constData(), "%u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &id, &v.x(), &v.y(), &v.z()) != 4)
					throw Exception(tr("Invalid velocity specification (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));

				velocityProperty->setVector3(i, v);
			}
		}
		else if(keyword.isEmpty() == false) {
			throw Exception(tr("Unknown keyword in line %1 of LAMMPS data file: %2.\nNote that the file parser supports only \"atomic style\" LAMMPS data files.").arg(stream.lineNumber()-1).arg(QString::fromLocal8Bit(keyword)));
		}
		else break;

		// Read up to non-blank line plus one subsequent line.
		while(!stream.eof() && string(stream.readLine().constData()).find_first_not_of(" \t\n\r") == string::npos);

		// Read identifier strings one by one in free-form part of data file.
		keyword = stream.line();
	}

	if(!foundAtomsSection)
		throw Exception("LAMMPS data file does not contain atomic coordinates.");

	setInfoText(tr("Number of particles: %1").arg(natoms));
}

};
