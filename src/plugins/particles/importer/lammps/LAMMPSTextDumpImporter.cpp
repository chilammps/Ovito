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
#include <core/utilities/concurrent/Task.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <plugins/particles/importer/InputColumnMappingDialog.h>
#include "LAMMPSTextDumpImporter.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, LAMMPSTextDumpImporter, ParticleImporter)
IMPLEMENT_OVITO_OBJECT(Particles, LAMMPSTextDumpImporterEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(LAMMPSTextDumpImporter, LAMMPSTextDumpImporterEditor)
DEFINE_PROPERTY_FIELD(LAMMPSTextDumpImporter, _useCustomColumnMapping, "UseCustomColumnMapping")
SET_PROPERTY_FIELD_LABEL(LAMMPSTextDumpImporter, _useCustomColumnMapping, "Custom file column mapping")

/******************************************************************************
 * Sets the user-defined mapping between data columns in the input file and
 * the internal particle properties.
 *****************************************************************************/
void LAMMPSTextDumpImporter::setCustomColumnMapping(const InputColumnMapping& mapping)
{
	_customColumnMapping = mapping;

#if 0
	if(Application::instance().guiMode()) {
		// Remember the mapping for the next time.
		QSettings settings;
		settings.beginGroup("atomviz/io/columnmapping/");
		settings.setValue(pluginClassDescriptor()->name(), mapping.toByteArray());
		OVITO_ASSERT(settings.contains(pluginClassDescriptor()->name()));
		settings.endGroup();
	}
#endif

	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool LAMMPSTextDumpImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextParserStream stream(input, sourceLocation.path());

	// Read first line.
	stream.readLine(15);
	if(stream.lineStartsWith("ITEM: TIMESTEP"))
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
	QFileInfo fileInfo(stream.device().fileName());
	QString filename = fileInfo.fileName();
	QDateTime lastModified = fileInfo.lastModified();

	while(!stream.eof()) {
		qint64 byteOffset = stream.byteOffset();

		// Parse next line.
		stream.readLine();

		do {
			int startLineNumber = stream.lineNumber();
			if(stream.lineStartsWith("ITEM: TIMESTEP")) {
				if(sscanf(stream.readLine(), "%i", &timestep) != 1)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid timestep number (line %1):\n%2").arg(stream.lineNumber()).arg(QString::fromLocal8Bit(stream.line())));
				FrameSourceInformation frame;
				frame.sourceFile = sourceUrl;
				frame.byteOffset = byteOffset;
				frame.lineNumber = startLineNumber;
				frame.lastModificationTime = lastModified;
				frame.label = QString("Timestep %1").arg(timestep);
				frames.push_back(frame);
				break;
			}
			else if(stream.lineStartsWith("ITEM: NUMBER OF ATOMS")) {
				// Parse number of atoms.
				unsigned int u;
				if(sscanf(stream.readLine(), "%u", &u) != 1 || u > 1e9)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid number of atoms in line %1:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));
				numParticles = u;
				break;
			}
			else if(stream.lineStartsWith("ITEM: ATOMS")) {
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
			else if(stream.lineStartsWith("ITEM:")) {
				// Skip lines up to next ITEM:
				while(!stream.eof()) {
					byteOffset = stream.byteOffset();
					stream.readLine();
					if(stream.lineStartsWith("ITEM:"))
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
void LAMMPSTextDumpImporter::LAMMPSTextDumpImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Reading LAMMPS dump file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	int timestep;
	size_t numParticles = 0;

	while(!stream.eof()) {

		// Parse next line.
		stream.readLine();

		do {
			if(stream.lineStartsWith("ITEM: TIMESTEP")) {
				if(sscanf(stream.readLine(), "%i", &timestep) != 1)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid timestep number (line %1):\n%2").arg(stream.lineNumber()).arg(QString::fromLocal8Bit(stream.line())));
				break;
			}
			else if(stream.lineStartsWith("ITEM: NUMBER OF ATOMS")) {
				// Parse number of atoms.
				unsigned int u;
				if(sscanf(stream.readLine(), "%u", &u) != 1 || u > 1e9)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid number of atoms in line %1:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));

				numParticles = u;
				futureInterface.setProgressRange(u);
				break;
			}
			else if(stream.lineStartsWith("ITEM: BOX BOUNDS xy xz yz")) {

				// Parse optional boundary condition flags.
				QStringList tokens = stream.lineString().mid(qstrlen("ITEM: BOX BOUNDS xy xz yz")).split(ws_re, QString::SkipEmptyParts);
				if(tokens.size() >= 3)
					simulationCell().setPbcFlags(tokens[0] == "pp", tokens[1] == "pp", tokens[2] == "pp");

				// Parse triclinic simulation box.
				FloatType tiltFactors[3];
				Box3 simBox;
				for(int k = 0; k < 3; k++) {
					if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &simBox.minc[k], &simBox.maxc[k], &tiltFactors[k]) != 3)
						throw Exception(tr("Invalid box size in line %1 of LAMMPS dump file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
				}

				// LAMMPS only stores the outer bounding box of the simulation cell in the dump file.
				// We have to determine the size of the actual triclinic cell.
				simBox.minc.x() -= std::min(std::min(std::min(tiltFactors[0], tiltFactors[1]), tiltFactors[0]+tiltFactors[1]), (FloatType)0);
				simBox.maxc.x() -= std::max(std::max(std::max(tiltFactors[0], tiltFactors[1]), tiltFactors[0]+tiltFactors[1]), (FloatType)0);
				simBox.minc.y() -= std::min(tiltFactors[2], (FloatType)0);
				simBox.maxc.y() -= std::max(tiltFactors[2], (FloatType)0);
				simulationCell().setMatrix(AffineTransformation(
						Vector3(simBox.sizeX(), 0, 0),
						Vector3(tiltFactors[0], simBox.sizeY(), 0),
						Vector3(tiltFactors[1], tiltFactors[2], simBox.sizeZ()),
						simBox.minc - Point3::Origin()));
				break;
			}
			else if(stream.lineStartsWith("ITEM: BOX BOUNDS")) {
				// Parse optional boundary condition flags.
				QStringList tokens = stream.lineString().mid(qstrlen("ITEM: BOX BOUNDS")).split(ws_re, QString::SkipEmptyParts);
				if(tokens.size() >= 3)
					simulationCell().setPbcFlags(tokens[0] == "pp", tokens[1] == "pp", tokens[2] == "pp");

				// Parse orthogonal simulation box size.
				Box3 simBox;
				for(int k = 0; k < 3; k++) {
					if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &simBox.minc[k], &simBox.maxc[k]) != 2)
						throw Exception(tr("Invalid box size in line %1 of dump file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
				}

				simulationCell().setMatrix(AffineTransformation(
						Vector3(simBox.sizeX(), 0, 0),
						Vector3(0, simBox.sizeY(), 0),
						Vector3(0, 0, simBox.sizeZ()),
						simBox.minc - Point3::Origin()));
				break;
			}
			else if(stream.lineStartsWith("ITEM: ATOMS")) {

				// Read the column names list.
				QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
				OVITO_ASSERT(tokens[0] == "ITEM:" && tokens[1] == "ATOMS");
				QStringList fileColumnNames = tokens.mid(2);

				// Stop here if we are only inspecting the file's header.
				if(_parseFileHeaderOnly) {
					if(fileColumnNames.isEmpty()) {
						// If no file columns names are available, count at least the number
						// of data columns.
						stream.readLine();
						int columnCount = stream.lineString().split(ws_re, QString::SkipEmptyParts).size();
						_customColumnMapping.setColumnCount(columnCount);
					}
					else {
						_customColumnMapping = generateAutomaticColumnMapping(fileColumnNames);
					}
					return;
				}

				// Set up column-to-property mapping.
				InputColumnMapping columnMapping;
				if(_useCustomColumnMapping)
					columnMapping = _customColumnMapping;
				else
					columnMapping = generateAutomaticColumnMapping(fileColumnNames);

				// Parse data columns.
				InputColumnReader columnParser(columnMapping, *this, numParticles);
				try {
					for(size_t i = 0; i < numParticles; i++) {
						if((i % 4096) == 0) {
							if(futureInterface.isCanceled())
								return;	// Abort!
							futureInterface.setProgressValue((int)i);
						}
						stream.readLine();
						columnParser.readParticle(i, const_cast<char*>(stream.line()));
					}
				}
				catch(Exception& ex) {
					throw ex.prependGeneralMessage(tr("Parsing error in line %1 of LAMMPS dump file.").arg(stream.lineNumber()));
				}

				// Find out if coordinates are given in reduced format and need to be rescaled to absolute format.
				bool reducedCoordinates = false;
				if(!fileColumnNames.empty()) {
					for(int i = 0; i < columnMapping.columnCount() && i < fileColumnNames.size(); i++) {
						if(columnMapping.propertyType(i) == ParticleProperty::PositionProperty) {
							reducedCoordinates = (
									fileColumnNames[i] == "xs" || fileColumnNames[i] == "xsu" ||
									fileColumnNames[i] == "ys" || fileColumnNames[i] == "ysu" ||
									fileColumnNames[i] == "zs" || fileColumnNames[i] == "zsu");
						}
					}
				}
				else {
					// Check if all atom coordinates are within the [0,1] interval.
					// If yes, we assume reduced coordinate format.
					ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
					if(posProperty) {
						Box3 boundingBox;
						boundingBox.addPoints(posProperty->constDataPoint3(), posProperty->size());
						if(Box3(Point3(-0.02f), Point3(1.02f)).containsBox(boundingBox))
							reducedCoordinates = true;
					}
				}

				if(reducedCoordinates) {
					// Convert all atom coordinates from reduced to absolute (Cartesian) format.
					ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
					if(posProperty) {
						const AffineTransformation simCell = simulationCell().matrix();
						Point3* p = posProperty->dataPoint3();
						Point3* p_end = p + posProperty->size();
						for(; p != p_end; ++p)
							*p = simCell * (*p);
					}
				}

				setInfoText(tr("%1 particles at simulation timestep %2").arg(numParticles).arg(timestep));
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

/******************************************************************************
 * Guesses the mapping of input file columns to internal particle properties.
 *****************************************************************************/
InputColumnMapping LAMMPSTextDumpImporter::generateAutomaticColumnMapping(const QStringList& columnNames)
{
	InputColumnMapping columnMapping;
	for(int i = 0; i < columnNames.size(); i++) {
		QString name = columnNames[i].toLower();
		if(name == "x" || name == "xu" || name == "coordinates") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 0, name);
		else if(name == "y" || name == "yu") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 1, name);
		else if(name == "z" || name == "zu") columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 2, name);
		else if(name == "xs" || name == "xsu") { columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 0, name); }
		else if(name == "ys" || name == "ysu") { columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 1, name); }
		else if(name == "zs" || name == "zsu") { columnMapping.mapStandardColumn(i, ParticleProperty::PositionProperty, 2, name); }
		else if(name == "vx" || name == "velocities") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 0, name);
		else if(name == "vy") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 1, name);
		else if(name == "vz") columnMapping.mapStandardColumn(i, ParticleProperty::VelocityProperty, 2, name);
		else if(name == "id") columnMapping.mapStandardColumn(i, ParticleProperty::IdentifierProperty, 0, name);
		else if(name == "type" || name == "element" || name == "atom_types") columnMapping.mapStandardColumn(i, ParticleProperty::ParticleTypeProperty, 0, name);
		else if(name == "mass") columnMapping.mapStandardColumn(i, ParticleProperty::MassProperty, 0, name);
		else if(name == "radius") columnMapping.mapStandardColumn(i, ParticleProperty::RadiusProperty, 0, name);
		else if(name == "q") columnMapping.mapStandardColumn(i, ParticleProperty::ChargeProperty, 0, name);
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
		else {
			columnMapping.mapCustomColumn(i, name, qMetaTypeId<FloatType>(), 0, ParticleProperty::UserProperty, name);
		}
	}
	return columnMapping;
}

