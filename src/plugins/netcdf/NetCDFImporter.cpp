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
#include <core/utilities/concurrent/Task.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <plugins/particles/importer/InputColumnMappingDialog.h>

#include "NetCDFImporter.h"

#ifdef WIN32
	#define DLL_NETCDF
#endif
#include <netcdf.h>

#define NCERR(x)  _ncerr(x, __FILE__, __LINE__)

namespace NetCDF {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(NetCDF, NetCDFImporter, ParticleImporter)
IMPLEMENT_OVITO_OBJECT(NetCDF, NetCDFImporterEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(NetCDFImporter, NetCDFImporterEditor)
DEFINE_PROPERTY_FIELD(NetCDFImporter, _useCustomColumnMapping, "UseCustomColumnMapping")
SET_PROPERTY_FIELD_LABEL(NetCDFImporter, _useCustomColumnMapping, "Custom file column mapping")

/******************************************************************************
* Check for NetCDF error and throw exception
******************************************************************************/
static void _ncerr(int err, const char *file, int line)
{
	if (err != NC_NOERR)
		throw Exception(NetCDFImporter::tr("NetCDF error in line %1 of source file %2: %3").arg(line).arg(file).arg(QString(nc_strerror(err))));
}

/******************************************************************************
 * Sets the user-defined mapping between data columns in the input file and
 * the internal particle properties.
 *****************************************************************************/
void NetCDFImporter::setCustomColumnMapping(const InputColumnMapping& mapping)
{
	_customColumnMapping = mapping;
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool NetCDFImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	QString filename = QDir::toNativeSeparators(input.fileName());

	// Check if we can open the input file for reading.
	int tmp_ncid;
	int err = nc_open(filename.toLocal8Bit().constData(), NC_NOWRITE, &tmp_ncid);
	if (err == NC_NOERR) {
		nc_close(tmp_ncid);
		return true;
	}

	return false;
}

/******************************************************************************
* Scans the input file for simulation timesteps.
******************************************************************************/
void NetCDFImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<LinkedFileImporter::FrameSourceInformation>& frames, const QUrl& sourceUrl, CompressedTextParserStream& stream)
{
	QString filename = QDir::toNativeSeparators(stream.device().fileName());

	// Open the input and read number of frames.
	int ncid;
	NCERR( nc_open(filename.toLocal8Bit().constData(), NC_NOWRITE, &ncid) );
	int frame_dim;
	NCERR( nc_inq_dimid(ncid, "frame", &frame_dim) );
	size_t nFrames;
	NCERR( nc_inq_dimlen(ncid, frame_dim, &nFrames) );
	NCERR( nc_close(ncid) );

	QFileInfo fileInfo(stream.device().fileName());
	QDateTime lastModified = fileInfo.lastModified();
	for(int i = 0; i < nFrames; i++) {
		FrameSourceInformation frame;
		frame.sourceFile = sourceUrl;
		frame.byteOffset = 0;
		frame.lineNumber = i;
		frame.lastModificationTime = lastModified;
		frame.label = tr("Frame %1").arg(i);
		frames.push_back(frame);
	}
}

/******************************************************************************
* Open file if not already opened
******************************************************************************/
void NetCDFImporter::NetCDFImportTask::openNetCDF(const QString &filename)
{
	closeNetCDF();

	// Open the input file for reading.
	NCERR( nc_open(filename.toLocal8Bit().constData(), NC_NOWRITE, &_ncid) );
	_ncIsOpen = true;

	// Make sure we have the right file conventions
	size_t len;
	NCERR( nc_inq_attlen(_ncid, NC_GLOBAL, "Conventions", &len) );
	char *conventions_str = new char[len+1];
	NCERR( nc_get_att_text(_ncid, NC_GLOBAL, "Conventions", conventions_str) );
	conventions_str[len] = 0;
	if (strcmp(conventions_str, "AMBER"))
		throw Exception(tr("NetCDF file %1 follows '%2' conventions, expected 'AMBER'.").arg(filename, conventions_str));
	delete [] conventions_str;

#if 0
	// Read creator information
	NCERR( nc_inq_attlen(_ncid, NC_GLOBAL, "program", &len) );
	char *program_str = new char[len+1];
	NCERR( nc_get_att_text(_ncid, NC_GLOBAL, "program", program_str) );
	program_str[len] = 0;

	NCERR( nc_inq_attlen(_ncid, NC_GLOBAL, "programVersion", &len) );
	char *program_version_str = new char[len+1];
	NCERR( nc_get_att_text(_ncid, NC_GLOBAL, "programVersion", program_version_str) );
	program_version_str[len] = 0;

	// Log this
	VerboseLogger() << "Opened AMBER-style NetCDF file " << filename << ". File written by " << program_str << ", " << program_version_str << "." << endl;

	delete [] program_str;
	delete [] program_version_str;
#endif

	// Get dimensions
	NCERR( nc_inq_dimid(_ncid, "frame", &_frame_dim) );
	NCERR( nc_inq_dimid(_ncid, "atom", &_atom_dim) );
	NCERR( nc_inq_dimid(_ncid, "spatial", &_spatial_dim) );
	NCERR( nc_inq_dimid(_ncid, "cell_spatial", &_cell_spatial_dim) );
	NCERR( nc_inq_dimid(_ncid, "cell_angular", &_cell_angular_dim) );

	// Get some variables
	if (nc_inq_varid(_ncid, "cell_origin", &_cell_origin_var) != NC_NOERR)
		_cell_origin_var = -1;
	NCERR( nc_inq_varid(_ncid, "cell_lengths", &_cell_lengths_var) );
	NCERR( nc_inq_varid(_ncid, "cell_angles", &_cell_angles_var) );
	if (nc_inq_varid(_ncid, "shear_dx", &_shear_dx_var) != NC_NOERR)
		_shear_dx_var = -1;
}

/******************************************************************************
* Close the current NetCDF file.
******************************************************************************/
void NetCDFImporter::NetCDFImportTask::closeNetCDF()
{
	if (_ncIsOpen) {
		NCERR( nc_close(_ncid) );
		_ncid = -1;
		_ncIsOpen = false;
	}
}

/******************************************************************************
* Parses the given input file and stores the data in this container object.
******************************************************************************/
void NetCDFImporter::NetCDFImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Reading NetCDF file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// First close text stream so we can re-open it in binary mode.
	QFileDevice& file = stream.device();
	file.close();

