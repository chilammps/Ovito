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
//  Contributions:
//
//  Support for extended XYZ format has been added by James Kermode,
//  Department of Physics, King's College London.
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
#include "XYZImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, XYZImporter, ParticleImporter);
SET_OVITO_OBJECT_EDITOR(XYZImporter, XYZImporterEditor);
OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, XYZImporterEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
 * Sets the user-defined mapping between data columns in the input file and
 * the internal particle properties.
 *****************************************************************************/
void XYZImporter::setColumnMapping(const InputColumnMapping& mapping)
{
	_columnMapping = mapping;
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool XYZImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

	// Read first line.
	stream.readLine(20);
	if(stream.line()[0] == '\0')
		return false;

	// Skip initial whitespace.
	const char* p = stream.line();
	while(isspace(*p)) {
		if(*p == '\0') return false;
		++p;
	}
	if(!isdigit(*p)) return false;
	// Skip digits.
	while(isdigit(*p)) {
		if(*p == '\0') break;
		++p;
	}
	// Check trailing whitespace.
	bool foundNewline = false;
	while(*p != '\0') {
		if(!isspace(*p)) return false;
		if(*p == '\n' || *p == '\r')
			foundNewline = true;
		++p;
	}

	return foundNewline;
}

/******************************************************************************
* This method is called by the FileSource each time a new source
* file has been selected by the user.
******************************************************************************/
bool XYZImporter::inspectNewFile(FileSource* obj)
{
	if(!ParticleImporter::inspectNewFile(obj))
		return false;

	if(obj->frames().empty())
		return false;

	// Start task that inspects the file header to determine the number of columns.
	std::shared_ptr<XYZImportTask> inspectionTask = std::make_shared<XYZImportTask>(dataset()->container(), obj->frames().front());
	if(!dataset()->container()->taskManager().runTask(inspectionTask))
		return false;

	// If column names were given in the XYZ file, use them rather than popping up a dialog.
	if(inspectionTask->propertiesAssigned()) {
		setColumnMapping(inspectionTask->columnMapping());
		return true;
	}

	// Don't show column mapping dialog in console mode.
	if(Application::instance().consoleMode())
		return true;

	InputColumnMapping mapping(_columnMapping);
	mapping.resize(inspectionTask->columnMapping().size());
	mapping.setFileExcerpt(inspectionTask->columnMapping().fileExcerpt());
	if(_columnMapping.size() != mapping.size()) {
		if(_columnMapping.empty()) {
			// Load last mapping from settings store.
			QSettings settings;
			settings.beginGroup("viz/importer/xyz/");
			if(settings.contains("columnmapping")) {
				try {
					mapping.fromByteArray(settings.value("columnmapping").toByteArray());

				}
				catch(Exception& ex) {
					ex.prependGeneralMessage(tr("Failed to load last used column-to-property mapping from application settings store."));
					ex.logError();
				}
			}

			mapping.resize(inspectionTask->columnMapping().size());
			for(auto& column : mapping)
				column.columnName.clear();
		}

		InputColumnMappingDialog dialog(mapping, dataset()->mainWindow());
		if(dialog.exec() == QDialog::Accepted) {
			setColumnMapping(dialog.mapping());
			// Remember the user-defined mapping for the next time.
			QSettings settings;
			settings.beginGroup("viz/importer/xyz/");
			settings.setValue("columnmapping", _columnMapping.toByteArray());
			settings.endGroup();
			return true;
		}
		return false;
	}
	else _columnMapping.setFileExcerpt(inspectionTask->columnMapping().fileExcerpt());

	return true;
}

