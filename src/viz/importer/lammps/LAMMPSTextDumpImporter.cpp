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

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, LAMMPSTextDumpImporter, ParticleImporter)

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
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void LAMMPSTextDumpImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<LinkedFileImporter::FrameSourceInformation>& frames, const QUrl& sourceUrl, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Scanning LAMMPS dump file %1").arg(stream.filename()));
	futureInterface.setProgressRange(stream.underlyingSize() / 1000);

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	int timestep;
	size_t numParticles = 0;
	QFileInfo fileInfo(stream.filename());
	QString filename = fileInfo.fileName();
	QDateTime lastModified = fileInfo.lastModified();

	while(!stream.eof()) {
		qint64 byteOffset = stream.byteOffset();

		// Parse next line.
		stream.readLine();

		do {
			int startLineNumber = stream.lineNumber();
			if(stream.line().startsWith("ITEM: TIMESTEP")) {
				if(sscanf(stream.readLine().constData(), "%i", &timestep) != 1)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid timestep number (line %1):\n%2").arg(stream.lineNumber()).arg(QString::fromLocal8Bit(stream.line())));
				FrameSourceInformation frame;
				frame.sourceFile = sourceUrl;
				frame.byteOffset = byteOffset;
				frame.lineNumber = startLineNumber;
				frame.lastModificationTime = lastModified;
				frame.label = QString("%1 (Timestep %2)").arg(filename).arg(timestep);
				frames.push_back(frame);
				break;
			}
			else if(stream.line().startsWith("ITEM: NUMBER OF ATOMS")) {
				// Parse number of atoms.
				unsigned int u;
				if(sscanf(stream.readLine().constData(), "%u", &u) != 1 || u > 1e9)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid number of atoms in line %1:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));
				numParticles = u;
				break;
			}
			else if(stream.line().startsWith("ITEM: ATOMS")) {
				for(size_t i = 0; i < numParticles; i++) {
					stream.readLine();
					if((i % 4096) == 0) {
						futureInterface.setProgressValue(stream.underlyingByteOffset() / 1000);
						if(futureInterface.isCanceled())
							return;
					}
				}
				break;
			}
			else if(stream.line().startsWith("ITEM:")) {
				// Skip lines up to next ITEM:
				while(!stream.eof()) {
					byteOffset = stream.byteOffset();
					stream.readLine();
					if(stream.line().startsWith("ITEM:"))
						break;
				}
			}
			else {
				throw Exception(tr("LAMMPS dump file parsing error. Line %1 of file %2 is invalid.").arg(stream.lineNumber()).arg(stream.filename()));
			}
		}
		while(!stream.eof());
	}
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void LAMMPSTextDumpImporter::parseFile(FutureInterfaceBase& futureInterface, ParticleImportData& container, CompressedTextParserStream& stream)
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
					container.simulationCell().setPbcFlags(tokens[0] == "pp", tokens[1] == "pp", tokens[2] == "pp");

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
				container.simulationCell().setMatrix(AffineTransformation(
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
					container.simulationCell().setPbcFlags(tokens[0] == "pp", tokens[1] == "pp", tokens[2] == "pp");

				// Parse orthogonal simulation box size.
				Box3 simBox;
				for(int k = 0; k < 3; k++) {
					if(sscanf(stream.readLine().constData(), FLOAT_SCANF_STRING_2, &simBox.minc[k], &simBox.maxc[k]) != 2)
						throw Exception(tr("Invalid box size in line %1 of dump file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
				}

				container.simulationCell().setMatrix(AffineTransformation(
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
				bool reducedCoordinates = false;
				for(int i = 0; i < columnNames.size(); i++) {
					QString name = columnNames[i].toLower();
					if(name == "x" || name == "xu" || name == "coordinates") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 0, name);
					else if(name == "y" || name == "yu") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 1, name);
					else if(name == "z" || name == "zu") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 2, name);
					else if(name == "xs" || name == "xsu") { columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 0, name); reducedCoordinates = true; }
					else if(name == "ys" || name == "ysu") { columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 1, name); reducedCoordinates = true; }
					else if(name == "zs" || name == "zsu") { columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 2, name); reducedCoordinates = true; }
					else if(name == "vx" || name == "velocities") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 0, name);
					else if(name == "vy") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 1, name);
					else if(name == "vz") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 2, name);
					else if(name == "id") columnMapping.mapStandardColumn(i, ParticleProperty::IdentifierProperty, 0, name);
					else if(name == "type" || name == "element" || name == "atom_types") columnMapping.mapStandardColumn(i, ParticleProperty::ParticleTypeProperty, 0, name);
					else if(name == "mass") columnMapping.mapStandardColumn(i, ParticleProperty::MassProperty, 0, name);
					else if(name == "radius") columnMapping.mapStandardColumn(i, ParticleProperty::RadiusProperty, 0, name);
					else if(name == "ix") columnMapping.mapStandardColumn(i, ParticleProperty::PeriodicImageProperty, 0, name);
					else if(name == "iy") columnMapping.mapStandardColumn(i, ParticleProperty::PeriodicImageProperty, 1, name);
					else if(name == "iz") columnMapping.mapStandardColumn(i, ParticleProperty::PeriodicImageProperty, 2, name);
					else if(name == "fx" || name == "forces") columnMapping.mapStandardColumn(i, ParticleProperty::ForceProperty, 0, name);
					else if(name == "fy") columnMapping.mapStandardColumn(i, ParticleProperty::ForceProperty, 1, name);
					else if(name == "fz") columnMapping.mapStandardColumn(i, ParticleProperty::ForceProperty, 2, name);
					else if(name == "mux") columnMapping.mapStandardColumn(i, ParticleProperty::DipoleOrientationProperty, 0, name);
					else if(name == "muy") columnMapping.mapStandardColumn(i, ParticleProperty::DipoleOrientationProperty, 1, name);
					else if(name == "muz") columnMapping.mapStandardColumn(i, ParticleProperty::DipoleOrientationProperty, 2, name);
					else if(name == "mu") columnMapping.mapStandardColumn(i, ParticleProperty::DipoleMagnitudeProperty, 0, name);
					else if(name == "omegax") columnMapping.mapStandardColumn(i, ParticleProperty::AngularVelocityProperty, 0, name);
					else if(name == "omegay") columnMapping.mapStandardColumn(i, ParticleProperty::AngularVelocityProperty, 1, name);
					else if(name == "omegaz") columnMapping.mapStandardColumn(i, ParticleProperty::AngularVelocityProperty, 2, name);
					else if(name == "angmomx") columnMapping.mapStandardColumn(i, ParticleProperty::AngularMomentumProperty, 0, name);
					else if(name == "angmomy") columnMapping.mapStandardColumn(i, ParticleProperty::AngularMomentumProperty, 1, name);
					else if(name == "angmomz") columnMapping.mapStandardColumn(i, ParticleProperty::AngularMomentumProperty, 2, name);
					else if(name == "tqx") columnMapping.mapStandardColumn(i, ParticleProperty::TorqueProperty, 0, name);
					else if(name == "tqy") columnMapping.mapStandardColumn(i, ParticleProperty::TorqueProperty, 1, name);
					else if(name == "tqz") columnMapping.mapStandardColumn(i, ParticleProperty::TorqueProperty, 2, name);
					else if(name == "spin") columnMapping.mapStandardColumn(i, ParticleProperty::SpinProperty, 0, name);
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

				if(reducedCoordinates) {
					ParticleProperty* posProperty = container.particleProperty(ParticleProperty::PositionProperty);
					if(posProperty) {
						const AffineTransformation simCell = container.simulationCell().matrix();
						Point3* p = posProperty->dataPoint3();
						Point3* p_end = p + posProperty->size();
						for(; p != p_end; ++p)
							*p = simCell * (*p);
					}
				}

				container.setInfoText(tr("%1 particles at timestep %2").arg(numParticles).arg(timestep));
				return;	// Done!
			}
			else {
				throw Exception(tr("LAMMPS dump file parsing error. Line %1 of file %2 is invalid.").arg(stream.lineNumber()).arg(stream.filename()));
			}
		}
		while(!stream.eof());
	}

	throw Exception(tr("LAMMPS dump file parsing error. Unexpected end of file at line %1.").arg(stream.lineNumber()));
}

};