	// Open file.
	QString filename = file.fileName();

	openNetCDF(filename);

	// Scan NetCDF and iterate supported column names.
	InputColumnMapping columnMapping;

	// Now iterate over all variables and see whether they start with either atom or frame dimensions.
	int nVars;
	NCERR( nc_inq_nvars(_ncid, &nVars) );
	int column = 0;
	for (int varid = 0; varid < nVars; varid++) {
		char name[NC_MAX_NAME+1];
		nc_type type;

		// Retrieve NetCDF meta-information.
		int nDims, dimIds[NC_MAX_VAR_DIMS];
		NCERR( nc_inq_var(_ncid, varid, name, &type, &nDims, dimIds, NULL) );

		// Check if dimensions make sense and we can understand them.
		if (dimIds[0] == _atom_dim || ( nDims > 1 && dimIds[0] == _frame_dim && dimIds[1] == _atom_dim )) {
			// Do we support this data type?
			if (type == NC_BYTE || type == NC_SHORT || type == NC_INT || type == NC_LONG) {
				mapVariableToColumn(columnMapping, column, name, qMetaTypeId<int>());
				column++;
			}
			else if (type == NC_FLOAT || type == NC_DOUBLE) {
				mapVariableToColumn(columnMapping, column, name, qMetaTypeId<FloatType>());
				column++;
			}
			else {
				qDebug() << "Skipping NetCDF variable " << name << " because type is not known." << endl;
			}
		}
	}

	// Check if the only thing we need to do is read column information.
	if (_parseFileHeaderOnly) {
		_customColumnMapping = columnMapping;
		closeNetCDF();
		return;
	}

	// Set up column-to-property mapping.
	if(_useCustomColumnMapping && _customColumnMapping.columnCount() > 0)
		columnMapping = _customColumnMapping;