/******************************************************************************
* Scans the given input file to find all contained simulation frames.
******************************************************************************/
void XYZImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<FileSourceImporter::Frame>& frames, const QUrl& sourceUrl, CompressedTextReader& stream)
{
	futureInterface.setProgressText(tr("Scanning XYZ file %1").arg(stream.filename()));
	futureInterface.setProgressRange(stream.underlyingSize() / 1000);

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	int numParticles = 0;
	QFileInfo fileInfo(stream.device().fileName());
	QString filename = fileInfo.fileName();
	QDateTime lastModified = fileInfo.lastModified();
	int frameNumber = 0;

	while(!stream.eof()) {
		qint64 byteOffset = stream.byteOffset();

		// Parse number of atoms.
		stream.readLine();
		int startLineNumber = stream.lineNumber();

		if(stream.line()[0] == '\0') break;
		if(sscanf(stream.line(), "%u", &numParticles) != 1 || numParticles < 0 || numParticles > 1e9)
			throw Exception(tr("Invalid number of particles in line %1 of XYZ file: %2").arg(stream.lineNumber()).arg(stream.lineString()));

		// Create a new record for the time step.
		Frame frame;
		frame.sourceFile = sourceUrl;
		frame.byteOffset = byteOffset;
		frame.lineNumber = startLineNumber;
		frame.lastModificationTime = lastModified;
		frame.label = QString("%1 (Frame %2)").arg(filename).arg(frameNumber++);
		frames.push_back(frame);

		// Skip comment line.
		stream.readLine();

		// Skip atom lines.
		for(int i = 0; i < numParticles; i++) {
			stream.readLine();
			if((i % 4096) == 0) {
				futureInterface.setProgressValue(stream.underlyingByteOffset() / 1000);
				if(futureInterface.isCanceled())
					return;
			}
		}
	}
}

/******************************************************************************
 * Guesses the mapping of input file columns to internal particle properties.
 * Naming conventions followed are those used by QUIP code <http://www.libatoms.org>
 *****************************************************************************/
