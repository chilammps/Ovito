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
#include "IMDImporter.h"
#include "../InputColumnMapping.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, IMDImporter, ParticleImporter)

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool IMDImporter::checkFileFormat(QIODevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextParserStream stream(input, sourceLocation.path());

	// Read first header line.
	stream.readLine(1024);

	// Read first line.
	return stream.line().startsWith("#F A ");
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void IMDImporter::IMDImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Reading IMD file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Read first header line.
	stream.readLine();
	if(!stream.line().startsWith("#F"))
		throw Exception(tr("Not an IMD atom file."));
	QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
	if(tokens.size() < 2 || tokens[1] != "A")
		throw Exception(tr("Not an IMD atom file in ASCII format."));

	InputColumnMapping columnMapping;
	AffineTransformation cell = AffineTransformation::Identity();

	// Read remaining header lines
	for(;;) {
		stream.readLine();
		if(stream.line().isEmpty() || stream.line().at(0) != '#')
			throw Exception(tr("Invalid header in IMD atom file (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(stream.line().at(1) == '#') continue;
		else if(stream.line().at(1) == 'E') break;
		else if(stream.line().at(1) == 'C') {
			QStringList tokens = stream.lineString().split(ws_re, QString::SkipEmptyParts);
			int columnIndex = 0;
			for(int t = 1; t < tokens.size(); t++) {
				const QString& token = tokens[t];
				if(token == "mass") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::MassProperty);
				else if(token == "type") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::ParticleTypeProperty);
				else if(token == "number") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::IdentifierProperty);
				else if(token == "x") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::PositionProperty, 0);
				else if(token == "y") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::PositionProperty, 1);
				else if(token == "z") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::PositionProperty, 2);
				else if(token == "vx") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::VelocityProperty, 0);
				else if(token == "vy") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::VelocityProperty, 1);
				else if(token == "vz") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::VelocityProperty, 2);
				else if(token == "Epot") columnMapping.mapStandardColumn(columnIndex, ParticleProperty::PotentialEnergyProperty);
				else {
					bool isStandardProperty = false;
					QMap<QString, ParticleProperty::Type> standardPropertyList = ParticleProperty::standardPropertyList();
					QRegularExpression specialCharacters(QStringLiteral("[^A-Za-z\\d_]"));
					Q_FOREACH(ParticleProperty::Type id, standardPropertyList) {
						for(size_t component = 0; component < ParticleProperty::standardPropertyComponentCount(id); component++) {
							QString columnName = ParticleProperty::standardPropertyName(id);
							columnName.remove(specialCharacters);
							QStringList componentNames = ParticleProperty::standardPropertyComponentNames(id);
							if(!componentNames.empty()) {
								QString componentName = componentNames[component];
								componentName.remove(specialCharacters);
								columnName += componentName;
							}
							if(columnName == token) {
								columnMapping.mapStandardColumn(columnIndex, id, component);
								isStandardProperty = true;
								break;
							}
						}
						if(isStandardProperty) break;
					}
					if(!isStandardProperty)
						columnMapping.mapCustomColumn(columnIndex, QString(token), qMetaTypeId<FloatType>());
				}
				columnIndex++;
			}
		}
		else if(stream.line().at(1) == 'X') {
			if(sscanf(stream.line().constData() + 2, FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &cell(0,0), &cell(1,0), &cell(2,0)) != 3)
				throw Exception(tr("Invalid simulation cell bounds in line %1 of IMD file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
		}
		else if(stream.line().at(1) == 'Y') {
			if(sscanf(stream.line().constData() + 2, FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &cell(0,1), &cell(1,1), &cell(2,1)) != 3)
				throw Exception(tr("Invalid simulation cell bounds in line %1 of IMD file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
		}
		else if(stream.line().at(1) == 'Z') {
			if(sscanf(stream.line().constData() + 2, FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &cell(0,2), &cell(1,2), &cell(2,2)) != 3)
				throw Exception(tr("Invalid simulation cell bounds in line %1 of IMD file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
		}
		else throw Exception(tr("Invalid header line key in IMD atom file (line %2).").arg(stream.lineNumber()));
	}
	simulationCell().setMatrix(cell);

	// Save file position.
	qint64 headerOffset = stream.byteOffset();
	int headerLinerNumber = stream.lineNumber();

	// Count the number of atoms (=lines) in the input file.
	int numAtoms = 0;
	while(!stream.eof()) {
		if(stream.readLine().isEmpty()) break;
		numAtoms++;

		if((numAtoms % 1000) == 0 && futureInterface.isCanceled())
			return;
	}

	futureInterface.setProgressRange(numAtoms);

	// Jump back to beginning of atom list.
	stream.seek(headerOffset);

	// Parse data columns.
	InputColumnReader columnParser(columnMapping, *this, numAtoms);
	for(size_t i = 0; i < numAtoms; i++) {
		try {
			if((i % 4096) == 0) {
				if(futureInterface.isCanceled())
					return;	// Abort!
				futureInterface.setProgressValue((int)i);
			}
			stream.readLine();
			columnParser.readParticle(i, const_cast<QByteArray&>(stream.line()).data());
		}
		catch(Exception& ex) {
			throw ex.prependGeneralMessage(tr("Parsing error in line %1 of IMD file.").arg(headerLinerNumber + i));
		}
	}

	setInfoText(tr("Number of particles: %1").arg(numAtoms));
}

};