	// Get frame number.
	size_t movieFrame = frame().lineNumber;

	// Total number of particles.
	size_t particleCount;
	NCERR( nc_inq_dimlen(_ncid, _atom_dim, &particleCount) );

	// Simulation cell. Note that cell_origin is an extension to the AMBER specification.
	double o[3] = { 0.0, 0.0, 0.0 };
	double l[3], a[3];
	double d[3] = { 0.0, 0.0, 0.0 };
	size_t startp[3] = { movieFrame, 0, 0 };
	size_t countp[3] = { 1, 3, 0 };
	if (_cell_origin_var != -1)
		NCERR( nc_get_vara_double(_ncid, _cell_origin_var, startp, countp, o) );
	NCERR( nc_get_vara_double(_ncid, _cell_lengths_var, startp, countp, l) );
	NCERR( nc_get_vara_double(_ncid, _cell_angles_var, startp, countp, a) );
	if (_shear_dx_var != -1)
		NCERR( nc_get_vara_double(_ncid, _shear_dx_var, startp, countp, d) );
	
	// Express cell vectors va, vb and vc in the X,Y,Z-system
	a[0] *= M_PI/180.0;
	a[1] *= M_PI/180.0;
	a[2] *= M_PI/180.0;
	Vector3 va(l[0], 0, 0);
	Vector3 vb(l[1]*cos(a[2]), l[1]*sin(a[2]), 0);
	double cx = cos(a[1]);
	double cy = (cos(a[0]) - cos(a[1])*cos(a[2]))/sin(a[2]);
	double cz = sqrt(1. - cx*cx - cy*cy);
	Vector3 vc(l[2]*cx+d[0], l[2]*cy+d[1], l[2]*cz);

	// Set simulation cell.
	simulationCell().setMatrix(AffineTransformation(va, vb, vc, Vector3(o[0], o[1], o[2])));

	// Report to user.
	futureInterface.setProgressRange(columnMapping.columnCount());