bool XYZImporter::mapVariableToProperty(InputColumnMapping& columnMapping, int column, QString name, int dataType, int vec)
{
	columnMapping[column].columnName = name;
	QString loweredName = name.toLower();
	if(loweredName == "type" || loweredName == "element" || loweredName == "atom_types" ||loweredName == "species") 
		columnMapping[column].mapStandardColumn(ParticleProperty::ParticleTypeProperty);
	else if(loweredName == "pos") columnMapping[column].mapStandardColumn(ParticleProperty::PositionProperty, vec);
	else if(loweredName == "selection") columnMapping[column].mapStandardColumn(ParticleProperty::SelectionProperty, vec);
	else if(loweredName == "color") columnMapping[column].mapStandardColumn(ParticleProperty::ColorProperty, vec);
	else if(loweredName == "disp") columnMapping[column].mapStandardColumn(ParticleProperty::DisplacementProperty, vec);
	else if(loweredName == "disp_mag") columnMapping[column].mapStandardColumn(ParticleProperty::DisplacementMagnitudeProperty);
	else if(loweredName == "local_energy") columnMapping[column].mapStandardColumn(ParticleProperty::PotentialEnergyProperty);
	else if(loweredName == "kinetic_energy") columnMapping[column].mapStandardColumn(ParticleProperty::KineticEnergyProperty);
	else if(loweredName == "total_energy") columnMapping[column].mapStandardColumn(ParticleProperty::TotalEnergyProperty);
	else if(loweredName == "velo") columnMapping[column].mapStandardColumn(ParticleProperty::VelocityProperty, vec);
	else if(loweredName == "velo_mag") columnMapping[column].mapStandardColumn(ParticleProperty::VelocityMagnitudeProperty);
	else if(loweredName == "radius") columnMapping[column].mapStandardColumn(ParticleProperty::RadiusProperty);
	else if(loweredName == "cluster") columnMapping[column].mapStandardColumn(ParticleProperty::ClusterProperty);
	else if(loweredName == "n_neighb") columnMapping[column].mapStandardColumn(ParticleProperty::CoordinationProperty);
 	else if(loweredName == "structure_type") columnMapping[column].mapStandardColumn(ParticleProperty::StructureTypeProperty);
	else if(loweredName == "id") columnMapping[column].mapStandardColumn(ParticleProperty::IdentifierProperty);
	else if(loweredName == "stress") columnMapping[column].mapStandardColumn(ParticleProperty::StressTensorProperty, vec);
	else if(loweredName == "strain") columnMapping[column].mapStandardColumn(ParticleProperty::StrainTensorProperty, vec);
	else if(loweredName == "deform") columnMapping[column].mapStandardColumn(ParticleProperty::DeformationGradientProperty, vec);
	else if(loweredName == "orientation") columnMapping[column].mapStandardColumn(ParticleProperty::OrientationProperty, vec);
	else if(loweredName == "force" || loweredName == "forces") columnMapping[column].mapStandardColumn(ParticleProperty::ForceProperty, vec);
	else if(loweredName == "mass") columnMapping[column].mapStandardColumn(ParticleProperty::MassProperty);
	else if(loweredName == "charge") columnMapping[column].mapStandardColumn(ParticleProperty::ChargeProperty);
	else if(loweredName == "map_shift") columnMapping[column].mapStandardColumn(ParticleProperty::PeriodicImageProperty, vec);
	else if(loweredName == "transparency") columnMapping[column].mapStandardColumn(ParticleProperty::TransparencyProperty);
	else if(loweredName == "dipoles") columnMapping[column].mapStandardColumn(ParticleProperty::DipoleOrientationProperty, vec);
	else if(loweredName == "dipoles_mag") columnMapping[column].mapStandardColumn(ParticleProperty::DipoleMagnitudeProperty);
	else if(loweredName == "omega") columnMapping[column].mapStandardColumn(ParticleProperty::AngularVelocityProperty, vec);
	else if(loweredName == "angular_momentum") columnMapping[column].mapStandardColumn(ParticleProperty::AngularMomentumProperty, vec);
	else if(loweredName == "torque") columnMapping[column].mapStandardColumn(ParticleProperty::TorqueProperty, vec);
	else if(loweredName == "spin") columnMapping[column].mapStandardColumn(ParticleProperty::SpinProperty, vec);
	else if(loweredName == "centro_symmetry") columnMapping[column].mapStandardColumn(ParticleProperty::CentroSymmetryProperty);
	else {
		// Only int or float custom properties are supported
		if(dataType == qMetaTypeId<FloatType>() || dataType == qMetaTypeId<int>())
			columnMapping[column].mapCustomColumn(name, dataType, vec);
		else
			return false;
	}
	return true;
}

/******************************************************************************
 * Helper function that converts a string repr. of a bool ('T' or 'F') to an int
 *****************************************************************************/
