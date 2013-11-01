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
#include <particles/data/ParticlePropertyObject.h>
#include <particles/data/ParticleTypeProperty.h>
#include <particles/data/SimulationCell.h>
#include "IMDExporter.h"
#include "../ParticleExporterSettingsDialog.h"
#include "../OutputColumnMapping.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, IMDExporter, ParticleExporter)

/******************************************************************************
* Opens the export settings dialog for this exporter service.
******************************************************************************/
bool IMDExporter::showSettingsDialog(DataSet* dataset, const PipelineFlowState& state, QWidget* parent)
{
	ParticleExporterSettingsDialog dialog(parent, this, dataset, state);
	return (dialog.exec() == QDialog::Accepted);
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool IMDExporter::exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress)
{
	// Get particle positions.
	ParticlePropertyObject* posProperty = findStandardProperty(ParticleProperty::PositionProperty, state);
	if(!posProperty)
		throw Exception(tr("No particle positions available. Cannot write IMD file."));
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(findStandardProperty(ParticleProperty::ParticleTypeProperty, state));
	if(typeProperty && typeProperty->particleTypes().empty()) typeProperty = nullptr;
	ParticlePropertyObject* identifierProperty = findStandardProperty(ParticleProperty::IdentifierProperty, state);
	ParticlePropertyObject* velocityProperty = findStandardProperty(ParticleProperty::VelocityProperty, state);
	ParticlePropertyObject* massProperty = findStandardProperty(ParticleProperty::MassProperty, state);

	// Get simulation cell info.
	SimulationCell* simulationCell = state.findObject<SimulationCell>();
	if(!simulationCell)
		throw Exception(tr("No simulation cell available. Cannot write IMD file."));

	AffineTransformation simCell = simulationCell->cellMatrix();
	size_t atomsCount = posProperty->size();

	OutputColumnMapping columnMapping;
	QVector<QString> columnNames;
	textStream() << "#F A ";
	if(identifierProperty) {
		textStream() << "1 ";
		columnMapping.insertColumn(columnMapping.columnCount(), identifierProperty->type(), identifierProperty->name());
		columnNames.push_back("number");
	}
	else textStream() << "0 ";
	if(typeProperty) {
		textStream() << "1 ";
		columnMapping.insertColumn(columnMapping.columnCount(), typeProperty->type(), typeProperty->name());
		columnNames.push_back("type");
	}
	else textStream() << "0 ";
	if(massProperty) {
		textStream() << "1 ";
		columnMapping.insertColumn(columnMapping.columnCount(), massProperty->type(), massProperty->name());
		columnNames.push_back("mass");
	}
	else textStream() << "0 ";
	if(posProperty) {
		textStream() << "3 ";
		columnMapping.insertColumn(columnMapping.columnCount(), posProperty->type(), posProperty->name(), 0);
		columnMapping.insertColumn(columnMapping.columnCount(), posProperty->type(), posProperty->name(), 1);
		columnMapping.insertColumn(columnMapping.columnCount(), posProperty->type(), posProperty->name(), 2);
		columnNames.push_back("x");
		columnNames.push_back("y");
		columnNames.push_back("z");
	}
	else textStream() << "0 ";
	if(velocityProperty) {
		textStream() << "3 ";
		columnMapping.insertColumn(columnMapping.columnCount(), velocityProperty->type(), velocityProperty->name(), 0);
		columnMapping.insertColumn(columnMapping.columnCount(), velocityProperty->type(), velocityProperty->name(), 1);
		columnMapping.insertColumn(columnMapping.columnCount(), velocityProperty->type(), velocityProperty->name(), 2);
		columnNames.push_back("vx");
		columnNames.push_back("vy");
		columnNames.push_back("vz");
	}
	else textStream() << "0 ";

	int otherColumnsCount = 0;
	for(const auto& o : state.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(!property) continue;
		if(property == posProperty || property == typeProperty || property == identifierProperty || property == massProperty || property == velocityProperty)
			continue;
		if(property->type() == ParticleProperty::ColorProperty || property->type() == ParticleProperty::SelectionProperty)
			continue;
		for(size_t component = 0; component < property->componentCount(); component++) {
			columnMapping.insertColumn(columnMapping.columnCount(), property->type(), property->name(), component);
			otherColumnsCount += 1;
			QString columnName = property->nameWithComponent(component);
			columnName.remove(QRegExp("[^A-Za-z\\d_.]"));
			columnNames.push_back(columnName);
		}
	}
	textStream() << otherColumnsCount << endl;

	textStream() << "#C";
	Q_FOREACH(const QString& cname, columnNames)
		textStream() << " " << cname;
	textStream() << endl;

	textStream() << "#X " << simCell.column(0)[0] << " " << simCell.column(0)[1] << " " << simCell.column(0)[2] << endl;
	textStream() << "#Y " << simCell.column(1)[0] << " " << simCell.column(1)[1] << " " << simCell.column(1)[2] << endl;
	textStream() << "#Z " << simCell.column(2)[0] << " " << simCell.column(2)[1] << " " << simCell.column(2)[2] << endl;

	textStream() << "## Generated on " << QDateTime::currentDateTime().toString() << endl;
	textStream() << "## IMD file written by " << QCoreApplication::applicationName() << endl;
	textStream() << "#E" << endl;

	OutputColumnWriter columnWriter(columnMapping, state);
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
