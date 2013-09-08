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
#include <viz/data/ParticlePropertyObject.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/SimulationCell.h>
#include "POSCARExporter.h"
#include "../ParticleExporterSettingsDialog.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, POSCARExporter, ParticleExporter)

/******************************************************************************
* Opens the export settings dialog for this exporter service.
******************************************************************************/
bool POSCARExporter::showSettingsDialog(DataSet* dataset, const PipelineFlowState& state, QWidget* parent)
{
	ParticleExporterSettingsDialog dialog(parent, this, dataset, state);
	return (dialog.exec() == QDialog::Accepted);
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool POSCARExporter::exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress)
{
	// Get particle positions.
	ParticlePropertyObject* posProperty = findStandardProperty(ParticleProperty::PositionProperty, state);
	ParticlePropertyObject* velocityProperty = findStandardProperty(ParticleProperty::VelocityProperty, state);
	if(!posProperty)
		throw Exception(tr("No particle positions available. Cannot write POSCAR file."));

	// Get simulation cell info.
	SimulationCell* simulationCell = state.findObject<SimulationCell>();
	if(!simulationCell)
		throw Exception(tr("No simulation cell available. Cannot write POSCAR file."));

	// Write POSCAR header including the simulation cell geometry.
	textStream() << "POSCAR file written by OVITO" << endl;
	textStream() << "1" << endl;
	AffineTransformation cell = simulationCell->cellMatrix();
	for(size_t i = 0; i < 3; i++)
		textStream() << cell(0, i) << " " << cell(1, i) << " " << cell(2, i) << endl;

	// Count number of particle per particle type.
	QMap<int,int> particleCounts;
	ParticleTypeProperty* particleTypeProperty = dynamic_object_cast<ParticleTypeProperty>(findStandardProperty(ParticleProperty::ParticleTypeProperty, state));
	if(particleTypeProperty) {
		const int* ptype = particleTypeProperty->constDataInt();
		const int* ptype_end = ptype + particleTypeProperty->size();
		for(; ptype != ptype_end; ++ptype) {
			particleCounts[*ptype]++;
		}
	}
	else particleCounts[0] = posProperty->size();

	for(auto c = particleCounts.begin(); c != particleCounts.end(); ++c) {
		textStream() << c.value() << " ";
	}
	textStream() << endl;

	size_t totalProgressCount = posProperty->size();
	if(velocityProperty) totalProgressCount += posProperty->size();
	size_t currentProgress = 0;

	// Write atomic positions.
	textStream() << "Cartesian" << endl;
	for(auto c = particleCounts.begin(); c != particleCounts.end(); ++c) {
		int ptype = c.key();
		const Point3* p = posProperty->constDataPoint3();
		for(size_t i = 0; i < posProperty->size(); i++, ++p) {
			if(particleTypeProperty && particleTypeProperty->getInt(i) != ptype)
				continue;
			textStream() << p->x() << " " << p->y() << " " << p->z() << endl;
			currentProgress++;

			if((currentProgress % 1000) == 0) {
				progress.setPercentage(currentProgress * 100 / totalProgressCount);
				if(progress.wasCanceled())
					return false;
			}
		}
	}

	// Write atomic velocities.
	if(velocityProperty) {
		textStream() << "Cartesian" << endl;
		for(auto c = particleCounts.begin(); c != particleCounts.end(); ++c) {
			int ptype = c.key();
			const Vector3* v = velocityProperty->constDataVector3();
			for(size_t i = 0; i < velocityProperty->size(); i++, ++v) {
				if(particleTypeProperty && particleTypeProperty->getInt(i) != ptype)
					continue;
				textStream() << v->x() << " " << v->y() << " " << v->z() << endl;
				currentProgress++;

				if((currentProgress % 1000) == 0) {
					progress.setPercentage(currentProgress * 100 / totalProgressCount);
					if(progress.wasCanceled())
						return false;
				}
			}
		}
	}

	return true;
}

};
