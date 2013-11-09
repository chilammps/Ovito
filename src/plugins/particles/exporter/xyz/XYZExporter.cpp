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
#include "XYZExporter.h"
#include "../ParticleExporterSettingsDialog.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, XYZExporter, ParticleExporter)

/******************************************************************************
* Constructs a new instance of this class.
******************************************************************************/
XYZExporter::XYZExporter()
{
	// Use the last mapping by default.
	QSettings settings;
	settings.beginGroup("viz/exporter/xyz/");
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
bool XYZExporter::showSettingsDialog(DataSet* dataset, const PipelineFlowState& state, QWidget* parent)
{
	ParticleExporterSettingsDialog dialog(parent, this, dataset, state, &_columnMapping);
	if(dialog.exec() == QDialog::Accepted) {

		// Remember the output column mapping for the next time.
		QSettings settings;
		settings.beginGroup("viz/exporter/xyz/");
		settings.setValue("columnmapping", _columnMapping.toByteArray());
		settings.endGroup();

		return true;
	}
	return false;
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool XYZExporter::exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress)
{
	// Get particle positions.
	ParticlePropertyObject* posProperty = findStandardProperty(ParticleProperty::PositionProperty, state);
	if(!posProperty)
		throw Exception(tr("No particle positions available. Cannot write XYZ file."));

	size_t atomsCount = posProperty->size();
	textStream() << atomsCount << endl;

	textStream() << "Frame " << frameNumber;
	SimulationCell* simulationCell = state.findObject<SimulationCell>();
	if(simulationCell) {
		AffineTransformation simCell = simulationCell->cellMatrix();
		textStream() << " cell_orig " << simCell.translation().x() << " " << simCell.translation().y() << " " << simCell.translation().z();
		textStream() << " cell_vec1 " << simCell.column(0).x() << " " << simCell.column(0).y() << " " << simCell.column(0).z();
		textStream() << " cell_vec2 " << simCell.column(1).x() << " " << simCell.column(1).y() << " " << simCell.column(1).z();
		textStream() << " cell_vec3 " << simCell.column(2).x() << " " << simCell.column(2).y() << " " << simCell.column(2).z();
		textStream() << " pbc " << simulationCell->pbcX() << " " << simulationCell->pbcY() << " " << simulationCell->pbcZ();
	}
	textStream() << endl;

	const OutputColumnMapping& mapping = columnMapping();
	if(mapping.columnCount() <= 0)
		throw Exception(tr("No particle properties have been selected for export to the XYZ file. Cannot write file with zero columns."));

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