inline bool parseBool(const char* s, int& d)
{
	if(s[1] != '\0') return false;
	if(s[0] == 'T') {
		d = 1;
		return true;
	}
	else if(s[0] == 'F') {
		d = 0;
		return true;
	}
	return false;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void XYZImporter::XYZImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(
			tr("Reading XYZ file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Parse number of atoms.
	int numParticles;
	if(sscanf(stream.readLine(), "%u", &numParticles) != 1 || numParticles < 0 || numParticles > 1e9)
		throw Exception(tr("Invalid number of particles in line %1 of XYZ file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
	setProgressRange(numParticles);
	QString fileExcerpt = stream.lineString();

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Extract some useful information from the comment line.
	stream.readLine();
	bool hasSimulationCell = false;
	int movieMode = -1;

	simulationCell().setPbcFlags(false, false, false);
	Vector3 cellOrigin = Vector3::Zero();
	Vector3 cellVector1 = Vector3::Zero();
	Vector3 cellVector2 = Vector3::Zero();
	Vector3 cellVector3 = Vector3::Zero();
	QString remainder;
	int index;

	// Try to parse the simulation cell geometry from the comment line.
	QString commentLine = stream.lineString();
	if((index = commentLine.indexOf("Lxyz=")) >= 0)
		remainder = commentLine.mid(index + 5).trimmed();
	else if((index = commentLine.indexOf("boxsize")) >= 0)
		remainder = commentLine.mid(index + 7).trimmed();
	if(!remainder.isEmpty()) {
		QStringList list = remainder.split(ws_re);
		if(list.size() >= 3) {
			bool ok1, ok2, ok3;
			FloatType sx = (FloatType)list[0].toDouble(&ok1);
			FloatType sy = (FloatType)list[1].toDouble(&ok2);
			FloatType sz = (FloatType)list[2].toDouble(&ok3);
			if(ok1 && ok2 && ok3) {
				simulationCell().setMatrix(AffineTransformation(Vector3(sx, 0, 0), Vector3(0, sy, 0), Vector3(0, 0, sz), Vector3(-sx / 2, -sy / 2, -sz / 2)));
				hasSimulationCell = true;
			}
		}
	}
	if((index = commentLine.toLower().indexOf("lattice=\"")) >= 0) {
		// Extended XYZ format: Lattice="R11 R21 R31 R12 R22 R32 R13 R23 R33"
		// See http://jrkermode.co.uk/quippy/io.html#extendedxyz for details

		QString latticeStr = commentLine.mid(index + 9);
		latticeStr.truncate(latticeStr.indexOf("\""));
		QStringList list = latticeStr.split(ws_re, QString::SkipEmptyParts);
		if(list.size() >= 9) {
			for(int k = 0; k < 3; k++)
				cellVector1[k] = (FloatType)list[k].toDouble();
			for(int k = 3; k < 6; k++)
				cellVector2[k - 3] = (FloatType)list[k].toDouble();
			for(int k = 6; k < 9; k++)
				cellVector3[k - 6] = (FloatType)list[k].toDouble();
		}

		if ((index = commentLine.toLower().indexOf("cell_origin=\"")) >= 0) {
			QString cellOriginStr = commentLine.mid(index + 13);
			cellOriginStr.truncate(cellOriginStr.indexOf("\""));
			QStringList list = cellOriginStr.split(ws_re, QString::SkipEmptyParts);
			for(int k = 0; k < list.size() && k < 3; k++)
				cellOrigin[k] = (FloatType)list[k].toDouble();
		}
	}
	else {
		// XYZ file written by Parcas MD code contain simulation cell info in comment line.

		if((index = commentLine.indexOf("cell_orig ")) >= 0) {
			QStringList list = commentLine.mid(index + 10).split(ws_re, QString::SkipEmptyParts);
			for(int k = 0; k < list.size() && k < 3; k++)
				cellOrigin[k] = (FloatType)list[k].toDouble();
		}
		if((index = commentLine.indexOf("cell_vec1 ")) >= 0) {
			QStringList list = commentLine.mid(index + 10).split(ws_re, QString::SkipEmptyParts);
			for(int k = 0; k < list.size() && k < 3; k++)
				cellVector1[k] = (FloatType)list[k].toDouble();
		}
		if((index = commentLine.indexOf("cell_vec2 ")) >= 0) {
			QStringList list = commentLine.mid(index + 10).split(ws_re, QString::SkipEmptyParts);
			for(int k = 0; k < list.size() && k < 3; k++)
				cellVector2[k] = (FloatType)list[k].toDouble();
		}
		if((index = commentLine.indexOf("cell_vec3 ")) >= 0) {
			QStringList list = commentLine.mid(index + 10).split(ws_re, QString::SkipEmptyParts);
			for(int k = 0; k < list.size() && k < 3; k++)
				cellVector3[k] = (FloatType)list[k].toDouble();
		}
	}

	if(cellVector1 != Vector3::Zero() && cellVector2 != Vector3::Zero() && cellVector3 != Vector3::Zero()) {
		simulationCell().setMatrix(AffineTransformation(cellVector1, cellVector2, cellVector3, cellOrigin));
		hasSimulationCell = true;
	}

	if((index = commentLine.indexOf("pbc ")) >= 0) {
		QStringList list = commentLine.mid(index + 4).split(ws_re);
		simulationCell().setPbcFlags((bool)list[0].toInt(), (bool)list[1].toInt(), (bool)list[2].toInt());
	} else if ((index = commentLine.indexOf("pbc=\"")) >= 0) {
		// Look for Extended XYZ PBC keyword
		QString pbcStr = commentLine.mid(index + 5);
		pbcStr.truncate(pbcStr.indexOf("\""));
		QStringList list = pbcStr.split(ws_re);
		int pbcFlags[3] = {0, 0, 0};
		for (int i=0; i < list.size() && i < 3; i++) {
			QByteArray ba = list[i].toLatin1();
			parseBool(ba.data(), pbcFlags[i]);
		}
		simulationCell().setPbcFlags(pbcFlags[0], pbcFlags[1], pbcFlags[2]);
	} else if (hasSimulationCell)
		simulationCell().setPbcFlags(true, true, true);

	if(_parseFileHeaderOnly) {
		// Read first atoms line and count number of data columns.
		fileExcerpt += stream.lineString();
		QString lineString;
		for(int i = 0; i < 5 && i < numParticles; i++) {
			stream.readLine();
			lineString = stream.lineString();
			fileExcerpt += lineString;
		}
		if(numParticles > 5) fileExcerpt += QStringLiteral("...\n");
		_columnMapping.resize(lineString.split(ws_re, QString::SkipEmptyParts).size());
		_columnMapping.setFileExcerpt(fileExcerpt);

		// check for Extended XYZ Properties key and use instead of popping up dialog box
		// format is described at http://jrkermode.co.uk/quippy/io.html#extendedxyz
		// example: Properties=species:S:1:pos:R:3 for atomic species (1 column, string property)
		// and atomic positions (3 columns, real property)
		if((index = commentLine.toLower().indexOf("properties=")) >= 0) {
			QString propertiesStr = commentLine.mid(index + 11);
			propertiesStr = propertiesStr.left(propertiesStr.indexOf(ws_re));
			QStringList fields = propertiesStr.split(":");

			int col = 0;
			for(int i = 0; i < fields.size() / 3; i += 1) {
				QString propName = (fields[3 * i + 0]);
				QString propTypeStr = (fields[3 * i + 1]).left(1);
				QByteArray propTypeBA = propTypeStr.toLatin1();
				char propType = propTypeBA.data()[0];
				int nCols = (int)fields[3 * i + 2].toInt();
				switch(propType) {
				case 'I':
					for(int k = 0; k < nCols; k++) {
						mapVariableToProperty(_columnMapping, col, propName, qMetaTypeId<int>(), k);
						col++;
					}
					break;
				case 'R':
					for(int k = 0; k < nCols; k++) {
						mapVariableToProperty(_columnMapping, col, propName, qMetaTypeId<FloatType>(), k);
						col++;
					}
					break;
				case 'L':
					for(int k = 0; k < nCols; k++) {
						mapVariableToProperty(_columnMapping, col, propName, qMetaTypeId<int>(), k);
						col++;
					}
					break;
				case 'S':
					for(int k = 0; k < nCols; k++) {
						if(!mapVariableToProperty(_columnMapping, col, propName, qMetaTypeId<char>(), k) && k == 0)
							qDebug() << "Warning: Skipping field" << propName << "of XYZ file because it has an unsupported data type (string).";
						col++;
					}
					break;
				}
			}
			_propertiesAssigned = true;
		}

		return;
	}

	// Parse data columns.
	InputColumnReader columnParser(_columnMapping, *this, numParticles);
	try {
		for(int i = 0; i < numParticles; i++) {
			if(!reportProgress(i)) return;
			stream.readLine();
			columnParser.readParticle(i, stream.line());
		}
	}
	catch(Exception& ex) {
		throw ex.prependGeneralMessage(tr("Parsing error in line %1 of XYZ file.").arg(stream.lineNumber()));
	}

	// Since we created particle types on the go while reading the particle, the assigned particle type IDs
	// depends on the storage order of particles in the file. We rather want a well-defined particle type ordering, that's
	// why we sort them now according to their names.
	if(columnParser.usingNamedParticleTypes())
		sortParticleTypesByName();
	else
		sortParticleTypesById();

	ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
	if(posProperty && numParticles > 0) {
		Box3 boundingBox;
		boundingBox.addPoints(posProperty->constDataPoint3(), posProperty->size());

		if(!hasSimulationCell) {
			// If the input file does not contain simulation cell info,
			// Use bounding box of particles as simulation cell.
			simulationCell().setMatrix(AffineTransformation(
					Vector3(boundingBox.sizeX(), 0, 0),
					Vector3(0, boundingBox.sizeY(), 0),
					Vector3(0, 0, boundingBox.sizeZ()),
					boundingBox.minc - Point3::Origin()));
		}
		else {
			// Determine if coordinates are given in reduced format and need to be rescaled to absolute format.
			// Assume reduced format if all coordinates are within the [0,1] or [-0.5,+0.5] range (plus some small epsilon).
			if(Box3(Point3(-0.01f), Point3(1.01f)).containsBox(boundingBox)) {
				// Convert all atom coordinates from reduced to absolute (Cartesian) format.
				const AffineTransformation simCell = simulationCell().matrix();
				for(Point3& p : posProperty->point3Range())
					p = simCell * p;
			}
			else if(Box3(Point3(-0.51f), Point3(0.51f)).containsBox(boundingBox)) {
				// Convert all atom coordinates from reduced to absolute (Cartesian) format.
				const AffineTransformation simCell = simulationCell().matrix();
				for(Point3& p : posProperty->point3Range())
					p = simCell * (p + Vector3(FloatType(0.5)));
			}
		}
	}

	if(commentLine.isEmpty())
		setStatus(tr("%1 particles").arg(numParticles));
	else
		setStatus(tr("%1 particles\n%2").arg(numParticles).arg(commentLine));
}

/******************************************************************************
 * Saves the class' contents to the given stream.
 *****************************************************************************/
void XYZImporter::saveToStream(ObjectSaveStream& stream)
{
	ParticleImporter::saveToStream(stream);

	stream.beginChunk(0x01);
	_columnMapping.saveToStream(stream);
	stream.endChunk();
}

/******************************************************************************
 * Loads the class' contents from the given stream.
 *****************************************************************************/
void XYZImporter::loadFromStream(ObjectLoadStream& stream)
{
	ParticleImporter::loadFromStream(stream);

	stream.expectChunk(0x01);
	_columnMapping.loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
 * Creates a copy of this object.
 *****************************************************************************/
OORef<RefTarget> XYZImporter::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<XYZImporter> clone = static_object_cast<XYZImporter>(ParticleImporter::clone(deepCopy, cloneHelper));
	clone->_columnMapping = this->_columnMapping;
	return clone;
}

/******************************************************************************
 * Displays a dialog box that allows the user to edit the custom file column to particle
 * property mapping.
 *****************************************************************************/
void XYZImporter::showEditColumnMappingDialog(QWidget* parent)
{
	InputColumnMappingDialog dialog(_columnMapping, parent);
	if(dialog.exec() == QDialog::Accepted) {
		setColumnMapping(dialog.mapping());
		// Remember the user-defined mapping for the next time.
		QSettings settings;
		settings.beginGroup("viz/importer/xyz/");
		settings.setValue("columnmapping", _columnMapping.toByteArray());
		settings.endGroup();
		requestReload();
	}
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void XYZImporterEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("XYZ file"), rolloutParams);

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
	connect(editMappingButton, &QPushButton::clicked, this, &XYZImporterEditor::onEditColumnMapping);
}

/******************************************************************************
* Is called when the user pressed the "Edit column mapping" button.
******************************************************************************/
void XYZImporterEditor::onEditColumnMapping()
{
	if(XYZImporter* importer = static_object_cast<XYZImporter>(editObject()))
		importer->showEditColumnMappingDialog(mainWindow());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