/******************************************************************************
 * Saves the class' contents to the given stream.
 *****************************************************************************/
void LAMMPSTextDumpImporter::saveToStream(ObjectSaveStream& stream)
{
	ParticleImporter::saveToStream(stream);

	stream.beginChunk(0x01);
	_customColumnMapping.saveToStream(stream);
	stream.endChunk();
}

/******************************************************************************
 * Loads the class' contents from the given stream.
 *****************************************************************************/
void LAMMPSTextDumpImporter::loadFromStream(ObjectLoadStream& stream)
{
	ParticleImporter::loadFromStream(stream);

	stream.expectChunk(0x01);
	_customColumnMapping.loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
 * Creates a copy of this object.
 *****************************************************************************/
OORef<RefTarget> LAMMPSTextDumpImporter::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<LAMMPSTextDumpImporter> clone = static_object_cast<LAMMPSTextDumpImporter>(ParticleImporter::clone(deepCopy, cloneHelper));
	clone->_customColumnMapping = this->_customColumnMapping;
	return clone;
}

/******************************************************************************
 * Displays a dialog box that allows the user to edit the custom file column to particle
 * property mapping.
 *****************************************************************************/
void LAMMPSTextDumpImporter::showEditColumnMappingDialog(QWidget* parent)
{
	// Retrieve column names from current input file.
	LinkedFileObject* obj = nullptr;
	for(RefMaker* refmaker : dependents()) {
		obj = dynamic_object_cast<LinkedFileObject>(refmaker);
		if(obj) break;
	}
	if(!obj) return;

	// Start task that inspects the file header to determine the number of data columns.
	std::unique_ptr<LAMMPSTextDumpImportTask> inspectionTask(new LAMMPSTextDumpImportTask(obj->frames().front()));
	DataSetContainer& datasetContainer = *dataset()->container();
	Future<void> future = datasetContainer.taskManager().runInBackground<void>(std::bind(&LAMMPSTextDumpImportTask::load,
			inspectionTask.get(), std::ref(datasetContainer), std::placeholders::_1));
	if(!datasetContainer.taskManager().waitForTask(future))
		return;

	try {
		// This is to detect if an error has occurred.
		future.result();
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}

	InputColumnMapping mapping;
	if(_customColumnMapping.columnCount() == 0)
		mapping = inspectionTask->columnMapping();
	else {
		mapping = _customColumnMapping;
		mapping.setColumnCount(inspectionTask->columnMapping().columnCount());
		for(int i = 0; i < mapping.columnCount(); i++)
			mapping.setColumnName(i, inspectionTask->columnMapping().columnName(i));
	}

	InputColumnMappingDialog dialog(mapping, parent);
	if(dialog.exec() == QDialog::Accepted) {
		setCustomColumnMapping(dialog.mapping());
		setUseCustomColumnMapping(true);
		requestReload();
	}
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void LAMMPSTextDumpImporterEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("LAMMPS dump file"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGroupBox* animFramesBox = new QGroupBox(tr("Timesteps"), rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(animFramesBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(animFramesBox);

	// Multi-timestep file
	BooleanParameterUI* multitimestepUI = new BooleanParameterUI(this, PROPERTY_FIELD(ParticleImporter::_isMultiTimestepFile));
	sublayout->addWidget(multitimestepUI->checkBox());

	QGroupBox* columnMappingBox = new QGroupBox(tr("File columns"), rollout);
	sublayout = new QVBoxLayout(columnMappingBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(columnMappingBox);

	BooleanRadioButtonParameterUI* useCustomMappingUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(LAMMPSTextDumpImporter::_useCustomColumnMapping));
	useCustomMappingUI->buttonFalse()->setText(tr("Automatic mapping"));
	sublayout->addWidget(useCustomMappingUI->buttonFalse());
	useCustomMappingUI->buttonTrue()->setText(tr("User-defined mapping to particle properties"));
	sublayout->addWidget(useCustomMappingUI->buttonTrue());

	QPushButton* editMappingButton = new QPushButton(tr("Edit column mapping..."));
	sublayout->addWidget(editMappingButton);
	connect(editMappingButton, SIGNAL(clicked(bool)), this, SLOT(onEditColumnMapping()));
}

/******************************************************************************
* Is called when the user pressed the "Edit column mapping" button.
******************************************************************************/
void LAMMPSTextDumpImporterEditor::onEditColumnMapping()
{
	if(LAMMPSTextDumpImporter* importer = static_object_cast<LAMMPSTextDumpImporter>(editObject()))
		importer->showEditColumnMappingDialog(mainWindow());
}

};
