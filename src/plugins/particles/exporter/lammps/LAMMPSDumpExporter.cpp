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
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/SimulationCell.h>
#include "LAMMPSDumpExporter.h"
#include "../ParticleExporterSettingsDialog.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, LAMMPSDumpExporter, ParticleExporter)

/******************************************************************************
* Constructs a new instance of this class.
******************************************************************************/
LAMMPSDumpExporter::LAMMPSDumpExporter(DataSet* dataset) : ParticleExporter(dataset)
{
	// Use the last mapping by default.
	QSettings settings;
	settings.beginGroup("viz/exporter/lammpsdump/");
	if(settings.contains("columnmapping")) {
		try {
			_columnMapping.fromByteArray(settings.value("columnmapping").toByteArray());
		}
		catch(Exception& ex) {
			ex.prependGeneralMessage(tr("Failed to load last output column mapping from application settings store."));
			ex.logError();
		}
	}
	settings.endGroup();
}

/******************************************************************************
* Opens the export settings dialog for this exporter service.
******************************************************************************/
bool LAMMPSDumpExporter::showSettingsDialog(const PipelineFlowState& state, QWidget* parent)
{
	ParticleExporterSettingsDialog dialog(parent, this, state, &_columnMapping);
	if(dialog.exec() == QDialog::Accepted) {

		// Remember the output column mapping for the next time.
		QSettings settings;
		settings.beginGroup("viz/exporter/lammpsdump/");
		settings.setValue("columnmapping", _columnMapping.toByteArray());
		settings.endGroup();

		return true;
	}
	return false;
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool LAMMPSDumpExporter::exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress)
{
	// Get particle positions.
	ParticlePropertyObject* posProperty = findStandardProperty(ParticleProperty::PositionProperty, state);
	if(!posProperty)
		throw Exception(tr("No particle positions available. Cannot write LAMMPS file."));

	// Get simulation cell info.
	SimulationCell* simulationCell = state.findObject<SimulationCell>();
	if(!simulationCell)
		throw Exception(tr("No simulation cell available. Cannot write LAMMPS file."));

	AffineTransformation simCell = simulationCell->cellMatrix();
	size_t atomsCount = posProperty->size();

	FloatType xlo = simCell.translation().x();
	FloatType ylo = simCell.translation().y();
	FloatType zlo = simCell.translation().z();
	FloatType xhi = simCell.column(0).x() + xlo;
	FloatType yhi = simCell.column(1).y() + ylo;
	FloatType zhi = simCell.column(2).z() + zlo;
	FloatType xy = simCell.column(1).x();
	FloatType xz = simCell.column(2).x();
	FloatType yz = simCell.column(2).y();

	if(simCell.column(0).y() != 0 || simCell.column(0).z() != 0 || simCell.column(1).z() != 0)
		throw Exception(tr("Cannot save simulation cell to a LAMMPS dump file. This type of non-orthogonal "
				"cell is not supported by LAMMPS and its file format. See the documentation of LAMMPS for details."));

	xlo += std::min((FloatType)0, std::min(xy, std::min(xz, xy+xz)));
	xhi += std::max((FloatType)0, std::max(xy, std::max(xz, xy+xz)));
	ylo += std::min((FloatType)0, yz);
	yhi += std::max((FloatType)0, yz);

	textStream() << "ITEM: TIMESTEP" << endl;
	textStream() << frameNumber << endl;
	textStream() << "ITEM: NUMBER OF ATOMS" << endl;
	textStream() << atomsCount << endl;
	if(xy != 0 || xz != 0 || yz != 0) {
		textStream() << "ITEM: BOX BOUNDS xy xz yz";
		textStream() << (simulationCell->pbcX() ? " pp" : " ff");
		textStream() << (simulationCell->pbcY() ? " pp" : " ff");
		textStream() << (simulationCell->pbcZ() ? " pp" : " ff");
		textStream() << endl;
		textStream() << xlo << " " << xhi << " " << xy << endl;
		textStream() << ylo << " " << yhi << " " << xz << endl;
		textStream() << zlo << " " << zhi << " " << yz << endl;
	}
	else {
		textStream() << "ITEM: BOX BOUNDS";
		textStream() << (simulationCell->pbcX() ? " pp" : " ff");
		textStream() << (simulationCell->pbcY() ? " pp" : " ff");
		textStream() << (simulationCell->pbcZ() ? " pp" : " ff");
		textStream() << endl;
		textStream() << xlo << " " << xhi << endl;
		textStream() << ylo << " " << yhi << endl;
		textStream() << zlo << " " << zhi << endl;
	}
	textStream() << "ITEM: ATOMS";

	const OutputColumnMapping& mapping = columnMapping();
	if(mapping.columnCount() <= 0)
		throw Exception(tr("No particle properties have been selected for export to the LAMMPS dump file. Cannot write dump file with zero columns."));

	// Write column names.
	for(int i = 0; i < mapping.columnCount(); i++) {
		QString columnName;
		switch(mapping.propertyType(i)) {
		case ParticleProperty::PositionProperty:
			if(mapping.vectorComponent(i) == 0) columnName = QStringLiteral("x");
			else if(mapping.vectorComponent(i) == 1) columnName = QStringLiteral("y");
			else if(mapping.vectorComponent(i) == 2) columnName = QStringLiteral("z");
			else columnName = QStringLiteral("position");
			break;
		case ParticleProperty::VelocityProperty:
			if(mapping.vectorComponent(i) == 0) columnName = QStringLiteral("vx");
			else if(mapping.vectorComponent(i) == 1) columnName = QStringLiteral("vy");
			else if(mapping.vectorComponent(i) == 2) columnName = QStringLiteral("vz");
			else columnName = QStringLiteral("velocity");
			break;
		case ParticleProperty::ForceProperty:
			if(mapping.vectorComponent(i) == 0) columnName = QStringLiteral("fx");
			else if(mapping.vectorComponent(i) == 1) columnName = QStringLiteral("fy");
			else if(mapping.vectorComponent(i) == 2) columnName = QStringLiteral("fz");
			else columnName = QStringLiteral("force");
			break;
		case ParticleProperty::PeriodicImageProperty:
			if(mapping.vectorComponent(i) == 0) columnName = QStringLiteral("ix");
			else if(mapping.vectorComponent(i) == 1) columnName = QStringLiteral("iy");
			else if(mapping.vectorComponent(i) == 2) columnName = QStringLiteral("iz");
			else columnName = QStringLiteral("pbcimage");
			break;
		case ParticleProperty::IdentifierProperty: columnName = QStringLiteral("id"); break;
		case ParticleProperty::ParticleTypeProperty: columnName = QStringLiteral("type"); break;
		case ParticleProperty::MassProperty: columnName = QStringLiteral("mass"); break;
		case ParticleProperty::RadiusProperty: columnName = QStringLiteral("radius"); break;
		default:
			columnName = mapping.propertyName(i);
			columnName.remove(QRegExp("[^A-Za-z\\d_]"));
			if(mapping.propertyType(i) == ParticleProperty::UserProperty) {
				if(mapping.vectorComponent(i) > 0)
					columnName += "." + QString::number(mapping.vectorComponent(i));
			}
			else {
				QStringList componentNames = ParticleProperty::standardPropertyComponentNames(mapping.propertyType(i));
				if(mapping.vectorComponent(i) < componentNames.size()) {
					columnName += "." + componentNames[mapping.vectorComponent(i)];
				}
			}
		}
		textStream() << " " << columnName;
	}
	textStream() << endl;

	OutputColumnWriter columnWriter(mapping, state);
	for(size_t i = 0; i < atomsCount; i++) {
		columnWriter.writeParticle(i, textStream());
		textStream() << endl;

		if((i % 4096) == 0) {
			progress.setPercentage((quint64)i * 100 / atomsCount);
			if(progress.wasCanceled())
				return false;
		}
	}

	return true;
}

};
