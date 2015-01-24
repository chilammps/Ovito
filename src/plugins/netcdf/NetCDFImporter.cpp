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

///////////////////////////////////////////////////////////////////////////////
//
//  This module implements import of AMBER-style NetCDF trajectory files.
//  For specification documents see <http://ambermd.org/netcdf/>.
//
//  Extensions to this specification are supported through OVITO's manual
//  column mappings.
//
//  A LAMMPS dump style for this file format can be found at
//  <https://github.com/pastewka/lammps-netcdf>.
//
//  An ASE trajectory container is found in ase.io.netcdftrajectory.
//  <https://wiki.fysik.dtu.dk/ase/epydoc/ase.io.netcdftrajectory-module.html>.
//
//  Please contact Lars Pastewka <lars.pastewka@iwm.fraunhofer.de> for
//  questions and suggestions.
//
///////////////////////////////////////////////////////////////////////////////

#include <core/Core.h>
#include <core/utilities/io/FileManager.h>
#include <core/utilities/concurrent/Future.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/FileSource.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <plugins/particles/import/InputColumnMappingDialog.h>

#include "NetCDFImporter.h"

#ifdef WIN32
	#define DLL_NETCDF
#endif
#include <netcdf.h>

#define NCERR(x)  _ncerr(x, __FILE__, __LINE__)
#define NCERRI(x, info)  _ncerr_with_info(x, __FILE__, __LINE__, info)

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(NetCDFPlugin, NetCDFImporter, ParticleImporter);
SET_OVITO_OBJECT_EDITOR(NetCDFImporter, NetCDFImporterEditor);
DEFINE_PROPERTY_FIELD(NetCDFImporter, _useCustomColumnMapping, "UseCustomColumnMapping");
SET_PROPERTY_FIELD_LABEL(NetCDFImporter, _useCustomColumnMapping, "Custom file column mapping");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(NetCDFPlugin, NetCDFImporterEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

// Convert full tensor to Voigt tensor
template<typename T> void fullToVoigt(size_t particleCount, T *full, T *voigt) {
	for (size_t i = 0; i < particleCount; i++) {
		voigt[6*i] = full[9*i];
		voigt[6*i+1] = full[9*i+4];
		voigt[6*i+2] = full[9*i+8];
		voigt[6*i+3] = 0.5*(full[9*i+5]+full[9*i+7]);
		voigt[6*i+4] = 0.5*(full[9*i+2]+full[9*i+6]);
		voigt[6*i+5] = 0.5*(full[9*i+1]+full[9*i+3]);
    }
}

/******************************************************************************
* Check for NetCDF error and throw exception
******************************************************************************/
static void _ncerr(int err, const char *file, int line)
{
	if (err != NC_NOERR)
		throw Exception(NetCDFImporter::tr("NetCDF error in line %1 of source file %2: %3").arg(line).arg(file).arg(QString(nc_strerror(err))));
}

/******************************************************************************
* Check for NetCDF error and throw exception (and attach additional information
* to exception string)
******************************************************************************/
static void _ncerr_with_info(int err, const char *file, int line, const QString &info)
{
	if (err != NC_NOERR)
		throw Exception(NetCDFImporter::tr("NetCDF error in line %1 of source file %2: %3 %4").arg(line).arg(file).arg(QString(nc_strerror(err))).arg(info));
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
void NetCDFImporter::scanFileForTimesteps(FutureInterfaceBase& futureInterface, QVector<FileSourceImporter::Frame>& frames, const QUrl& sourceUrl, CompressedTextReader& stream)
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
	for(int i = 0; i < (int)nFrames; i++) {
		Frame frame;
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
	std::unique_ptr<char[]> conventions_str(new char[len+1]);
	NCERR( nc_get_att_text(_ncid, NC_GLOBAL, "Conventions", conventions_str.get()) );
	conventions_str[len] = 0;
	if (strcmp(conventions_str.get(), "AMBER"))
		throw Exception(tr("NetCDF file %1 follows '%2' conventions, expected 'AMBER'.").arg(filename, conventions_str.get()));

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
	if (nc_inq_dimid(_ncid, "Voigt", &_Voigt_dim) != NC_NOERR)
		_Voigt_dim = -1;
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
* Close the current NetCDF file.
******************************************************************************/
void NetCDFImporter::NetCDFImportTask::detectDims(int movieFrame, int particleCount, int nDims, int *dimIds, int &nDimsDetected, int &componentCount, int &nativeComponentCount, size_t *startp, size_t *countp)
{
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
            nativeComponentCount = 3;
            nDimsDetected = 3;

            if (nDims > 3 && dimIds[2] == _spatial_dim) {
                // This is a tensor property
                startp[3] = 0;
                countp[3] = 3;
                componentCount = 6;
                nativeComponentCount = 9;
                nDimsDetected = 4;
            }
        }
        else if (nDims == 3 && dimIds[2] == _Voigt_dim) {
            // This is a tensor property, in Voigt notation
            startp[2] = 0;
            countp[2] = 6;
            componentCount = 6;
            nativeComponentCount = 6;
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
            nativeComponentCount = 3;
            nDimsDetected = 2;

            if (nDims > 2 && dimIds[2] == _spatial_dim) {
                // This is a tensor property
                startp[2] = 0;
                countp[2] = 3;
                componentCount = 6;
                nativeComponentCount = 9;
                nDimsDetected = 3;
            }
        }
        else if (nDims == 2 && dimIds[1] == _Voigt_dim) {
            // This is a tensor property, in Voigt notation
            startp[1] = 0;
            countp[1] = 6;
            componentCount = 6;
            nativeComponentCount = 6;
            nDimsDetected = 2;
        }
    }
}

/******************************************************************************
* Parses the given input file and stores the data in this container object.
******************************************************************************/
void NetCDFImporter::NetCDFImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading NetCDF file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

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
	for (int varId = 0; varId < nVars; varId++) {
		char name[NC_MAX_NAME+1];
		nc_type type;

		// Retrieve NetCDF meta-information.
		int nDims, dimIds[NC_MAX_VAR_DIMS];
		NCERR( nc_inq_var(_ncid, varId, name, &type, &nDims, dimIds, NULL) );

		// Check if dimensions make sense and we can understand them.
		if (dimIds[0] == _atom_dim || ( nDims > 1 && dimIds[0] == _frame_dim && dimIds[1] == _atom_dim )) {
			// Do we support this data type?
			if (type == NC_BYTE || type == NC_SHORT || type == NC_INT || type == NC_LONG || type == NC_CHAR) {
				columnMapping.push_back(mapVariableToColumn(name, qMetaTypeId<int>()));
			}
			else if (type == NC_FLOAT || type == NC_DOUBLE) {
				columnMapping.push_back(mapVariableToColumn(name, qMetaTypeId<FloatType>()));
			}
			else {
				qDebug() << "Skipping NetCDF variable " << name << " because type is not known.";
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
	if(_useCustomColumnMapping && !_customColumnMapping.empty())
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
	size_t startp[4] = { movieFrame, 0, 0, 0 };
	size_t countp[4] = { 1, 3, 0, 0 };
	if (_cell_origin_var != -1)
		NCERR( nc_get_vara_double(_ncid, _cell_origin_var, startp, countp, o) );
	NCERR( nc_get_vara_double(_ncid, _cell_lengths_var, startp, countp, l) );
	NCERR( nc_get_vara_double(_ncid, _cell_angles_var, startp, countp, a) );
	if (_shear_dx_var != -1)
		NCERR( nc_get_vara_double(_ncid, _shear_dx_var, startp, countp, d) );

    // Periodic boundary conditions. Non-periodic dimensions have length zero
    // according to AMBER specification.
    std::array<bool,3> pbc;
    for (int i = 0; i < 3; i++) {
        if (std::abs(l[i]) < 1e-12)  pbc[i] = false;
        else pbc[i] = true;
    }
    simulationCell().setPbcFlags(pbc);
	
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
	setProgressRange(columnMapping.size());

    // Now iterate over all variables and see if we have to reduce particleCount
    // We use the only float properties for this because at least one must be present (coordinates)
	for (const InputColumnInfo& column : columnMapping) {
		int dataType = column.dataType;

        if (dataType == qMetaTypeId<FloatType>()) {

			QString columnName = column.columnName;
            ParticleProperty::Type propertyType = column.property.type();

            // Retrieve NetCDF meta-information.
            nc_type type;
            int varId, nDims, dimIds[NC_MAX_VAR_DIMS];
            NCERR( nc_inq_varid(_ncid, columnName.toLocal8Bit().constData(), &varId) );
            NCERR( nc_inq_var(_ncid, varId, NULL, &type, &nDims, dimIds, NULL) );

            if (nDims > 0 && type == NC_FLOAT) {
                // Detect dims
                int nDimsDetected = -1, componentCount = 1, nativeComponentCount = 1;
                detectDims(movieFrame, particleCount, nDims, dimIds, nDimsDetected, componentCount, nativeComponentCount, startp, countp);

                std::unique_ptr<FloatType[]> data(new FloatType[nativeComponentCount*particleCount]);

#ifdef FLOATTYPE_FLOAT
                NCERRI( nc_get_vara_float(_ncid, varId, startp, countp, data.get()), tr("(While reading variable '%1'.)").arg(columnName) );
                while (particleCount > 0 && data[nativeComponentCount*(particleCount-1)] == NC_FILL_FLOAT)  particleCount--;
#else
                NCERRI( nc_get_vara_double(_ncid, varId, startp, countp, data.get()), tr("(While reading variable '%1'.)").arg(columnName) );
                while (particleCount > 0 && data[nativeComponentCount*(particleCount-1)] == NC_FILL_DOUBLE)  particleCount--;
#endif
            }

        }
    }

	// Now iterate over all variables and load the appropriate frame
	for (const InputColumnInfo& column : columnMapping) {
		if(isCanceled()) {
			closeNetCDF();
			return;
		}
		incrementProgressValue();
		
		ParticleProperty* property = nullptr;

		int dataType = column.dataType;
		QString columnName = column.columnName;
		QString propertyName = column.property.name();

		if (dataType != QMetaType::Void) {
			size_t dataTypeSize;
			if (dataType == qMetaTypeId<int>())
				dataTypeSize = sizeof(int);
			else if (dataType == qMetaTypeId<FloatType>())
				dataTypeSize = sizeof(FloatType);
			else
				throw Exception(tr("Invalid custom particle property (data type %1) for input file column %2 of NetCDF file.").arg(dataType).arg(columnName));

			// Retrieve NetCDF meta-information.
			nc_type type;
			int varId, nDims, dimIds[NC_MAX_VAR_DIMS];
			NCERR( nc_inq_varid(_ncid, columnName.toLocal8Bit().constData(), &varId) );
			NCERR( nc_inq_var(_ncid, varId, NULL, &type, &nDims, dimIds, NULL) );

			// Construct pointers to NetCDF dimension indices.
			countp[0] = 1;
			countp[1] = 1;
			countp[2] = 1;

			int nDimsDetected = -1, componentCount = 1, nativeComponentCount = 1;
			if (nDims > 0) {
                detectDims(movieFrame, particleCount, nDims, dimIds, nDimsDetected, componentCount, nativeComponentCount, startp, countp);

				// Skip all fields that don't have the expected format.
				if (nDimsDetected != -1 && (nDimsDetected == nDims || type == NC_CHAR)) {
					// Find property to load this information into.
					ParticleProperty::Type propertyType = column.property.type();

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
							property = new ParticleProperty(particleCount, propertyType, 0, true);
							addParticleProperty(property);
						}
					}
					else {
						// Look for existing user-defined property with the same name.
						for(int j = 0; j < (int)particleProperties().size(); j++) {
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
							property = new ParticleProperty(particleCount, dataType, dataTypeSize, componentCount, dataTypeSize * componentCount, propertyName, true);
							addParticleProperty(property);
						}
					}

					OVITO_ASSERT(property != nullptr);
					property->setName(propertyName);

					if (property->componentCount() != componentCount) {
						qDebug() << "Warning: Skipping field '" << columnName << "' of NetCDF file because internal and NetCDF component counts do not match.";
					}
					else {
						// Type mangling.
						if (property->dataType() == qMetaTypeId<int>()) {
							// This is integer data.

							if (componentCount == 6 && nativeComponentCount == 9 && type != NC_CHAR) {
								// Convert this property to Voigt notation.
								std::unique_ptr<int[]> data(new int[9*particleCount]);
								NCERRI( nc_get_vara_int(_ncid, varId, startp, countp, data.get()), tr("(While reading variable '%1'.)").arg(columnName));
								fullToVoigt(particleCount, data.get(), property->dataInt());
							}
							else {						
                                // Create particles types if this is the particle type property.
                                if (propertyType == ParticleProperty::ParticleTypeProperty) {
                                    if (type == NC_CHAR) {
                                        // We can only read this if there is an additional dimension
                                        if (nDims == nDimsDetected+1) {
											std::vector<int> dimids(nDims);
											NCERR( nc_inq_vardimid(_ncid, varId, dimids.data()) );

                                            size_t strLen;
                                            NCERR( nc_inq_dimlen(_ncid, dimids[nDims-1], &strLen) );

                                            startp[nDimsDetected] = 0;
                                            countp[nDimsDetected] = strLen;
                                            std::unique_ptr<char[]> particleNamesData(new char[strLen*particleCount]);
                                                
                                            // This is a string particle type, i.e. element names
                                            NCERRI( nc_get_vara_text(_ncid, varId, startp, countp, particleNamesData.get()), tr("(While reading variable '%1'.)").arg(columnName) );

                                            // Collect all distinct particle names
                                            QMap<QString, bool> discoveredParticleNames;
											for (size_t i = 0; i < particleCount; i++) {
                                                QString name = QString::fromLocal8Bit(&particleNamesData[strLen*i], strLen);
                                                name = name.trimmed();
                                                discoveredParticleNames[name] = true;
                                            }

                                            // Assing a particle type id to each particle name
                                            QMap<QString, bool>::const_iterator particleName = discoveredParticleNames.constBegin();
                                            QMap<QString, int> particleNameToType;
                                            int i = 0;
                                            while (particleName != discoveredParticleNames.constEnd()) {
                                                addParticleTypeId(i, particleName.key());
                                                particleNameToType[particleName.key()] = i;
                                                i++;
                                                particleName++;
                                            }

                                            // Convert particle names to particle ids and set them accordingly
                                            int *particleTypes = property->dataInt();
											for (size_t i = 0; i < particleCount; i++) {
                                                QString name = QString::fromLocal8Bit(&particleNamesData[strLen*i], strLen);
                                                name = name.trimmed();

                                                *particleTypes = particleNameToType.value(name);
                                                particleTypes++;
                                            }
                                        }
                                    }
                                    else {
                                        // This is an integer particle type, i.e. atomic numbers or internal element numbers
                                        NCERRI( nc_get_vara_int(_ncid, varId, startp, countp, property->dataInt()), tr("(While reading variable '%1'.)").arg(columnName) );

                                        // Find maximum atom type.
                                        int maxType = 0;
										for (size_t i = 0; i < particleCount; i++)
                                            maxType = std::max(property->getInt(i), maxType);
                                        
                                        // Count number of atoms for each type.
                                        QVector<int> typeCount(maxType+1, 0);
										for (size_t i = 0; i < particleCount; i++)
                                            typeCount[property->getInt(i)]++;
								
                                        for (int i = 0; i <= maxType; i++) {
                                            // Only define atom type if really present.
                                            if (typeCount[i] > 0)
                                                addParticleTypeId(i);
                                        }
                                    }
                                }
                                else {
                                    if (type != NC_CHAR) {
                                        NCERRI( nc_get_vara_int(_ncid, varId, startp, countp, property->dataInt()), tr("(While reading variable '%1'.)").arg(columnName) );
                                    }
                                }
							}
						}
						else if (property->dataType() == qMetaTypeId<FloatType>()) {
							// This is floating point data.

							if (componentCount == 6 && nativeComponentCount == 9) {
								// Convert this property to Voigt notation.
								std::unique_ptr<FloatType[]> data(new FloatType[9*particleCount]);
#ifdef FLOATTYPE_FLOAT
								NCERRI( nc_get_vara_float(_ncid, varId, startp, countp, data.get()), tr("(While reading variable '%1'.)").arg(columnName) );
#else
								NCERRI( nc_get_vara_double(_ncid, varId, startp, countp, data.get()), tr("(While reading variable '%1'.)").arg(columnName) );
#endif
								fullToVoigt(particleCount, data.get(), property->dataFloat());
							}
							else {
#ifdef FLOATTYPE_FLOAT
								NCERRI( nc_get_vara_float(_ncid, varId, startp, countp, property->dataFloat()), tr("(While reading variable '%1'.)").arg(columnName) );
#else
								NCERRI( nc_get_vara_double(_ncid, varId, startp, countp, property->dataFloat()), tr("(While reading variable '%1'.)").arg(columnName) );
#endif

                                // If this is the particle coordinates, check if we need to update pbcs.
                                if (propertyType == ParticleProperty::PositionProperty) {

                                    FloatType *r = property->dataFloat();
                                    // Do we have any non-periodic dimension?
                                    if (!(pbc[0] && pbc[1] && pbc[2])) {

                                        // Yes. Let's find the bounding box.
                                        // FIXME! As implemented, this works for rectangular cells only.
                                        FloatType minvals[3], maxvals[3];
                                        std::copy(r, r+3, minvals);
                                        std::copy(r, r+3, maxvals);
										for (size_t i = 0; i < particleCount; i++) {
                                            for (int k = 0; k < 3; k++) {
                                                minvals[k] = std::min(minvals[k], r[3*i+k]);
                                                maxvals[k] = std::max(maxvals[k], r[3*i+k]);
                                            }
                                        }

                                        // Compute new cell length and origin.
                                        for (int k = 0; k < 3; k++) {
                                            if (!pbc[k]) {
                                                l[k] = maxvals[k]-minvals[k];
                                                o[k] = minvals[k];
                                            }
                                        }

                                        // Set new cell.
                                        Vector3 va(l[0], 0, 0);
                                        Vector3 vb(l[1]*cos(a[2]), l[1]*sin(a[2]), 0);
                                        Vector3 vc(l[2]*cx+d[0], l[2]*cy+d[1], l[2]*cz);

                                        // Set simulation cell.
                                        simulationCell().setMatrix(AffineTransformation(va, vb, vc, Vector3(o[0], o[1], o[2])));

                                    }

                                }
							}
						}
						else {
							qDebug() << "Warning: Skipping field '" << columnName << "' of NetCDF file because it has an unrecognized data type.";
						}
					}
				}
			}
		}
	}

	setStatus(tr("%1 particles").arg(particleCount));
}

/******************************************************************************
 * Guesses the mapping of an input file field to one of OVITO's internal
 * particle properties.
 *****************************************************************************/
InputColumnInfo NetCDFImporter::mapVariableToColumn(const QString& name, int dataType)
{
	InputColumnInfo column;
	column.columnName = name;
	QString loweredName = name.toLower();
	if(loweredName == "coordinates") column.mapStandardColumn(ParticleProperty::PositionProperty, 0);
	else if(loweredName == "velocities") column.mapStandardColumn(ParticleProperty::VelocityProperty, 0);
	else if(loweredName == "id") column.mapStandardColumn(ParticleProperty::IdentifierProperty);
	else if(loweredName == "type" || loweredName == "element" || loweredName == "atom_types" || loweredName == "species") column.mapStandardColumn(ParticleProperty::ParticleTypeProperty);
	else if(loweredName == "mass") column.mapStandardColumn(ParticleProperty::MassProperty);
	else if(loweredName == "radius") column.mapStandardColumn(ParticleProperty::RadiusProperty);
	else if(loweredName == "c_cna" || loweredName == "pattern") column.mapStandardColumn(ParticleProperty::StructureTypeProperty);
	else if(loweredName == "c_epot") column.mapStandardColumn(ParticleProperty::PotentialEnergyProperty);
	else if(loweredName == "c_kpot") column.mapStandardColumn(ParticleProperty::KineticEnergyProperty);
	else if(loweredName == "c_stress[1]") column.mapStandardColumn(ParticleProperty::StressTensorProperty, 0);
	else if(loweredName == "c_stress[2]") column.mapStandardColumn(ParticleProperty::StressTensorProperty, 1);
	else if(loweredName == "c_stress[3]") column.mapStandardColumn(ParticleProperty::StressTensorProperty, 2);
	else if(loweredName == "c_stress[4]") column.mapStandardColumn(ParticleProperty::StressTensorProperty, 3);
	else if(loweredName == "c_stress[5]") column.mapStandardColumn(ParticleProperty::StressTensorProperty, 4);
	else if(loweredName == "c_stress[6]") column.mapStandardColumn(ParticleProperty::StressTensorProperty, 5);
	else if(loweredName == "selection") column.mapStandardColumn(ParticleProperty::SelectionProperty);
	else if(loweredName == "forces") column.mapStandardColumn(ParticleProperty::ForceProperty, 0);
	else {
		column.mapCustomColumn(name, dataType);
	}
	return column;
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
	FileSource* obj = nullptr;
	for(RefMaker* refmaker : dependents()) {
		obj = dynamic_object_cast<FileSource>(refmaker);
		if(obj) break;
	}
	if(!obj) return;

	// Start task that inspects the file header to determine the number of data columns.
	std::shared_ptr<NetCDFImportTask> inspectionTask = std::make_shared<NetCDFImportTask>(dataset()->container(), obj->frames().front());
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
	connect(editMappingButton, &QPushButton::clicked, this, &NetCDFImporterEditor::onEditColumnMapping);
}

/******************************************************************************
* Is called when the user pressed the "Edit column mapping" button.
******************************************************************************/
void NetCDFImporterEditor::onEditColumnMapping()
{
	if(NetCDFImporter* importer = static_object_cast<NetCDFImporter>(editObject()))
		importer->showEditColumnMappingDialog(mainWindow());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
