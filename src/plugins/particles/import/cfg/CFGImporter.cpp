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
#include "CFGImporter.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CFGImporter, ParticleImporter);

struct CFGHeader {

	int numParticles;
	FloatType unitMultiplier;
	Matrix3 H0;
	Matrix3 transform;
	FloatType rateScale;
	bool isExtendedFormat;
	bool containsVelocities;
	QStringList auxiliaryFields;

	void parse(CompressedTextReader& stream);
};

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool CFGImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

	// Read first line.
	stream.readLine(20);

	// CFG files start with the string "Number of particles".
	if(stream.lineStartsWith("Number of particles"))
		return true;

	return false;
}

/******************************************************************************
* Parses the header of a CFG file.
******************************************************************************/
void CFGHeader::parse(CompressedTextReader& stream)
{
	using namespace std;

	numParticles = -1;
	unitMultiplier = 1;
	H0.setIdentity();
	transform.setIdentity();
	rateScale = 1;
	isExtendedFormat = false;
	containsVelocities = true;
	int entry_count = 0;

	while(!stream.eof()) {

		string line(stream.readLine());

		// Ignore comments
		size_t commentChar = line.find('#');
		if(commentChar != string::npos) line.resize(commentChar);

		// Skip empty lines.
		size_t trimmedLine = line.find_first_not_of(" \t\n\r");
		if(trimmedLine == string::npos) continue;
		if(trimmedLine != 0) line = line.substr(trimmedLine);

		size_t splitChar = line.find('=');
		if(splitChar == string::npos) {
			if(stream.lineStartsWith(".NO_VELOCITY.")) {
				containsVelocities = false;
				continue;
			}
			break;
		}

		string key = line.substr(0, line.find_last_not_of(" \t\n\r", splitChar - 1) + 1);
		size_t valuestart = line.find_first_not_of(" \t\n\r", splitChar + 1);
		if(valuestart == string::npos) valuestart = splitChar+1;
		string value = line.substr(valuestart);

		if(key == "Number of particles") {
			numParticles = atoi(value.c_str());
			if(numParticles < 0 || numParticles > 1e9)
				throw Exception(CFGImporter::tr("CFG file parsing error. Invalid number of atoms (line %1): %2").arg(stream.lineNumber()).arg(QString::fromStdString(value)));
		}
		else if(key == "A") unitMultiplier = atof(value.c_str());
		else if(key == "H0(1,1)") H0(0,0) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(1,2)") H0(0,1) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(1,3)") H0(0,2) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(2,1)") H0(1,0) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(2,2)") H0(1,1) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(2,3)") H0(1,2) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(3,1)") H0(2,0) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(3,2)") H0(2,1) = atof(value.c_str()) * unitMultiplier;
		else if(key == "H0(3,3)") H0(2,2) = atof(value.c_str()) * unitMultiplier;
		else if(key == "Transform(1,1)") transform(0,0) = atof(value.c_str());
		else if(key == "Transform(1,2)") transform(0,1) = atof(value.c_str());
		else if(key == "Transform(1,3)") transform(0,2) = atof(value.c_str());
		else if(key == "Transform(2,1)") transform(1,0) = atof(value.c_str());
		else if(key == "Transform(2,2)") transform(1,1) = atof(value.c_str());
		else if(key == "Transform(2,3)") transform(1,2) = atof(value.c_str());
		else if(key == "Transform(3,1)") transform(2,0) = atof(value.c_str());
		else if(key == "Transform(3,2)") transform(2,1) = atof(value.c_str());
		else if(key == "Transform(3,3)") transform(2,2) = atof(value.c_str());
		else if(key == "eta(1,1)") {}
		else if(key == "eta(1,2)") {}
		else if(key == "eta(1,3)") {}
		else if(key == "eta(2,2)") {}
		else if(key == "eta(2,3)") {}
		else if(key == "eta(3,3)") {}
		else if(key == "R") rateScale = atof(value.c_str());
		else if(key == "entry_count") {
			entry_count = atoi(value.c_str());
			isExtendedFormat = true;
		}
		else if(key.compare(0, 10, "auxiliary[") == 0) {
			isExtendedFormat = true;
			size_t endOfName = value.find_first_of(" \t");
			auxiliaryFields.push_back(QString::fromStdString(value.substr(0, endOfName)).trimmed());
		}
		else {
			throw Exception(CFGImporter::tr("Unknown key in CFG file header at line %1: %2").arg(stream.lineNumber()).arg(QString::fromStdString(line)));
		}
	}
	if(numParticles < 0)
		throw Exception(CFGImporter::tr("Invalid file header. This is not a valid CFG file."));
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void CFGImporter::CFGImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading CFG file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	CFGHeader header;
	header.parse(stream);

	InputColumnMapping cfgMapping;
	if(header.isExtendedFormat == false) {
		cfgMapping.resize(8);
		cfgMapping[0].mapStandardColumn(ParticleProperty::MassProperty);
		cfgMapping[1].mapStandardColumn(ParticleProperty::ParticleTypeProperty);
		cfgMapping[2].mapStandardColumn(ParticleProperty::PositionProperty, 0);
		cfgMapping[3].mapStandardColumn(ParticleProperty::PositionProperty, 1);
		cfgMapping[4].mapStandardColumn(ParticleProperty::PositionProperty, 2);
		cfgMapping[5].mapStandardColumn(ParticleProperty::VelocityProperty, 0);
		cfgMapping[6].mapStandardColumn(ParticleProperty::VelocityProperty, 1);
		cfgMapping[7].mapStandardColumn(ParticleProperty::VelocityProperty, 2);
	}
	else {
		cfgMapping.resize(header.containsVelocities ? 6 : 3);
		cfgMapping[0].mapStandardColumn(ParticleProperty::PositionProperty, 0);
		cfgMapping[1].mapStandardColumn(ParticleProperty::PositionProperty, 1);
		cfgMapping[2].mapStandardColumn(ParticleProperty::PositionProperty, 2);
		if(header.containsVelocities) {
			cfgMapping[3].mapStandardColumn(ParticleProperty::VelocityProperty, 0);
			cfgMapping[4].mapStandardColumn(ParticleProperty::VelocityProperty, 1);
			cfgMapping[5].mapStandardColumn(ParticleProperty::VelocityProperty, 2);
		}
		generateAutomaticColumnMapping(cfgMapping, header.auxiliaryFields);
	}

	setProgressRange(header.numParticles);

	// Prepare the mapping between input file columns and particle properties.
	InputColumnReader columnParser(cfgMapping, *this, header.numParticles);

	// Create particle mass and type properties.
	int currentAtomType = 0;
	FloatType currentMass = 0;
	FloatType* massPointer = nullptr;
	int* atomTypePointer = nullptr;
	ParticleProperty* typeProperty = nullptr;
	if(header.isExtendedFormat) {
		typeProperty = new ParticleProperty(header.numParticles, ParticleProperty::ParticleTypeProperty, 0, false);
		addParticleProperty(typeProperty);
		atomTypePointer = typeProperty->dataInt();
		ParticleProperty* massProperty = new ParticleProperty(header.numParticles, ParticleProperty::MassProperty, 0, false);
		addParticleProperty(massProperty);
		massPointer = massProperty->dataFloat();
	}

	// Read per-particle data.
	bool isFirstLine = true;
	for(int particleIndex = 0; particleIndex < header.numParticles; ) {

		// Update progress indicator.
		if(!reportProgress(particleIndex)) return;

		if(!isFirstLine)
			stream.readLine();
		else
			isFirstLine = false;

		if(header.isExtendedFormat) {
			bool isNewType = true;
			for(const char* line = stream.line(); *line != '\0'; ++line) {
				if(*line <= ' ') {
					for(; *line != '\0'; ++line) {
						if(*line > ' ') {
							isNewType = false;
							break;
						}
					}
					break;
				}
			}
			if(isNewType) {
				// Parse mass and atom type name.
				currentMass = atof(stream.line());
				const char* line = stream.readLine();
				while(*line != '\0' && *line <= ' ') ++line;
				const char* line_end = line;
				while(*line_end != '\0' && *line_end > ' ') ++line_end;
				currentAtomType = addParticleTypeName(line, line_end);

				continue;
			}

			*atomTypePointer++ = currentAtomType;
			*massPointer++ = currentMass;
		}

		try {
			columnParser.readParticle(particleIndex, stream.line());
			particleIndex++;
		}
		catch(Exception& ex) {
			throw ex.prependGeneralMessage(tr("Parsing error in line %1 of CFG file.").arg(stream.lineNumber()));
		}
	}

	// Since we created particle types on the go while reading the particle, the assigned particle type IDs
	// depends on the storage order of particles in the file. We rather want a well-defined particle type ordering, that's
	// why we sort them now.
	if(header.isExtendedFormat || columnParser.usingNamedParticleTypes())
		sortParticleTypesByName();
	else
		sortParticleTypesById();

	AffineTransformation H((header.transform * header.H0).transposed());
	H.translation() = H * Vector3(-0.5f, -0.5f, -0.5f);
	simulationCell().setMatrix(H);

	// The CFG file stores particle positions in reduced coordinates.
	// Rescale them now to absolute (Cartesian) coordinates.
	// However, do this only if no absolute coordinates have been read from the extra data columns in the CFG file.
	ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
	if(posProperty && header.numParticles > 0) {
		Point3* p = posProperty->dataPoint3();
		Point3* pend = p + posProperty->size();
		for(; p != pend; ++p)
			*p = H * (*p);
	}

	setStatus(tr("Number of particles: %1").arg(header.numParticles));
}