	// Now iterate over all variables and load the appropriate frame
	for (int column = 0; column < columnMapping.columnCount(); column++) {
		if(futureInterface.isCanceled()) {
			closeNetCDF();
			return;
		}
		futureInterface.setProgressValue(column);
		
		ParticleProperty* property = nullptr;

		int dataType = columnMapping.dataType(column);

		if(dataType != QMetaType::Void) {
			size_t dataTypeSize;
			if(dataType == qMetaTypeId<int>())
				dataTypeSize = sizeof(int);
			else if(dataType == qMetaTypeId<FloatType>())
				dataTypeSize = sizeof(FloatType);
			else
				throw Exception(tr("Invalid custom particle property (data type %1) for input file column %2 of NetCDF file.").arg(dataType).arg(column+1));

			QString columnName = columnMapping.columnName(column);
			QString propertyName = columnMapping.propertyName(column);

			// Retrieve NetCDF meta-information.
			nc_type type;
			int varId, nDims, dimIds[NC_MAX_VAR_DIMS];
			NCERR( nc_inq_varid(_ncid, columnName.toLocal8Bit().constData(), &varId) );
			NCERR( nc_inq_var(_ncid, varId, NULL, &type, &nDims, dimIds, NULL) );

			// Construct pointers to NetCDF dimension indices.
			countp[0] = 1;
			countp[1] = 1;
			countp[2] = 1;

			int nDimsDetected = -1, componentCount = 1;
			if (nDims > 0) {
				// This is a per frame property
				startp[0] = movieFrame;
				countp[0] = 1;

				if (nDims > 1 && dimIds[1] == _atom_dim) {
					// This is a per atom property
					startp[1] = 0;
					countp[1] = particleCount;
					nDimsDetected = 2;

					if (nDims > 2 && dimIds[2] == _spatial_dim) {
						// This is a vector property
						startp[2] = 0;
						countp[2] = 3;
						componentCount = 3;
						nDimsDetected = 3;
					}
				}
				else if (nDims > 0 && dimIds[0] == _atom_dim) {
					// This is a per atom property, but global (per-file, not per frame)
					startp[0] = 0;
					countp[0] = particleCount;
					nDimsDetected = 1;
					
					if (nDims > 1 && dimIds[1] == _spatial_dim) {
						// This is a vector property
						startp[1] = 0;
						countp[1] = 3;
						componentCount = 3;
						nDimsDetected = 2;
					}
				}

				// Skip all fields that don't have the expected format.
				if (nDimsDetected != -1 && nDimsDetected == nDims) {
					// Find property to load this information into.
					ParticleProperty::Type propertyType = columnMapping.propertyType(column);

					if(propertyType != ParticleProperty::UserProperty) {
						// Look for existing standard property.
						for(const auto& p : particleProperties()) {
							if(p->type() == propertyType) {
								property = p.get();
								break;
							}
						}
						if(!property) {
							// Create standard property.
							property = new ParticleProperty(particleCount, propertyType);
							addParticleProperty(property);
						}
					}
					else {
						// Look for existing user-defined property with the same name.
						for(int j = 0; j < particleProperties().size(); j++) {
							const auto& p = particleProperties()[j];
							if(p->name() == propertyName) {
								if(property->dataType() == dataType)
									property = p.get();
								else
									removeParticleProperty(j);
								break;
							}
						}
						if(!property) {
							// Create a new user-defined property for the column.
							property = new ParticleProperty(particleCount, dataType, dataTypeSize, componentCount, propertyName);
							addParticleProperty(property);
						}
					}

					OVITO_ASSERT(property != NULL);
					property->setName(propertyName);

					if (property->componentCount() != componentCount) {
						qDebug() << "Warning: Skipping field '" << columnName << "' of NetCDF file because internal and NetCDF component counts do not match." << endl;
					}
					else {
						// Type mangling.
						if (property->dataType() == qMetaTypeId<int>()) {
							// This is an integer data.

							NCERR( nc_get_vara_int(_ncid, varId, startp, countp, property->dataInt()) );

							// Create particles types if this is the particle type property.
							if (propertyType == ParticleProperty::ParticleTypeProperty) {
								// Find maximum atom type.
								int maxType = 0;
								for (int i = 0; i < particleCount; i++)
									maxType = std::max(property->getInt(i), maxType);

								// Count number of atoms for each type.
								QVector<int> typeCount(maxType+1, 0);
								for (int i = 0; i < particleCount; i++)
									typeCount[property->getInt(i)]++;
								
								for (int i = 0; i <= maxType; i++) {
									// Only define atom type if really present.
									if (typeCount[i] > 0)
										addParticleType(i);
								}							
							}
						}
						else if (property->dataType() == qMetaTypeId<FloatType>()) {
							NCERR( nc_get_vara_float(_ncid, varId, startp, countp, property->dataFloat()) );
						}
						else {
							qDebug() << "Warning: Skipping field '" << columnName << "' of NetCDF file because it has an unrecognized data type." << endl;
						}
					}
				}
			}
		}
	}
}

/******************************************************************************
 * Guesses the mapping of input file columns to internal particle properties.
 *****************************************************************************/
