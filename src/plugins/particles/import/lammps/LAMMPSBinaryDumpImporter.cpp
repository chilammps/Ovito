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
#include <plugins/particles/import/InputColumnMappingDialog.h>
#include "LAMMPSBinaryDumpImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, LAMMPSBinaryDumpImporter, ParticleImporter);
SET_OVITO_OBJECT_EDITOR(LAMMPSBinaryDumpImporter, LAMMPSBinaryDumpImporterEditor);
OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, LAMMPSBinaryDumpImporterEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

struct LAMMPSBinaryDumpHeader
{
	LAMMPSBinaryDumpHeader() :
		ntimestep(-1), natoms(-1),
		size_one(-1), nchunk(-1) {
			memset(boundary, 0, sizeof(boundary));
			memset(tiltFactors, 0, sizeof(tiltFactors));
		}

	int ntimestep;
	int natoms;
	int boundary[3][2];
	double bbox[3][2];
	double tiltFactors[3];
	int size_one;
	int nchunk;

	enum LAMMPSDataType {
		LAMMPS_SMALLSMALL,
		LAMMPS_SMALLBIG,
		LAMMPS_BIGBIG,
	};
	LAMMPSDataType dataType;

	bool parse(QIODevice& input);
	int readBigInt(QIODevice& input) {
		if(dataType == LAMMPS_SMALLSMALL) {
			int val;
			input.read(reinterpret_cast<char*>(&val), sizeof(val));
			return val;
		}
		else {
			qint64 val;
			input.read(reinterpret_cast<char*>(&val), sizeof(val));
			return val;
		}
	}
};

/******************************************************************************
 * Sets the user-defined mapping between data columns in the input file and
 * the internal particle properties.
 *****************************************************************************/
void LAMMPSBinaryDumpImporter::setColumnMapping(const InputColumnMapping& mapping)
{
	_columnMapping = mapping;

	if(Application::instance().guiMode()) {
		// Remember the mapping for the next time.
		QSettings settings;
		settings.beginGroup("viz/importer/lammps_binary_dump/");
		settings.setValue("columnmapping", mapping.toByteArray());
		settings.endGroup();
	}

	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool LAMMPSBinaryDumpImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	if(!input.open(QIODevice::ReadOnly))
		return false;

	LAMMPSBinaryDumpHeader header;
	return header.parse(input);
}

/******************************************************************************
*  This method is called by the FileSource each time a new source
* file has been selected by the user.
******************************************************************************/
bool LAMMPSBinaryDumpImporter::inspectNewFile(FileSource* obj)
{
	if(!ParticleImporter::inspectNewFile(obj))
		return false;

	if(obj->frames().empty())
		return false;

	// Don't show column mapping dialog in console mode.
	if(Application::instance().consoleMode())
		return true;

	// Start task that inspects the file header to determine the number of data columns.
	std::shared_ptr<LAMMPSBinaryDumpImportTask> inspectionTask = std::make_shared<LAMMPSBinaryDumpImportTask>(dataset()->container(), obj->frames().front());
	if(!dataset()->container()->taskManager().runTask(inspectionTask))
		return false;

	InputColumnMapping mapping(_columnMapping);
	mapping.resize(inspectionTask->columnMapping().size());
	mapping.setFileExcerpt(inspectionTask->columnMapping().fileExcerpt());
	if(_columnMapping.size() != mapping.size()) {
		if(_columnMapping.empty()) {
			size_t oldCount = 0;

			// Load last mapping from settings store.
			QSettings settings;
			settings.beginGroup("viz/importer/lammps_binary_dump/");
			if(settings.contains("columnmapping")) {
				try {
					mapping.fromByteArray(settings.value("columnmapping").toByteArray());
					oldCount = mapping.size();
				}
				catch(Exception& ex) {
					ex.prependGeneralMessage(tr("Failed to load last used column-to-property mapping from application settings store."));
					ex.logError();
				}
			}

			mapping.resize(inspectionTask->columnMapping().size());
		}

		InputColumnMappingDialog dialog(mapping, dataset()->mainWindow());
		if(dialog.exec() == QDialog::Accepted) {
			setColumnMapping(dialog.mapping());
			return true;
		}
		return false;
	}
	else {
		_columnMapping.setFileExcerpt(inspectionTask->columnMapping().fileExcerpt());
		return true;
	}
}

