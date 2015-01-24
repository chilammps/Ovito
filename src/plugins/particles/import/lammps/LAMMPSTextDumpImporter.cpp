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
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/FileSource.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <plugins/particles/import/InputColumnMappingDialog.h>
#include "LAMMPSTextDumpImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, LAMMPSTextDumpImporter, ParticleImporter);
SET_OVITO_OBJECT_EDITOR(LAMMPSTextDumpImporter, LAMMPSTextDumpImporterEditor);
DEFINE_PROPERTY_FIELD(LAMMPSTextDumpImporter, _useCustomColumnMapping, "UseCustomColumnMaspping");
SET_PROPERTY_FIELD_LABEL(LAMMPSTextDumpImporter, _useCustomColumnMapping, "Custom file column mapping");
OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, LAMMPSTextDumpImporterEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

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
	CompressedTextReader stream(input, sourceLocation.path());

	// Read first line.
	stream.readLine(15);
	if(stream.lineStartsWith("ITEM: TIMESTEP"))
		return true;

	return false;
}

/******************************************************************************
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void LAMMPSTextDumpImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<FileSourceImporter::Frame>& frames, const QUrl& sourceUrl, CompressedTextReader& stream)
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
				Frame frame;
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
void LAMMPSTextDumpImporter::LAMMPSTextDumpImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading LAMMPS dump file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

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
				setTimestep(timestep);
				break;
			}
			else if(stream.lineStartsWith("ITEM: NUMBER OF ATOMS")) {
				// Parse number of atoms.
				unsigned int u;
				if(sscanf(stream.readLine(), "%u", &u) != 1 || u > 1e9)
					throw Exception(tr("LAMMPS dump file parsing error. Invalid number of atoms in line %1:\n%2").arg(stream.lineNumber()).arg(stream.lineString()));

				numParticles = u;
				setProgressRange(u);
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
						_customColumnMapping.resize(columnCount);
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

				// If possible, use memory-mapped file access for best performance.
				const char* s;
				const char* s_end;
				std::tie(s, s_end) = stream.mmap();
				int lineNumber = stream.lineNumber() + 1;
				try {
					for(size_t i = 0; i < numParticles; i++, lineNumber++) {
						if(!reportProgress(i)) return;
						if(!s)
							columnParser.readParticle(i, stream.readLine());
						else
							s = columnParser.readParticle(i, s, s_end);
					}
				}
				catch(Exception& ex) {
					throw ex.prependGeneralMessage(tr("Parsing error in line %1 of LAMMPS dump file.").arg(lineNumber));
				}
				if(s) stream.munmap();

				// Sort the particle type list since we created particles on the go and their order depends on the occurrence of types in the file.
				if(columnParser.usingNamedParticleTypes())
					sortParticleTypesByName();
				else
					sortParticleTypesById();

				// Find out if coordinates are given in reduced format and need to be rescaled to absolute format.
				bool reducedCoordinates = false;
				if(!fileColumnNames.empty()) {
					for(int i = 0; i < (int)columnMapping.size() && i < fileColumnNames.size(); i++) {
						if(columnMapping[i].property.type() == ParticleProperty::PositionProperty) {
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

				setStatus(tr("%1 particles at timestep %2").arg(numParticles).arg(timestep));
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
	columnMapping.resize(columnNames.size());
	for(int i = 0; i < columnNames.size(); i++) {
		QString name = columnNames[i].toLower();
		columnMapping[i].columnName = columnNames[i];
		if(name == "x" || name == "xu" || name == "coordinates") columnMapping[i].mapStandardColumn(ParticleProperty::PositionProperty, 0);
		else if(name == "y" || name == "yu") columnMapping[i].mapStandardColumn(ParticleProperty::PositionProperty, 1);
		else if(name == "z" || name == "zu") columnMapping[i].mapStandardColumn(ParticleProperty::PositionProperty, 2);
		else if(name == "xs" || name == "xsu") { columnMapping[i].mapStandardColumn(ParticleProperty::PositionProperty, 0); }
		else if(name == "ys" || name == "ysu") { columnMapping[i].mapStandardColumn(ParticleProperty::PositionProperty, 1); }
		else if(name == "zs" || name == "zsu") { columnMapping[i].mapStandardColumn(ParticleProperty::PositionProperty, 2); }
		else if(name == "vx" || name == "velocities") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityProperty, 0);
		else if(name == "vy") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityProperty, 1);
		else if(name == "vz") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityProperty, 2);
		else if(name == "id") columnMapping[i].mapStandardColumn(ParticleProperty::IdentifierProperty);
		else if(name == "type" || name == "element" || name == "atom_types") columnMapping[i].mapStandardColumn(ParticleProperty::ParticleTypeProperty);
		else if(name == "mass") columnMapping[i].mapStandardColumn(ParticleProperty::MassProperty);
		else if(name == "radius") columnMapping[i].mapStandardColumn(ParticleProperty::RadiusProperty);
		else if(name == "mol") columnMapping[i].mapStandardColumn(ParticleProperty::MoleculeProperty);
		else if(name == "q") columnMapping[i].mapStandardColumn(ParticleProperty::ChargeProperty);
		else if(name == "ix") columnMapping[i].mapStandardColumn(ParticleProperty::PeriodicImageProperty, 0);
		else if(name == "iy") columnMapping[i].mapStandardColumn(ParticleProperty::PeriodicImageProperty, 1);
		else if(name == "iz") columnMapping[i].mapStandardColumn(ParticleProperty::PeriodicImageProperty, 2);
		else if(name == "fx" || name == "forces") columnMapping[i].mapStandardColumn(ParticleProperty::ForceProperty, 0);
		else if(name == "fy") columnMapping[i].mapStandardColumn(ParticleProperty::ForceProperty, 1);
		else if(name == "fz") columnMapping[i].mapStandardColumn(ParticleProperty::ForceProperty, 2);
		else if(name == "mux") columnMapping[i].mapStandardColumn(ParticleProperty::DipoleOrientationProperty, 0);
		else if(name == "muy") columnMapping[i].mapStandardColumn(ParticleProperty::DipoleOrientationProperty, 1);
		else if(name == "muz") columnMapping[i].mapStandardColumn(ParticleProperty::DipoleOrientationProperty, 2);
		else if(name == "mu") columnMapping[i].mapStandardColumn(ParticleProperty::DipoleMagnitudeProperty);
		else if(name == "omegax") columnMapping[i].mapStandardColumn(ParticleProperty::AngularVelocityProperty, 0);
		else if(name == "omegay") columnMapping[i].mapStandardColumn(ParticleProperty::AngularVelocityProperty, 1);
		else if(name == "omegaz") columnMapping[i].mapStandardColumn(ParticleProperty::AngularVelocityProperty, 2);
		else if(name == "angmomx") columnMapping[i].mapStandardColumn(ParticleProperty::AngularMomentumProperty, 0);
		else if(name == "angmomy") columnMapping[i].mapStandardColumn(ParticleProperty::AngularMomentumProperty, 1);
		else if(name == "angmomz") columnMapping[i].mapStandardColumn(ParticleProperty::AngularMomentumProperty, 2);
		else if(name == "tqx") columnMapping[i].mapStandardColumn(ParticleProperty::TorqueProperty, 0);
		else if(name == "tqy") columnMapping[i].mapStandardColumn(ParticleProperty::TorqueProperty, 1);
		else if(name == "tqz") columnMapping[i].mapStandardColumn(ParticleProperty::TorqueProperty, 2);
		else if(name == "spin") columnMapping[i].mapStandardColumn(ParticleProperty::SpinProperty);
		else if(name == "c_cna" || name == "pattern") columnMapping[i].mapStandardColumn(ParticleProperty::StructureTypeProperty);
		else if(name == "c_epot") columnMapping[i].mapStandardColumn(ParticleProperty::PotentialEnergyProperty);
		else if(name == "c_kpot") columnMapping[i].mapStandardColumn(ParticleProperty::KineticEnergyProperty);
		else if(name == "c_stress[1]") columnMapping[i].mapStandardColumn(ParticleProperty::StressTensorProperty, 0);
		else if(name == "c_stress[2]") columnMapping[i].mapStandardColumn(ParticleProperty::StressTensorProperty, 1);
		else if(name == "c_stress[3]") columnMapping[i].mapStandardColumn(ParticleProperty::StressTensorProperty, 2);
		else if(name == "c_stress[4]") columnMapping[i].mapStandardColumn(ParticleProperty::StressTensorProperty, 3);
		else if(name == "c_stress[5]") columnMapping[i].mapStandardColumn(ParticleProperty::StressTensorProperty, 4);
		else if(name == "c_stress[6]") columnMapping[i].mapStandardColumn(ParticleProperty::StressTensorProperty, 5);
		else if(name == "selection") columnMapping[i].mapStandardColumn(ParticleProperty::SelectionProperty, 0);
		else {
			columnMapping[i].mapCustomColumn(name, qMetaTypeId<FloatType>());
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
	FileSource* obj = nullptr;
	for(RefMaker* refmaker : dependents()) {
		obj = dynamic_object_cast<FileSource>(refmaker);
		if(obj) break;
	}
	if(!obj) return;

	// Inspect the file header to determine the number of data columns.
	std::shared_ptr<LAMMPSTextDumpImportTask> inspectionTask = std::make_shared<LAMMPSTextDumpImportTask>(dataset()->container(), obj->frames().front());
	try {
		if(!dataset()->container()->taskManager().runTask(inspectionTask))
			return;
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}

	InputColumnMapping mapping;
	if(_customColumnMapping.empty())
		mapping = inspectionTask->columnMapping();
	else {
		mapping = _customColumnMapping;
		mapping.resize(inspectionTask->columnMapping().size());
		for(size_t i = 0; i < mapping.size(); i++)
			mapping[i].columnName = inspectionTask->columnMapping()[i].columnName;
	}

	InputColumnMappingDialog dialog(mapping, parent);
	if(dialog.exec() == QDialog::Accepted) {
		setCustomColumnMapping(dialog.mapping());
		setUseCustomColumnMapping(true);
		requestReload();
	}
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

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
	connect(editMappingButton, &QPushButton::clicked, this, &LAMMPSTextDumpImporterEditor::onEditColumnMapping);
}

/******************************************************************************
* Is called when the user pressed the "Edit column mapping" button.
******************************************************************************/
void LAMMPSTextDumpImporterEditor::onEditColumnMapping()
{
	if(LAMMPSTextDumpImporter* importer = static_object_cast<LAMMPSTextDumpImporter>(editObject()))
		importer->showEditColumnMappingDialog(mainWindow());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
