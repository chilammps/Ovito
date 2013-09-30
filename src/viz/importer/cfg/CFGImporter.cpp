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
#include "CFGImporter.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, CFGImporter, ParticleImporter)

struct CFGHeader {

	int numParticles;
	FloatType unitMultiplier;
	Matrix3 H0;
	Matrix3 transform;
	FloatType rateScale;
	bool isExtendedFormat;
	bool containsVelocities;
	QStringList auxiliaryFields;

	void parse(CompressedTextParserStream& stream);
};

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool CFGImporter::checkFileFormat(QIODevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextParserStream stream(input, sourceLocation.path());

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
void CFGHeader::parse(CompressedTextParserStream& stream)
{
	using namespace std;

	numParticles = 0;
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
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void CFGImporter::CFGImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Reading CFG file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	CFGHeader header;
	header.parse(stream);

	InputColumnMapping cfgMapping;
	if(header.isExtendedFormat == false) {
		cfgMapping.mapStandardColumn(0, ParticleProperty::MassProperty);
		cfgMapping.mapStandardColumn(1, ParticleProperty::ParticleTypeProperty);
		cfgMapping.mapStandardColumn(2, ParticleProperty::PositionProperty, 0);
		cfgMapping.mapStandardColumn(3, ParticleProperty::PositionProperty, 1);
		cfgMapping.mapStandardColumn(4, ParticleProperty::PositionProperty, 2);
		cfgMapping.mapStandardColumn(5, ParticleProperty::VelocityProperty, 0);
		cfgMapping.mapStandardColumn(6, ParticleProperty::VelocityProperty, 1);
		cfgMapping.mapStandardColumn(7, ParticleProperty::VelocityProperty, 2);
	}
	else {
		int c = 3;
		cfgMapping.mapStandardColumn(0, ParticleProperty::PositionProperty, 0);
		cfgMapping.mapStandardColumn(1, ParticleProperty::PositionProperty, 1);
		cfgMapping.mapStandardColumn(2, ParticleProperty::PositionProperty, 2);
		if(header.containsVelocities) {
			cfgMapping.mapStandardColumn(3, ParticleProperty::VelocityProperty, 0);
			cfgMapping.mapStandardColumn(4, ParticleProperty::VelocityProperty, 1);
			cfgMapping.mapStandardColumn(5, ParticleProperty::VelocityProperty, 2);
			c = 6;
		}
		cfgMapping.setColumnCount(c + header.auxiliaryFields.size());
		for(int i = 0; i < header.auxiliaryFields.size(); i++, c++)
			cfgMapping.mapCustomColumn(c, header.auxiliaryFields[i], qMetaTypeId<FloatType>());
	}

	futureInterface.setProgressRange(header.numParticles);

	// Prepare the mapping between input file columns and particle properties.
	InputColumnReader columnParser(cfgMapping, *this, header.numParticles);

	// Create particle mass and type properties.
	int currentAtomType = 0;
	FloatType currentMass = 0;
	FloatType* massPointer = nullptr;
	int* atomTypePointer = nullptr;
	ParticleProperty* typeProperty = nullptr;
	if(header.isExtendedFormat) {
		typeProperty = new ParticleProperty(header.numParticles, ParticleProperty::ParticleTypeProperty);
		addParticleProperty(typeProperty);
		atomTypePointer = typeProperty->dataInt();
		ParticleProperty* massProperty = new ParticleProperty(header.numParticles, ParticleProperty::MassProperty);
		addParticleProperty(massProperty);
		massPointer = massProperty->dataFloat();
	}

	// Read per-particle data.
	bool isFirstLine = true;
	for(int particleIndex = 0; particleIndex < header.numParticles; ) {

		// Update progress indicator.
		if((particleIndex % 4000) == 0) {
			if(futureInterface.isCanceled())
				return;	// Abort!
			futureInterface.setProgressValue(particleIndex);
		}

		if(!isFirstLine)
			stream.readLine();
		else
			isFirstLine = false;

		if(header.isExtendedFormat) {
			bool isNewType = true;
			for(const char* line = stream.line(); *line != '\0'; ++line) {
				if(std::isspace(*line)) {
					for(; *line != '\0'; ++line) {
						if(!std::isspace(*line)) {
							isNewType = false;
							break;
						}
					}
					break;
				}
			}
			if(isNewType) {
				// Parse mass and atom type name.
				currentAtomType++;
				currentMass = atof(stream.line());
				stream.readLine();
				addParticleType(currentAtomType, stream.lineString().trimmed());
				continue;
			}

			*atomTypePointer++ = currentAtomType;
			*massPointer++ = currentMass;
		}

		try {
			columnParser.readParticle(particleIndex, const_cast<char*>(stream.line()));
			particleIndex++;
		}
		catch(Exception& ex) {
			throw ex.prependGeneralMessage(tr("Parsing error in line %1 of CFG file.").arg(stream.lineNumber()));
		}
	}

	AffineTransformation H(header.transform * header.H0);
	H.translation() = H * Vector3(-0.5, -0.5, -0.5);
	simulationCell().setMatrix(H);

	// The CFG file stores particle positions in reduced coordinates.
	// Rescale them now to absolute (Cartesian) coordinates.
	// However, do this only if no absolute coordinates have been read from the extra data columns in the CFG file.
	ParticleProperty* posProperty = particleProperty(ParticleProperty::PositionProperty);
	if(posProperty && header.numParticles > 0) {
		bool hasAbsoluteCoordinates = false;
#if 0
		for(int i = 0; i < columnMapping().columnCount(); i++) {
			if(columnMapping().propertyType(i) == ParticleProperty::PositionProperty) {
				hasAbsoluteCoordinates = true;
				break;
			}
		}
#endif
		if(!hasAbsoluteCoordinates) {
			Point3* p = posProperty->dataPoint3();
			Point3* pend = p + posProperty->size();
			for(; p != pend; ++p)
				*p = H * (*p);
		}
	}

	setInfoText(tr("Number of particles: %1").arg(header.numParticles));
}

};
