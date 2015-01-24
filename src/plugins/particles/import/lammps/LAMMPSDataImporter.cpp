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
#include <core/dataset/importexport/FileSource.h>
#include <core/dataset/DataSetContainer.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>
#include "LAMMPSDataImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, LAMMPSDataImporter, ParticleImporter);
DEFINE_PROPERTY_FIELD(LAMMPSDataImporter, _atomStyle, "AtomStyle");
SET_PROPERTY_FIELD_LABEL(LAMMPSDataImporter, _atomStyle, "Atom style");

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool LAMMPSDataImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

	// Read first comment line.
	stream.readLine(1024);

	// Read some lines until we encounter the "atoms" keyword.
	for(int i = 0; i < 20; i++) {
		if(stream.eof())
			return false;
		std::string line(stream.readLine(1024));
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
* This method is called by the FileSource each time a new source
* file has been selected by the user.
******************************************************************************/
bool LAMMPSDataImporter::inspectNewFile(FileSource* obj)
{
	if(!ParticleImporter::inspectNewFile(obj))
		return false;

	if(obj->frames().empty())
		return false;

	// Don't show any dialogs in console mode.
	if(Application::instance().consoleMode())
		return true;

	// Start task that inspects the file to detect the LAMMPS atom style.
	std::shared_ptr<LAMMPSDataImportTask> inspectionTask = std::make_shared<LAMMPSDataImportTask>(dataset()->container(), obj->frames().front(), true, atomStyle(), true);
	if(!dataset()->container()->taskManager().runTask(inspectionTask))
		return false;

	if(inspectionTask->atomStyle() == AtomStyle_Unknown) {
		return showAtomStyleDialog(dataset()->mainWindow());
	}
	else setAtomStyle(inspectionTask->atomStyle());

	return true;
}

/******************************************************************************
* Displays a dialog box that allows the user to select the LAMMPS atom style of the data file.
******************************************************************************/
bool LAMMPSDataImporter::showAtomStyleDialog(QWidget* parent)
{
	QMap<QString, LAMMPSAtomStyle> styleList = {
			{ QStringLiteral("atomic"), AtomStyle_Atomic },
			{ QStringLiteral("bond"), AtomStyle_Bond },
			{ QStringLiteral("charge"), AtomStyle_Charge },
			{ QStringLiteral("dipole"), AtomStyle_Dipole },
			{ QStringLiteral("molecular"), AtomStyle_Molecular },
			{ QStringLiteral("full"), AtomStyle_Full }
	};
	QStringList itemList = styleList.keys();

	QSettings settings;
	settings.beginGroup(LAMMPSDataImporter::OOType.plugin()->pluginId());
	settings.beginGroup(LAMMPSDataImporter::OOType.name());

	int currentIndex = -1;
	for(int i = 0; i < itemList.size(); i++)
		if(atomStyle() == styleList[itemList[i]])
			currentIndex = i;
	if(currentIndex == -1)
		currentIndex = itemList.indexOf(settings.value("DefaultAtomStyle").toString());
	if(currentIndex == -1)
		currentIndex = itemList.indexOf("atomic");

	bool ok;
	QString selectedItem = QInputDialog::getItem(parent, tr("LAMMPS data file"), tr("Select the LAMMPS atom style used by the data file:"), itemList, currentIndex, false, &ok);
	if(!ok) return false;

	settings.setValue("DefaultAtomStyle", selectedItem);
	setAtomStyle(styleList[selectedItem]);

	return true;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void LAMMPSDataImporter::LAMMPSDataImportTask::parseFile(CompressedTextReader& stream)
{
	using namespace std;

	setProgressText(tr("Reading LAMMPS data file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Read comment line
	stream.readLine();

	int natoms = 0;
	int natomtypes = 0;
	int nbonds = 0;
	int nangles = 0;
	int ndihedrals = 0;
	int nimpropers = 0;
	int nbondtypes = 0;
	int nangletypes = 0;
	int ndihedraltypes = 0;
	int nimpropertypes = 0;
	FloatType xlo = 0, xhi = 0;
	FloatType ylo = 0, yhi = 0;
	FloatType zlo = 0, zhi = 0;
	FloatType xy = 0, xz = 0, yz = 0;

	// Read header
	while(true) {

		string line(stream.readLine());

		// Trim anything from '#' onward.
		size_t commentStart = line.find('#');
		if(commentStart != string::npos) line.resize(commentStart);

    	// If line is blank, continue.
		if(line.find_first_not_of(" \t\n\r") == string::npos) continue;

    	if(line.find("atoms") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &natoms) != 1)
    			throw Exception(tr("Invalid number of atoms (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));

			setProgressRange(natoms);
		}
    	else if(line.find("atom types") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &natomtypes) != 1)
    			throw Exception(tr("Invalid number of atom types (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("xlo xhi") != string::npos) {
    		if(sscanf(line.c_str(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &xlo, &xhi) != 2)
    			throw Exception(tr("Invalid xlo/xhi values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("ylo yhi") != string::npos) {
    		if(sscanf(line.c_str(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &ylo, &yhi) != 2)
    			throw Exception(tr("Invalid ylo/yhi values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("zlo zhi") != string::npos) {
    		if(sscanf(line.c_str(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &zlo, &zhi) != 2)
    			throw Exception(tr("Invalid zlo/zhi values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("xy xz yz") != string::npos) {
    		if(sscanf(line.c_str(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &xy, &xz, &yz) != 3)
    			throw Exception(tr("Invalid xy/xz/yz values (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("bonds") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &nbonds) != 1)
    			throw Exception(tr("Invalid number of bonds (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("bond types") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &nbondtypes) != 1)
    			throw Exception(tr("Invalid number of bond types (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("angle types") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &nangletypes) != 1)
    			throw Exception(tr("Invalid number of angle types (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("dihedral types") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &ndihedraltypes) != 1)
    			throw Exception(tr("Invalid number of dihedral types (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("improper types") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &nimpropertypes) != 1)
    			throw Exception(tr("Invalid number of improper types (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("angles") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &nangles) != 1)
    			throw Exception(tr("Invalid number of angles (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("dihedrals") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &ndihedrals) != 1)
    			throw Exception(tr("Invalid number of dihedrals (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("impropers") != string::npos) {
    		if(sscanf(line.c_str(), "%u", &nimpropers) != 1)
    			throw Exception(tr("Invalid number of impropers (line %1): %2").arg(stream.lineNumber()).arg(line.c_str()));
    	}
    	else if(line.find("extra bond per atom") != string::npos) {}
    	else if(line.find("extra angle per atom") != string::npos) {}
    	else if(line.find("extra dihedral per atom") != string::npos) {}
    	else if(line.find("extra improper per atom") != string::npos) {}
    	else if(line.find("extra special per atom") != string::npos) {}
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
		addParticleTypeId(i);

	// Skip to following line after first non-blank line.
	while(!stream.eof() && string(stream.line()).find_first_not_of(" \t\n\r") == string::npos) {
		stream.readLine();
	}

	// This flag is set to true if the atomic coordinates have been parsed.
	bool foundAtomsSection = false;
	if(natoms == 0)
		foundAtomsSection = true;

	// Read identifier strings one by one in free-form part of data file.
	QByteArray keyword = QByteArray(stream.line()).trimmed();
	for(;;) {
	    // Skip blank line after keyword.
		if(stream.eof()) break;
		stream.readLine();
		if(stream.eof()) break;

		if(keyword.startsWith("Atoms")) {
			if(natoms != 0) {
				stream.readLine();
				bool withPBCImageFlags = detectAtomStyle(stream.line(), keyword);
				if(_detectAtomStyle)
					return;

				ParticleProperty* posProperty = new ParticleProperty(natoms, ParticleProperty::PositionProperty, 0, true);
				addParticleProperty(posProperty);
				Point3* pos = posProperty->dataPoint3();
				ParticleProperty* typeProperty = new ParticleProperty(natoms, ParticleProperty::ParticleTypeProperty, 0, true);
				addParticleProperty(typeProperty);
				int* atomType = typeProperty->dataInt();
				ParticleProperty* identifierProperty = new ParticleProperty(natoms, ParticleProperty::IdentifierProperty, 0, true);
				addParticleProperty(identifierProperty);
				int* atomId = identifierProperty->dataInt();

				Point3I* pbcImage = nullptr;
				if(withPBCImageFlags) {
					ParticleProperty* pbcProperty = new ParticleProperty(natoms, ParticleProperty::PeriodicImageProperty, 0, true);
					addParticleProperty(pbcProperty);
					pbcImage = pbcProperty->dataPoint3I();
				}

				if(_atomStyle == AtomStyle_Atomic || _atomStyle == AtomStyle_Hybrid) {
					for(int i = 0; i < natoms; i++, ++pos, ++atomType, ++atomId) {
						if(!reportProgress(i)) return;
						if(i != 0) stream.readLine();
						bool invalidLine;
						if(!pbcImage)
							invalidLine = (sscanf(stream.line(), "%u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, atomId, atomType, &pos->x(), &pos->y(), &pos->z()) != 5);
						else {
							invalidLine = (sscanf(stream.line(), "%u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " %i %i %i", atomId, atomType, &pos->x(), &pos->y(), &pos->z(), &pbcImage->x(), &pbcImage->y(), &pbcImage->z()) != 5+3);
							++pbcImage;
						}
						if(invalidLine)
							throw Exception(tr("Invalid data in Atoms section of LAMMPS data file at line %1: %2").arg(stream.lineNumber()).arg(stream.lineString()));
						if(*atomType < 1 || *atomType > natomtypes)
							throw Exception(tr("Atom type out of range in Atoms section of LAMMPS data file at line %1.").arg(stream.lineNumber()));
					}
				}
				else if(_atomStyle == AtomStyle_Charge || _atomStyle == AtomStyle_Dipole) {
					ParticleProperty* chargeProperty = new ParticleProperty(natoms, ParticleProperty::ChargeProperty, 0, true);
					addParticleProperty(chargeProperty);
					FloatType* charge = chargeProperty->dataFloat();
					for(int i = 0; i < natoms; i++, ++pos, ++atomType, ++atomId, ++charge) {
						if(!reportProgress(i)) return;
						if(i != 0) stream.readLine();
						bool invalidLine;
						if(!pbcImage)
							invalidLine = (sscanf(stream.line(), "%u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, atomId, atomType, charge, &pos->x(), &pos->y(), &pos->z()) != 6);
						else {
							invalidLine = (sscanf(stream.line(), "%u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " %i %i %i", atomId, atomType, charge, &pos->x(), &pos->y(), &pos->z(), &pbcImage->x(), &pbcImage->y(), &pbcImage->z()) != 6+3);
							++pbcImage;
						}
						if(invalidLine)
							throw Exception(tr("Invalid data in Atoms section of LAMMPS data file at line %1: %2").arg(stream.lineNumber()).arg(stream.lineString()));
						if(*atomType < 1 || *atomType > natomtypes)
							throw Exception(tr("Atom type out of range in Atoms section of LAMMPS data file at line %1.").arg(stream.lineNumber()));
					}
				}
				else if(_atomStyle == AtomStyle_Angle || _atomStyle == AtomStyle_Bond || _atomStyle == AtomStyle_Molecular) {
					ParticleProperty* moleculeProperty = new ParticleProperty(natoms, ParticleProperty::MoleculeProperty, 0, true);
					addParticleProperty(moleculeProperty);
					int* molecule = moleculeProperty->dataInt();
					for(int i = 0; i < natoms; i++, ++pos, ++atomType, ++atomId, ++molecule) {
						if(!reportProgress(i)) return;
						if(i != 0) stream.readLine();
						bool invalidLine;
						if(!pbcImage)
							invalidLine = (sscanf(stream.line(), "%u %u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, atomId, molecule, atomType, &pos->x(), &pos->y(), &pos->z()) != 6);
						else {
							invalidLine = (sscanf(stream.line(), "%u %u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " %i %i %i", atomId, molecule, atomType, &pos->x(), &pos->y(), &pos->z(), &pbcImage->x(), &pbcImage->y(), &pbcImage->z()) != 6+3);
							++pbcImage;
						}
						if(invalidLine)
							throw Exception(tr("Invalid data in Atoms section of LAMMPS data file at line %1: %2").arg(stream.lineNumber()).arg(stream.lineString()));
						if(*atomType < 1 || *atomType > natomtypes)
							throw Exception(tr("Atom type out of range in Atoms section of LAMMPS data file at line %1.").arg(stream.lineNumber()));
					}
				}
				else if(_atomStyle == AtomStyle_Full) {
					ParticleProperty* chargeProperty = new ParticleProperty(natoms, ParticleProperty::ChargeProperty, 0, true);
					addParticleProperty(chargeProperty);
					FloatType* charge = chargeProperty->dataFloat();
					ParticleProperty* moleculeProperty = new ParticleProperty(natoms, ParticleProperty::MoleculeProperty, 0, true);
					addParticleProperty(moleculeProperty);
					int* molecule = moleculeProperty->dataInt();
					for(int i = 0; i < natoms; i++, ++pos, ++atomType, ++atomId, ++charge, ++molecule) {
						if(!reportProgress(i)) return;
						if(i != 0) stream.readLine();
						bool invalidLine;
						if(!pbcImage)
							invalidLine = (sscanf(stream.line(), "%u %u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, atomId, molecule, atomType, charge, &pos->x(), &pos->y(), &pos->z()) != 7);
						else {
							invalidLine = (sscanf(stream.line(), "%u %u %u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " %i %i %i", atomId, molecule, atomType, charge, &pos->x(), &pos->y(), &pos->z(), &pbcImage->x(), &pbcImage->y(), &pbcImage->z()) != 7+3);
							++pbcImage;
						}
						if(invalidLine)
							throw Exception(tr("Invalid data in Atoms section of LAMMPS data file at line %1: %2").arg(stream.lineNumber()).arg(stream.lineString()));
						if(*atomType < 1 || *atomType > natomtypes)
							throw Exception(tr("Atom type out of range in Atoms section of LAMMPS data file at line %1.").arg(stream.lineNumber()));
					}
				}
				else if(_atomStyle == AtomStyle_Unknown) {
					throw Exception(tr("Number of columns in Atoms section of data file (line %1) does not match to selected LAMMPS atom style.").arg(stream.lineNumber()));
				}
				else {
					throw Exception(tr("Selected LAMMPS atom style is not supported by the file parser."));
				}
			}
			foundAtomsSection = true;
		}
		else if(keyword.startsWith("Velocities")) {

			// Get the atomic IDs.
			ParticleProperty* identifierProperty = particleProperty(ParticleProperty::IdentifierProperty);
			if(!identifierProperty)
				throw Exception(tr("Atoms section must precede Velocities section in data file (error in line %1).").arg(stream.lineNumber()));

			// Create the velocity property.
			ParticleProperty* velocityProperty = new ParticleProperty(natoms, ParticleProperty::VelocityProperty, 0, true);
			addParticleProperty(velocityProperty);

			for(int i = 0; i < natoms; i++) {
				if(!reportProgress(i)) return;
				stream.readLine();

				Vector3 v;
				int atomId;

    			if(sscanf(stream.line(), "%u " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &atomId, &v.x(), &v.y(), &v.z()) != 4)
					throw Exception(tr("Invalid velocity specification (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));

    			int atomIndex = i;
    			if(atomId != identifierProperty->getInt(i)) {
    				atomIndex = std::find(identifierProperty->constDataInt(), identifierProperty->constDataInt() + identifierProperty->size(), atomId) - identifierProperty->constDataInt();
					if(atomIndex >= (int)identifierProperty->size())
    					throw Exception(tr("Nonexistent atom ID encountered in line %1 of data file.").arg(stream.lineNumber()));
    			}

				velocityProperty->setVector3(atomIndex, v);
			}
		}
		else if(keyword.startsWith("Masses")) {
			for(int i = 1; i <= natomtypes; i++) {
				// Try to parse atom types names, which some data files list as comments in the Masses section.
				const char* start = stream.readLine();
				while(*start && *start != '#') start++;
				if(*start) {
					QStringList words = QString::fromLocal8Bit(start).split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
					if(words.size() == 2)
						setParticleTypeName(i, words[1]);
				}
			}
		}
		else if(keyword.startsWith("Pair Coeffs")) {
			for(int i = 0; i < natomtypes; i++) stream.readLine();
		}
		else if(keyword.startsWith("PairIJ Coeffs")) {
			for(int i = 0; i < natomtypes*(natomtypes+1)/2; i++) stream.readLine();
		}
		else if(keyword.startsWith("Bond Coeffs")) {
			for(int i = 0; i < nbondtypes; i++) stream.readLine();
		}
		else if(keyword.startsWith("Angle Coeffs") || keyword.startsWith("BondAngle Coeffs") || keyword.startsWith("BondBond Coeffs")) {
			for(int i = 0; i < nangletypes; i++) stream.readLine();
		}
		else if(keyword.startsWith("Dihedral Coeffs") || keyword.startsWith("EndBondTorsion Coeffs") || keyword.startsWith("BondBond13 Coeffs") || keyword.startsWith("MiddleBondTorsion Coeffs") || keyword.startsWith("AngleAngleTorsion Coeffs")) {
			for(int i = 0; i < ndihedraltypes; i++) stream.readLine();
		}
		else if(keyword.startsWith("Improper Coeffs") || keyword.startsWith("AngleAngle Coeffs")) {
			for(int i = 0; i < nimpropertypes; i++) stream.readLine();
		}
		else if(keyword.startsWith("Angles")) {
			for(int i = 0; i < nangles; i++) stream.readLine();
		}
		else if(keyword.startsWith("Dihedrals")) {
			for(int i = 0; i < ndihedrals; i++) stream.readLine();
		}
		else if(keyword.startsWith("Impropers")) {
			for(int i = 0; i < nimpropers; i++) stream.readLine();
		}
		else if(keyword.startsWith("Bonds")) {

			// Get the atomic IDs and positions.
			ParticleProperty* identifierProperty = particleProperty(ParticleProperty::IdentifierProperty);
			ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
			if(!identifierProperty || !posProperty)
				throw Exception(tr("Atoms section must precede Bonds section in data file (error in line %1).").arg(stream.lineNumber()));

			// Create bonds storage.
			setBonds(new BondsStorage());
			bonds()->bonds().reserve(nbonds);
			setProgressRange(nbonds);
			for(int i = 0; i < nbonds; i++) {
				if(!reportProgress(i)) return;
				stream.readLine();

				int bondId, bondType, atomId1, atomId2;
    			if(sscanf(stream.line(), "%u %u %u %u", &bondId, &bondType, &atomId1, &atomId2) != 4)
					throw Exception(tr("Invalid bond specification (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));

   				unsigned int atomIndex1 = std::find(identifierProperty->constDataInt(), identifierProperty->constDataInt() + identifierProperty->size(), atomId1) - identifierProperty->constDataInt();
   				unsigned int atomIndex2 = std::find(identifierProperty->constDataInt(), identifierProperty->constDataInt() + identifierProperty->size(), atomId2) - identifierProperty->constDataInt();
				if(atomIndex1 >= identifierProperty->size() || atomIndex2 >= identifierProperty->size())
					throw Exception(tr("Nonexistent atom ID encountered in line %1 of data file.").arg(stream.lineNumber()));

				// Use minimum image convention to determine PBC shift vector of the bond.
				Vector3 delta = simulationCell().absoluteToReduced(posProperty->getPoint3(atomIndex2) - posProperty->getPoint3(atomIndex1));
				Vector_3<int8_t> shift = Vector_3<int8_t>::Zero();
				for(size_t dim = 0; dim < 3; dim++) {
					if(simulationCell().pbcFlags()[dim])
						shift[dim] -= (int8_t)floor(delta[dim] + FloatType(0.5));
				}

				bonds()->addBond(atomIndex1, atomIndex2,  shift);
				bonds()->addBond(atomIndex2, atomIndex1, -shift);
			}
		}
		else if(keyword.isEmpty() == false) {
			throw Exception(tr("Unknown or unsupported keyword in line %1 of LAMMPS data file: %2.").arg(stream.lineNumber()-1).arg(QString::fromLocal8Bit(keyword)));
		}
		else break;

		// Read up to non-blank line plus one subsequent line.
		while(!stream.eof() && string(stream.readLine()).find_first_not_of(" \t\n\r") == string::npos);

		// Read identifier strings one by one in free-form part of data file.
		keyword = QByteArray(stream.line()).trimmed();
	}

	if(!foundAtomsSection)
		throw Exception("LAMMPS data file does not contain atomic coordinates.");

	setStatus(tr("Number of particles: %1").arg(natoms));
}

/******************************************************************************
* Detects or verifies the LAMMPS atom style used by the data file.
******************************************************************************/
bool LAMMPSDataImporter::LAMMPSDataImportTask::detectAtomStyle(const char* firstLine, const QByteArray& keywordLine)
{
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Some data files contain a comment after the Atoms keyword that indicates the atom type.
	QString atomTypeHint;
	int commentStart = keywordLine.indexOf('#');
	if(commentStart != -1) {
		QStringList words = QString::fromLatin1(keywordLine.data() + commentStart).split(ws_re, QString::SkipEmptyParts);
		if(words.size() == 2)
			atomTypeHint = words[1];
	}

	// Count fields in first line of Atoms section.
	QString str = QString::fromLatin1(firstLine);
	commentStart = str.indexOf(QChar('#'));
	if(commentStart >= 0) str.truncate(commentStart);
	QStringList tokens = str.split(ws_re, QString::SkipEmptyParts);
	int count = tokens.size();

	if(_atomStyle == AtomStyle_Unknown && !atomTypeHint.isEmpty()) {
		if(atomTypeHint == QStringLiteral("atomic")) _atomStyle = AtomStyle_Atomic;
		else if(atomTypeHint == QStringLiteral("full")) _atomStyle = AtomStyle_Full;
		else if(atomTypeHint == QStringLiteral("angle")) _atomStyle = AtomStyle_Angle;
		else if(atomTypeHint == QStringLiteral("bond")) _atomStyle = AtomStyle_Bond;
		else if(atomTypeHint == QStringLiteral("charge")) _atomStyle = AtomStyle_Charge;
		else if(atomTypeHint == QStringLiteral("molecular")) _atomStyle = AtomStyle_Molecular;
	}

	if(_atomStyle == AtomStyle_Unknown) {
		if(count == 5) {
			_atomStyle = AtomStyle_Atomic;
			return false;
		}
		else if(count == 5+3) {
			if(!tokens[5].contains(QChar('.')) && !tokens[6].contains(QChar('.')) && !tokens[7].contains(QChar('.'))) {
				_atomStyle = AtomStyle_Atomic;
				return true;
			}
		}
	}

	if(_atomStyle == AtomStyle_Atomic && (count == 5 || count == 5+3)) return (count == 5+3);
	if(_atomStyle == AtomStyle_Hybrid && count >= 5) return false;
	if((_atomStyle == AtomStyle_Angle || _atomStyle == AtomStyle_Bond || _atomStyle == AtomStyle_Charge || _atomStyle == AtomStyle_Molecular) && (count == 6 || count == 6+3)) return (count == 6+3);
	if((_atomStyle == AtomStyle_Body || _atomStyle == AtomStyle_Ellipsoid || _atomStyle == AtomStyle_Full || _atomStyle == AtomStyle_Peri || _atomStyle == AtomStyle_Sphere) && (count == 7 || count == 7+3)) return (count == 7+3);
	if((_atomStyle == AtomStyle_Electron || _atomStyle == AtomStyle_Line || _atomStyle == AtomStyle_Meso || _atomStyle == AtomStyle_Template || _atomStyle == AtomStyle_Tri) && (count == 8 || count == 8+3)) return (count == 8+3);
	if(_atomStyle == AtomStyle_Dipole && (count == 9 || count == 9+3)) return (count == 9+3);
	if(_atomStyle == AtomStyle_Wavepacket && (count == 11 || count == 11+3)) return (count == 11+3);
	_atomStyle = AtomStyle_Unknown;
	return false;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