void NetCDFImporter::mapVariableToColumn(InputColumnMapping &columnMapping, int column, QString name, int dataType)
{
	QString loweredName = name.toLower();
	if(loweredName == "coordinates") columnMapping.mapStandardColumn(column, ParticleProperty::PositionProperty, 0, name);
	else if(loweredName == "velocities") columnMapping.mapStandardColumn(column, ParticleProperty::VelocityProperty, 0, name);
	else if(loweredName == "id") columnMapping.mapStandardColumn(column, ParticleProperty::IdentifierProperty, 0, name);
	else if(loweredName == "type" || loweredName == "element" || loweredName == "atom_types") columnMapping.mapStandardColumn(column, ParticleProperty::ParticleTypeProperty, 0, name);
	else if(loweredName == "mass") columnMapping.mapStandardColumn(column, ParticleProperty::MassProperty, 0, name);
	else if(loweredName == "radius") columnMapping.mapStandardColumn(column, ParticleProperty::RadiusProperty, 0, name);
	else if(loweredName == "c_cna" || loweredName == "pattern") columnMapping.mapStandardColumn(column, ParticleProperty::StructureTypeProperty, 0, name);
	else if(loweredName == "c_epot") columnMapping.mapStandardColumn(column, ParticleProperty::PotentialEnergyProperty, 0, name);
	else if(loweredName == "c_kpot") columnMapping.mapStandardColumn(column, ParticleProperty::KineticEnergyProperty, 0, name);
	else if(loweredName == "c_stress[1]") columnMapping.mapStandardColumn(column, ParticleProperty::StressTensorProperty, 0, name);
	else if(loweredName == "c_stress[2]") columnMapping.mapStandardColumn(column, ParticleProperty::StressTensorProperty, 1, name);
	else if(loweredName == "c_stress[3]") columnMapping.mapStandardColumn(column, ParticleProperty::StressTensorProperty, 2, name);
	else if(loweredName == "c_stress[4]") columnMapping.mapStandardColumn(column, ParticleProperty::StressTensorProperty, 3, name);
	else if(loweredName == "c_stress[5]") columnMapping.mapStandardColumn(column, ParticleProperty::StressTensorProperty, 4, name);
	else if(loweredName == "c_stress[6]") columnMapping.mapStandardColumn(column, ParticleProperty::StressTensorProperty, 5, name);
	else if(loweredName == "selection") columnMapping.mapStandardColumn(column, ParticleProperty::SelectionProperty, 0, name);
	else if(loweredName == "forces") columnMapping.mapStandardColumn(column, ParticleProperty::ForceProperty, 0, name);
	else {
		columnMapping.mapCustomColumn(column, name, dataType, 0, ParticleProperty::UserProperty, name);
	}
}

/******************************************************************************
 * Saves the class' contents to the given stream.
 *****************************************************************************/
void NetCDFImporter::saveToStream(ObjectSaveStream& stream)
{
	ParticleImporter::saveToStream(stream);

	stream.beginChunk(0x01);
	_customColumnMapping.saveToStream(stream);
	stream.endChunk();
}

/******************************************************************************
 * Loads the class' contents from the given stream.
 *****************************************************************************/
void NetCDFImporter::loadFromStream(ObjectLoadStream& stream)
{
	ParticleImporter::loadFromStream(stream);

	stream.expectChunk(0x01);
	_customColumnMapping.loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
 * Creates a copy of this object.
 *****************************************************************************/
OORef<RefTarget> NetCDFImporter::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<NetCDFImporter> clone = static_object_cast<NetCDFImporter>(ParticleImporter::clone(deepCopy, cloneHelper));
	clone->_customColumnMapping = this->_customColumnMapping;
	return clone;
}

/******************************************************************************
 * Displays a dialog box that allows the user to edit the custom file column to particle
 * property mapping.
 *****************************************************************************/
void NetCDFImporter::showEditColumnMappingDialog(QWidget* parent)
{
	// Retrieve column names from current input file.
	LinkedFileObject* obj = nullptr;
	for(RefMaker* refmaker : dependents()) {
		obj = dynamic_object_cast<LinkedFileObject>(refmaker);
		if(obj) break;
	}
	if(!obj) return;

	// Start task that inspects the file header to determine the number of data columns.
	std::unique_ptr<NetCDFImportTask> inspectionTask(new NetCDFImportTask(obj->frames().front()));
	DataSetContainer& datasetContainer = *dataset()->container();	
	Future<void> future = datasetContainer.taskManager().runInBackground<void>(std::bind(&NetCDFImportTask::load,
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
void NetCDFImporterEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("NetCDF file"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGroupBox* columnMappingBox = new QGroupBox(tr("File columns"), rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(columnMappingBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(columnMappingBox);

	BooleanRadioButtonParameterUI* useCustomMappingUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(NetCDFImporter::_useCustomColumnMapping));
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
void NetCDFImporterEditor::onEditColumnMapping()
{
	if(NetCDFImporter* importer = static_object_cast<NetCDFImporter>(editObject()))
		importer->showEditColumnMappingDialog(mainWindow());
}

};
