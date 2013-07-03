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
#include "LAMMPSTextDumpImporter.h"
#include "LAMMPSDumpImporterSettingsDialog.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, LAMMPSTextDumpImporter, AtomsImporter)
DEFINE_PROPERTY_FIELD(LAMMPSTextDumpImporter, _isMultiTimestepFile, "IsMultiTimestepFile")

/******************************************************************************
* Opens the settings dialog for this importer.
******************************************************************************/
bool LAMMPSTextDumpImporter::showSettingsDialog(QWidget* parent)
{
	LAMMPSDumpImporterSettingsDialog dialog(this, parent);
	if(dialog.exec() != QDialog::Accepted)
		return false;
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

	int timestep;
	unsigned int numAtoms = 0;

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
				if(sscanf(stream.readLine().constData(), "%u", &numAtoms) != 1 || numAtoms > 1e9)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid number of atoms in line %1:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));

				futureInterface.setProgressRange(numAtoms);
				break;
			}
			else if(stream.line().startsWith("ITEM: BOX BOUNDS xy xz yz")) {
				// Parse optional boundary condition flags.
				std::istringstream ss(std::string(stream.line()).substr(qstrlen("ITEM: BOX BOUNDS xy xz yz")));
				std::string pbcx, pbcy, pbcz;
				ss >> pbcx >> pbcy >> pbcz;
				if(pbcx.length() == 2 && pbcy.length() == 2 && pbcz.length() == 2)
					container.setPbcFlags(pbcx == "pp", pbcy == "pp", pbcz == "pp");

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
						simBox.minc));
				break;
			}
			else if(stream.line().startsWith("ITEM: BOX BOUNDS")) {
				// Parse optional boundary condition flags.
				istringstream ss(stream.line().substr(qstrlen("ITEM: BOX BOUNDS")));
				string pbcx, pbcy, pbcz;
				ss >> pbcx >> pbcy >> pbcz;
				if(pbcx.length() == 2 && pbcy.length() == 2 && pbcz.length() == 2)
					destination->simulationCell()->setPeriodicity(pbcx == "pp", pbcy == "pp", pbcz == "pp");

				// Parse orthogonal simulation box size.
				for(size_t k=0; k<3; k++) {
					if(sscanf(stream.readline().c_str(), FLOAT_SCANF_STRING_2, &simBox.minc[k], &simBox.maxc[k]) != 2)
						throw Exception(tr("Invalid box size in line %1 of dump file: %2").arg(stream.lineNumber()).arg(stream.line().c_str()));
				}
				destination->simulationCell()->setBoxShape(simBox);
				break;
			}
			else if(stream.line().startsWith("ITEM: ATOMS")) {
				// Parse atom coordinates.
				destination->setAtomsCount(numAtoms);

				// Prepare the mapping between input file columns and data channels.
				DataRecordParserHelper recordParser(&columnMapping(), destination);

				// Parse one atom per line.
				for(int i = 0; i < numAtoms; i++) {

					// Update progress indicator.
					if((i % 4096) == 0) {
						progress.setValue(i);
						if(progress.isCanceled()) return EvaluationStatus(EvaluationStatus::EVALUATION_ERROR);
					}

					stream.readline();
					try {
						recordParser.storeAtom(i, (char*)stream.line().c_str());
					}
					catch(Exception& ex) {
						throw ex.prependGeneralMessage(tr("Parsing error in line %1 of LAMMPS dump file.").arg(stream.lineNumber()));
					}
				}

				if(recordParser.coordinatesOutOfRange()) {
					MsgLogger() << "WARNING: At least some of the atomic coordinates are out of the valid range." << endl;
					if(APPLICATION_MANAGER.guiMode() && QMessageBox::warning(NULL, tr("Warning"), tr("At least some of the atomic coordinates are out of the valid range. Do you want to ignore this and still load the dataset?"),
							QMessageBox::Ignore | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Cancel)
					return EvaluationStatus(EvaluationStatus::EVALUATION_ERROR);
				}

				DataChannel* posChannel = destination->getStandardDataChannel(DataChannel::PositionChannel);
				if(posChannel && posChannel->size() > 1) {

					// Rescale atoms if they are stored in reduced coordinates.

					// Check if the input pos column is named "xs", "ys", or "zs".
					bool areReducedCoordinates = false;
					for(int i = 0; i < columnMapping().columnCount(); i++) {
						if(columnMapping().getChannelId(i) == DataChannel::PositionChannel && columnMapping().getColumnName(i) == "xs") {
							areReducedCoordinates = true;
							break;
						}
					}
					// Alternatively, check if all atom coordinates are in the [0:1] internal.
					// Add some extra margin here, because atoms may have moved outside the simulation box slightly.
					if(!areReducedCoordinates) {
						const Box3& boundingBox = recordParser.boundingBox();
						if(Box3(Point3(-0.1f), Point3(1.1f)).containsBox(boundingBox))
							areReducedCoordinates = true;
					}

					if(areReducedCoordinates) {
						VerboseLogger() << "Rescaling reduced coordinates in interval [0,1] to box size." << endl;
						AffineTransformation simCell = destination->simulationCell()->cellMatrix();
						Point3* p = posChannel->dataPoint3();
						for(size_t i = posChannel->size(); i != 0; --i, ++p)
							*p = simCell * (*p);
					}
				}
				destination->invalidate();

				QString statusMessage = tr("%1 atoms at timestep %2").arg(numAtoms).arg(timeStep);
				return EvaluationStatus(EvaluationStatus::EVALUATION_SUCCESS, statusMessage);
			}
			else {
				throw Exception(tr("LAMMPS dump file parsing error. Line %1 is invalid:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));
			}
		}
		while(!stream.eof());
	}

	for(int i = 0; i <= 100 && !futureInterface.isCanceled(); i++) {
		futureInterface.setProgressValue(i);
		QThread::msleep(10);
	}

	container.setSimulationCell(AffineTransformation(
			Vector3(20,0,0), Vector3(0,10,0), Vector3(0,0,10), Vector3(-10,-5,-5)));

	QExplicitlySharedDataPointer<ParticleProperty> posProperty(new ParticleProperty(ParticleProperty::PositionProperty));
	posProperty->resize(numAtoms);
	static std::default_random_engine rng;
	std::uniform_real_distribution<FloatType> unitDistribution;
	for(size_t i = 0; i < numAtoms; i++) {
		Point3 pos(unitDistribution(rng), unitDistribution(rng), unitDistribution(rng));
		posProperty->setPoint3(i, container.simulationCell() * pos);
	}
	container.addParticleProperty(posProperty);
}

};