/******************************************************************************
 * Guesses the mapping of input file columns to internal particle properties.
 *****************************************************************************/
void CFGImporter::generateAutomaticColumnMapping(InputColumnMapping& columnMapping, const QStringList& columnNames)
{
	int i = columnMapping.size();
	columnMapping.resize(columnMapping.size() + columnNames.size());
	for(int j = 0; j < columnNames.size(); j++, i++) {
		columnMapping[i].columnName = columnNames[j];
		QString name = columnNames[j].toLower();
		if(name == "vx" || name == "velocities") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityProperty, 0);
		else if(name == "vy") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityProperty, 1);
		else if(name == "vz") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityProperty, 2);
		else if(name == "v") columnMapping[i].mapStandardColumn(ParticleProperty::VelocityMagnitudeProperty);
		else if(name == "id") columnMapping[i].mapStandardColumn(ParticleProperty::IdentifierProperty);
		else if(name == "radius") columnMapping[i].mapStandardColumn(ParticleProperty::RadiusProperty);
		else if(name == "q") columnMapping[i].mapStandardColumn(ParticleProperty::ChargeProperty);
		else if(name == "ix") columnMapping[i].mapStandardColumn(ParticleProperty::PeriodicImageProperty, 0);
		else if(name == "iy") columnMapping[i].mapStandardColumn(ParticleProperty::PeriodicImageProperty, 1);
		else if(name == "iz") columnMapping[i].mapStandardColumn(ParticleProperty::PeriodicImageProperty, 2);
		else if(name == "fx") columnMapping[i].mapStandardColumn(ParticleProperty::ForceProperty, 0);
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
		else {
			columnMapping[i].mapCustomColumn(columnNames[j], qMetaTypeId<FloatType>());
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