/******************************************************************************
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void LAMMPSBinaryDumpImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<FileSourceImporter::Frame>& frames, const QUrl& sourceUrl, CompressedTextReader& stream)
{
	futureInterface.setProgressText(tr("Scanning binary LAMMPS dump file %1").arg(stream.filename()));
	futureInterface.setProgressRange(stream.underlyingSize() / 1000);

	// First close text stream so we can re-open it in binary mode.
	QIODevice& file = stream.device();
	file.close();

	// Open input file for reading.
	if(!file.open(QIODevice::ReadOnly))
		throw Exception(tr("Failed to open binary LAMMPS dump file: %1.").arg(file.errorString()));

	QFileInfo fileInfo(stream.device().fileName());
	QString filename = fileInfo.fileName();
	QDateTime lastModified = fileInfo.lastModified();

	while(!file.atEnd()) {
		qint64 byteOffset = file.pos();

		// Parse file header.
		LAMMPSBinaryDumpHeader header;
		if(!header.parse(file))
			throw Exception(tr("Failed to read binary LAMMPS dump file: Invalid file header."));

		// Skip particle data.
		qint64 filePos = file.pos();
		for(int chunki = 0; chunki < header.nchunk; chunki++) {

			// Read chunk size.
			int n = -1;
			if(file.read(reinterpret_cast<char*>(&n), sizeof(n)) != sizeof(n) || n < 0 || n > header.natoms * header.size_one)
				throw Exception(tr("Invalid data chunk size: %1").arg(n));

			// Skip chunk data.
			filePos += sizeof(n) + n * sizeof(double);
			if(!file.seek(filePos))
				throw Exception(tr("Unexpected end of file."));

			futureInterface.setProgressValue(filePos / 1000);
			if(futureInterface.isCanceled())
				return;
		}

		// Create a new record for the time step.
		Frame frame;
		frame.sourceFile = sourceUrl;
		frame.byteOffset = byteOffset;
		frame.lineNumber = 0;
		frame.lastModificationTime = lastModified;
		frame.label = QString("Timestep %1").arg(header.ntimestep);
		frames.push_back(frame);
	}
}

/******************************************************************************
* Parses the file header of a binary LAMMPS dump file.
******************************************************************************/
bool LAMMPSBinaryDumpHeader::parse(QIODevice& input)
{
	qint64 headerPos = input.pos();
	for(int dataTypeIndex = LAMMPS_SMALLSMALL; dataTypeIndex <= LAMMPS_BIGBIG; dataTypeIndex++) {
		dataType = (LAMMPSDataType)dataTypeIndex;
		input.seek(headerPos);

		ntimestep = readBigInt(input);
		if(ntimestep < 0 || ntimestep > 200000000) continue;

		natoms = readBigInt(input);
		if(natoms < 0 || natoms > 200000000) continue;

		qint64 startPos = input.pos();

		// Try the new format first.
		int triclinic = -1;
		input.read(reinterpret_cast<char*>(&triclinic), sizeof(triclinic));
		input.read(reinterpret_cast<char*>(&boundary[0][0]), sizeof(boundary));
		bool isValid = true;
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 2; j++) {
				if(boundary[i][j] < 0 || boundary[i][j] > 3)
					isValid = false;
			}
		}

		if(!isValid) {
			// Try the old format.
			input.seek(startPos);
			isValid = true;
			triclinic = -1;
		}

		// Read bounding box.
		input.read(reinterpret_cast<char*>(&bbox[0][0]), sizeof(bbox));
		for(int i = 0; i < 3; i++) {
			if(bbox[i][0] > bbox[i][1])
				isValid = false;
			for(int j = 0; j < 2; j++) {
				if(!std::isfinite(bbox[i][j]) || bbox[i][j] < -1e10 || bbox[i][j] > 1e10)
					isValid = false;
			}
		}
		if(!isValid)
			continue;

		// Try to read shear parameters of triclinic cell.
		if(triclinic != 0) {
			startPos = input.pos();
			input.read(reinterpret_cast<char*>(&tiltFactors[0]), sizeof(tiltFactors));
			for(int i = 0; i < 3; i++) {
				if(!std::isfinite(tiltFactors[i]) || tiltFactors[i] < bbox[i][0] - bbox[i][1] || tiltFactors[i] > bbox[i][1] - bbox[i][0])
					isValid = false;
			}
			if(!isValid) {
				input.seek(startPos);
				tiltFactors[0] = tiltFactors[1] = tiltFactors[2] = 0;
			}
		}

		input.read(reinterpret_cast<char*>(&size_one), sizeof(size_one));
		if(size_one <= 0 || size_one > 40) continue;

		input.read(reinterpret_cast<char*>(&nchunk), sizeof(nchunk));
		if(nchunk <= 0 || nchunk > natoms) continue;

		if(!input.atEnd())
			return true;
	}
	return false;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void LAMMPSBinaryDumpImporter::LAMMPSBinaryDumpImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading binary LAMMPS dump file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// First close text stream so we can re-open it in binary mode.
	QIODevice& file = stream.device();
	file.close();

	// Open input file for reading.
	if(!file.open(QIODevice::ReadOnly))
		throw Exception(tr("Failed to open binary LAMMPS dump file: %1.").arg(file.errorString()));

	// Seek to byte offset.
	if(frame().byteOffset && !file.seek(frame().byteOffset))
		throw Exception(tr("Failed to read binary LAMMPS dump file: Could not jump to start byte offset."));

	// Parse file header.
	LAMMPSBinaryDumpHeader header;
	if(!header.parse(file))
		throw Exception(tr("Failed to read binary LAMMPS dump file: Invalid file header."));
	setTimestep(header.ntimestep);

	if(_parseFileHeaderOnly) {
		_columnMapping.resize(header.size_one);
		return;
	}

	setProgressRange(header.natoms);

	// LAMMPS only stores the outer bounding box of the simulation cell in the dump file.
	// We have to determine the size of the actual triclinic cell.
	Box3 simBox;
	simBox.minc = Point3(header.bbox[0][0], header.bbox[1][0], header.bbox[2][0]);
	simBox.maxc = Point3(header.bbox[0][1], header.bbox[1][1], header.bbox[2][1]);
	simBox.minc.x() -= std::min(std::min(std::min(header.tiltFactors[0], header.tiltFactors[1]), header.tiltFactors[0]+header.tiltFactors[1]), 0.0);
	simBox.maxc.x() -= std::max(std::max(std::max(header.tiltFactors[0], header.tiltFactors[1]), header.tiltFactors[0]+header.tiltFactors[1]), 0.0);
	simBox.minc.y() -= std::min(header.tiltFactors[2], 0.0);
	simBox.maxc.y() -= std::max(header.tiltFactors[2], 0.0);
	simulationCell().setMatrix(AffineTransformation(
			Vector3(simBox.sizeX(), 0, 0),
			Vector3(header.tiltFactors[0], simBox.sizeY(), 0),
			Vector3(header.tiltFactors[1], header.tiltFactors[2], simBox.sizeZ()),
			simBox.minc - Point3::Origin()));
	simulationCell().setPbcFlags(header.boundary[0][0] == 0, header.boundary[1][0] == 0, header.boundary[2][0] == 0);

	// Parse particle data.
	InputColumnReader columnParser(_columnMapping, *this, header.natoms);
	try {
		QVector<double> chunkData;
		int i = 0;
		for(int chunki = 0; chunki < header.nchunk; chunki++) {

			// Read chunk size.
			int n = -1;
			if(file.read(reinterpret_cast<char*>(&n), sizeof(n)) != sizeof(n) || n < 0 || n > header.natoms * header.size_one)
				throw Exception(tr("Invalid data chunk size: %1").arg(n));
			if(n == 0)
				continue;

			// Read chunk data.
			chunkData.resize(n);
			if(file.read(reinterpret_cast<char*>(chunkData.data()), n * sizeof(double)) != n * sizeof(double))
				throw Exception(tr("Unexpected end of file."));

			const double* iter = chunkData.constData();
			for(int nChunkAtoms = n / header.size_one; nChunkAtoms--; ++i, iter += header.size_one) {

				// Update progress indicator.
				if(!reportProgress(i)) return;

				try {
					columnParser.readParticle(i, iter, header.size_one);
				}
				catch(Exception& ex) {
					throw ex.prependGeneralMessage(tr("Parsing error in LAMMPS binary dump file."));
				}
			}
		}
	}
	catch(Exception& ex) {
		throw ex.prependGeneralMessage(tr("Parsing error at byte offset %1 of binary LAMMPS dump file.").arg(file.pos()));
	}

	// Sort the particle type list since we created particles on the go and their order depends on the occurrence of types in the file.
	sortParticleTypesById();

	ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
	if(posProperty && posProperty->size() > 0) {
		Box3 boundingBox;
		boundingBox.addPoints(posProperty->constDataPoint3(), posProperty->size());

		// Find out if coordinates are given in reduced format and need to be rescaled to absolute format.
		// Check if all atom coordinates are within the [0,1] interval.
		// If yes, we assume reduced coordinate format.

		if(Box3(Point3(-0.01f), Point3(1.01f)).containsBox(boundingBox)) {
			// Convert all atom coordinates from reduced to absolute (Cartesian) format.
			const AffineTransformation simCell = simulationCell().matrix();
			for(Point3& p : posProperty->point3Range())
				p = simCell * p;
		}
	}

	setStatus(tr("%1 particles at timestep %2").arg(header.natoms).arg(header.ntimestep));
}

