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
#include "LAMMPSTextDumpImporter.h"
#include "LAMMPSDumpImporterSettingsDialog.h"
#include <viz/importer/InputColumnMapping.h>

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, LAMMPSTextDumpImporter, AtomsImporter)
DEFINE_PROPERTY_FIELD(LAMMPSTextDumpImporter, _isMultiTimestepFile, "IsMultiTimestepFile")

/******************************************************************************
* Opens the settings dialog for this importer.
******************************************************************************/
bool LAMMPSTextDumpImporter::showSettingsDialog(QWidget* parent, LinkedFileObject* object)
{
	LAMMPSDumpImporterSettingsDialog dialog(this, parent);
	if(dialog.exec() != QDialog::Accepted)
		return false;

	// Scan the input source for animation frames.
	if(!object->updateFrames())
		return false;

	// Adjust the animation length number to match the number of frames in the input data source.
	object->adjustAnimationInterval();

	return true;
}

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool LAMMPSTextDumpImporter::checkFileFormat(QIODevice& input)
{
	// Open input file.
	CompressedTextParserStream stream(input);

	// Read first line.
	stream.readLine(15);
	if(stream.line().startsWith("ITEM: TIMESTEP"))
		return true;

	return false;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void LAMMPSTextDumpImporter::parseFile(FutureInterface<ImportedDataPtr>& futureInterface, AtomsData& container, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Loading LAMMPS dump file..."));

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	int timestep;
	size_t numParticles = 0;

	while(!stream.eof()) {

		// Parse next line.
		stream.readLine();

		do {
			if(stream.line().startsWith("ITEM: TIMESTEP")) {
				if(sscanf(stream.readLine().constData(), "%i", &timestep) != 1)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid timestep number (line %1):\n%2").arg(stream.lineNumber()).arg(QString::fromLocal8Bit(stream.line())));
				break;
			}
			else if(stream.line().startsWith("ITEM: NUMBER OF ATOMS")) {
				// Parse number of atoms.
				unsigned int u;
				if(sscanf(stream.readLine().constData(), "%u", &u) != 1 || u > 1e9)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid number of atoms in line %1:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));

				numParticles = u;
				futureInterface.setProgressRange(u);
				break;
			}
			else if(stream.line().startsWith("ITEM: BOX BOUNDS xy xz yz")) {

				// Parse optional boundary condition flags.
				QStringList tokens = stream.lineString().mid(qstrlen("ITEM: BOX BOUNDS xy xz yz")).split(ws_re, QString::SkipEmptyParts);
				if(tokens.size() >= 3)
					container.setPbcFlags(tokens[0] == "pp", tokens[1] == "pp", tokens[2] == "pp");

				// Parse triclinic simulation box.
				FloatType tiltFactors[3];
				Box3 simBox;
				for(int k = 0; k < 3; k++) {
					if(sscanf(stream.readLine().constData(), FLOAT_SCANF_STRING_3, &simBox.minc[k], &simBox.maxc[k], &tiltFactors[k]) != 3)
						throw Exception(tr("Invalid box size in line %1 of LAMMPS dump file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
				}

				// LAMMPS only stores the outer bounding box of the simulation cell in the dump file.
				// We have to determine the size of the actual triclinic cell.
				simBox.minc.x() -= std::min(std::min(std::min(tiltFactors[0], tiltFactors[1]), tiltFactors[0]+tiltFactors[1]), (FloatType)0);
				simBox.maxc.x() -= std::max(std::max(std::max(tiltFactors[0], tiltFactors[1]), tiltFactors[0]+tiltFactors[1]), (FloatType)0);
				simBox.minc.y() -= std::min(tiltFactors[2], (FloatType)0);
				simBox.maxc.y() -= std::max(tiltFactors[2], (FloatType)0);
				container.setSimulationCell(AffineTransformation(
						Vector3(simBox.sizeX(), 0, 0),
						Vector3(tiltFactors[0], simBox.sizeY(), 0),
						Vector3(tiltFactors[1], tiltFactors[2], simBox.sizeZ()),
						simBox.minc - Point3::Origin()));
				break;
			}
			else if(stream.line().startsWith("ITEM: BOX BOUNDS")) {
				// Parse optional boundary condition flags.
				QStringList tokens = stream.lineString().mid(qstrlen("ITEM: BOX BOUNDS xy xz yz")).split(ws_re, QString::SkipEmptyParts);
				if(tokens.size() >= 3)
					container.setPbcFlags(tokens[0] == "pp", tokens[1] == "pp", tokens[2] == "pp");

				// Parse orthogonal simulation box size.
				Box3 simBox;
				for(int k = 0; k < 3; k++) {
					if(sscanf(stream.readLine().constData(), FLOAT_SCANF_STRING_2, &simBox.minc[k], &simBox.maxc[k]) != 2)
						throw Exception(tr("Invalid box size in line %1 of dump file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
				}

				container.setSimulationCell(AffineTransformation(
						Vector3(simBox.sizeX(), 0, 0),
						Vector3(0, simBox.sizeY(), 0),
						Vector3(0, 0, simBox.sizeZ()),
						simBox.minc - Point3::Origin()));
				break;
			}
			else if(stream.line().startsWith("ITEM: ATOMS")) {

				// Read the column names list.
				QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
				OVITO_ASSERT(tokens[0] == "ITEM:" && tokens[1] == "ATOMS");
				QStringList columnNames = tokens.mid(2);

				// Set up column-to-property mapping.
				InputColumnMapping columnMapping;
				for(int i = 0; i < columnNames.size(); i++) {
					QString name = columnNames[i].toLower();
					if(name == "x" || name == "xu" || name == "xs" || name == "coordinates") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 0, name);
					else if(name == "y" || name == "yu" || name == "ys") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 1, name);
					else if(name == "z" || name == "zu" || name == "zs") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 2, name);
					else if(name == "vx" || name == "velocities") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 0, name);
					else if(name == "vy") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 1, name);
					else if(name == "vz") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 2, name);
					else if(name == "id") columnMapping.mapStandardColumn(i, ParticleProperty::IdentifierProperty, 0, name);
					else if(name == "type" || name == "atom_types") columnMapping.mapStandardColumn(i, ParticleProperty::ParticleTypeProperty, 0, name);
					else if(name == "mass") columnMapping.mapStandardColumn(i, ParticleProperty::MassProperty, 0, name);
					else if(name == "ix") columnMapping.mapStandardColumn(i, ParticleProperty::PeriodicImageProperty, 0, name);
					else if(name == "iy") columnMapping.mapStandardColumn(i, ParticleProperty::PeriodicImageProperty, 1, name);
					else if(name == "iz") columnMapping.mapStandardColumn(i, ParticleProperty::PeriodicImageProperty, 2, name);
					else if(name == "fx" || name == "forces") columnMapping.mapStandardColumn(i, ParticleProperty::ForceProperty, 0, name);
					else if(name == "fy") columnMapping.mapStandardColumn(i, ParticleProperty::ForceProperty, 1, name);
					else if(name == "fz") columnMapping.mapStandardColumn(i, ParticleProperty::ForceProperty, 2, name);
					else if(name == "c_cna" || name == "pattern") columnMapping.mapStandardColumn(i, ParticleProperty::StructureTypeProperty, 0, name);
					else if(name == "c_epot") columnMapping.mapStandardColumn(i, ParticleProperty::PotentialEnergyProperty, 0, name);
					else if(name == "c_kpot") columnMapping.mapStandardColumn(i, ParticleProperty::KineticEnergyProperty, 0, name);
					else if(name == "c_stress[1]") columnMapping.mapStandardColumn(i, ParticleProperty::StressTensorProperty, 0, name);
					else if(name == "c_stress[2]") columnMapping.mapStandardColumn(i, ParticleProperty::StressTensorProperty, 1, name);
					else if(name == "c_stress[3]") columnMapping.mapStandardColumn(i, ParticleProperty::StressTensorProperty, 2, name);
					else if(name == "c_stress[4]") columnMapping.mapStandardColumn(i, ParticleProperty::StressTensorProperty, 3, name);
					else if(name == "c_stress[5]") columnMapping.mapStandardColumn(i, ParticleProperty::StressTensorProperty, 4, name);
					else if(name == "c_stress[6]") columnMapping.mapStandardColumn(i, ParticleProperty::StressTensorProperty, 5, name);
					else if(name == "selection") columnMapping.mapStandardColumn(i, ParticleProperty::SelectionProperty, 0, name);
				}

				// Parse data columns.
				InputColumnReader columnParser(columnMapping, container, numParticles);
				try {
					for(size_t i = 0; i < numParticles; i++) {
						if((i % 4096) == 0) {
							if(futureInterface.isCanceled())
								return;	// Abort!
							futureInterface.setProgressValue((int)i);
						}
						stream.readLine();
						columnParser.readParticle(i, const_cast<QByteArray&>(stream.line()).data());
					}
				}
				catch(Exception& ex) {
					throw ex.prependGeneralMessage(tr("Parsing error in line %1 of LAMMPS dump file.").arg(stream.lineNumber()));
				}

				return;	// Done!
			}
			else {
				throw Exception(tr("LAMMPS dump file parsing error. Line %1 is invalid:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));
			}
		}
		while(!stream.eof());
	}

	throw Exception(tr("LAMMPS dump file parsing error. Unexpected end of file at line %1.").arg(stream.lineNumber()));
}

};
