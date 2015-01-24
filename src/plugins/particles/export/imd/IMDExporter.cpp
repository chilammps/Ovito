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
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include "IMDExporter.h"
#include "../ParticleExporterSettingsDialog.h"
#include "../OutputColumnMapping.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, IMDExporter, ParticleExporter);

/******************************************************************************
* Opens the export settings dialog for this exporter service.
******************************************************************************/
bool IMDExporter::showSettingsDialog(const PipelineFlowState& state, QWidget* parent)
{
	ParticleExporterSettingsDialog dialog(parent, this, state);
	return (dialog.exec() == QDialog::Accepted);
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool IMDExporter::exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress)
{
	// Get particle positions.
	ParticlePropertyObject* posProperty = ParticlePropertyObject::findInState(state, ParticleProperty::PositionProperty);
	if(!posProperty)
		throw Exception(tr("No particle positions available. Cannot write IMD file."));
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(ParticlePropertyObject::findInState(state, ParticleProperty::ParticleTypeProperty));
	if(typeProperty && typeProperty->particleTypes().empty()) typeProperty = nullptr;
	ParticlePropertyObject* identifierProperty = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);
	ParticlePropertyObject* velocityProperty = ParticlePropertyObject::findInState(state, ParticleProperty::VelocityProperty);
	ParticlePropertyObject* massProperty = ParticlePropertyObject::findInState(state, ParticleProperty::MassProperty);

	// Get simulation cell info.
	SimulationCellObject* simulationCell = state.findObject<SimulationCellObject>();
	if(!simulationCell)
		throw Exception(tr("No simulation cell available. Cannot write IMD file."));

	AffineTransformation simCell = simulationCell->cellMatrix();
	size_t atomsCount = posProperty->size();

	OutputColumnMapping columnMapping;
	QVector<QString> columnNames;
	textStream() << "#F A ";
	if(identifierProperty) {
		textStream() << "1 ";
		columnMapping.emplace_back(identifierProperty->type(), identifierProperty->name());
		columnNames.push_back("number");
	}
	else textStream() << "0 ";
	if(typeProperty) {
		textStream() << "1 ";
		columnMapping.emplace_back(typeProperty->type(), typeProperty->name());
		columnNames.push_back("type");
	}
	else textStream() << "0 ";
	if(massProperty) {
		textStream() << "1 ";
		columnMapping.emplace_back(massProperty->type(), massProperty->name());
		columnNames.push_back("mass");
	}
	else textStream() << "0 ";
	if(posProperty) {
		textStream() << "3 ";
		columnMapping.emplace_back(posProperty->type(), posProperty->name(), 0);
		columnMapping.emplace_back(posProperty->type(), posProperty->name(), 1);
		columnMapping.emplace_back(posProperty->type(), posProperty->name(), 2);
		columnNames.push_back("x");
		columnNames.push_back("y");
		columnNames.push_back("z");
	}
	else textStream() << "0 ";
	if(velocityProperty) {
		textStream() << "3 ";
		columnMapping.emplace_back(velocityProperty->type(), velocityProperty->name(), 0);
		columnMapping.emplace_back(velocityProperty->type(), velocityProperty->name(), 1);
		columnMapping.emplace_back(velocityProperty->type(), velocityProperty->name(), 2);
		columnNames.push_back("vx");
		columnNames.push_back("vy");
		columnNames.push_back("vz");
	}
	else textStream() << "0 ";

	int otherColumnsCount = 0;
	for(DataObject* o : state.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
		if(!property) continue;
		if(property == posProperty || property == typeProperty || property == identifierProperty || property == massProperty || property == velocityProperty)
			continue;
		if(property->type() == ParticleProperty::ColorProperty || property->type() == ParticleProperty::SelectionProperty)
			continue;
		for(size_t component = 0; component < property->componentCount(); component++) {
			columnMapping.emplace_back(property->type(), property->name(), component);
			otherColumnsCount += 1;
			QString columnName = property->nameWithComponent(component);
			columnName.remove(QRegExp("[^A-Za-z\\d_.]"));
			columnNames.push_back(columnName);
		}
	}
	textStream() << otherColumnsCount << "\n";

	textStream() << "#C";
	Q_FOREACH(const QString& cname, columnNames)
		textStream() << " " << cname;
	textStream() << "\n";

	textStream() << "#X " << simCell.column(0)[0] << " " << simCell.column(0)[1] << " " << simCell.column(0)[2] << "\n";
	textStream() << "#Y " << simCell.column(1)[0] << " " << simCell.column(1)[1] << " " << simCell.column(1)[2] << "\n";
	textStream() << "#Z " << simCell.column(2)[0] << " " << simCell.column(2)[1] << " " << simCell.column(2)[2] << "\n";

	textStream() << "## Generated on " << QDateTime::currentDateTime().toString() << "\n";
	textStream() << "## IMD file written by " << QCoreApplication::applicationName() << "\n";
	textStream() << "#E\n";

	OutputColumnWriter columnWriter(columnMapping, state);
	for(size_t i = 0; i < atomsCount; i++) {
		columnWriter.writeParticle(i, textStream());

		if((i % 4096) == 0) {
			progress.setPercentage((quint64)i * 100 / atomsCount);
			if(progress.wasCanceled())
				return false;
		}
	}

	return true;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