/******************************************************************************
 * Saves the class' contents to the given stream.
 *****************************************************************************/
void LAMMPSBinaryDumpImporter::saveToStream(ObjectSaveStream& stream)
{
	ParticleImporter::saveToStream(stream);

	stream.beginChunk(0x01);
	_columnMapping.saveToStream(stream);
	stream.endChunk();
}

/******************************************************************************
 * Loads the class' contents from the given stream.
 *****************************************************************************/
void LAMMPSBinaryDumpImporter::loadFromStream(ObjectLoadStream& stream)
{
	ParticleImporter::loadFromStream(stream);

	stream.expectChunk(0x01);
	_columnMapping.loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
 * Creates a copy of this object.
 *****************************************************************************/
OORef<RefTarget> LAMMPSBinaryDumpImporter::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<LAMMPSBinaryDumpImporter> clone = static_object_cast<LAMMPSBinaryDumpImporter>(ParticleImporter::clone(deepCopy, cloneHelper));
	clone->_columnMapping = this->_columnMapping;
	return clone;
}

/******************************************************************************
 * Displays a dialog box that allows the user to edit the custom file column to particle
 * property mapping.
 *****************************************************************************/
void LAMMPSBinaryDumpImporter::showEditColumnMappingDialog(QWidget* parent)
{
	InputColumnMappingDialog dialog(_columnMapping, parent);
	if(dialog.exec() == QDialog::Accepted) {
		setColumnMapping(dialog.mapping());
		requestReload();
	}
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void LAMMPSBinaryDumpImporterEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("LAMMPS binary dump file"), rolloutParams);

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

	QPushButton* editMappingButton = new QPushButton(tr("Edit column mapping..."));
	sublayout->addWidget(editMappingButton);
	connect(editMappingButton, &QPushButton::clicked, this, &LAMMPSBinaryDumpImporterEditor::onEditColumnMapping);
}

/******************************************************************************
* Is called when the user pressed the "Edit column mapping" button.
******************************************************************************/
void LAMMPSBinaryDumpImporterEditor::onEditColumnMapping()
{
	if(LAMMPSBinaryDumpImporter* importer = static_object_cast<LAMMPSBinaryDumpImporter>(editObject()))
		importer->showEditColumnMappingDialog(mainWindow());
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
