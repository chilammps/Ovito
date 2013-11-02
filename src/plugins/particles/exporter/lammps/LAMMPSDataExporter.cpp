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
#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/ParticleTypeProperty.h>
#include <plugins/particles/data/SimulationCell.h>
#include "LAMMPSDataExporter.h"
#include "../ParticleExporterSettingsDialog.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, LAMMPSDataExporter, ParticleExporter)

/******************************************************************************
* Opens the export settings dialog for this exporter service.
******************************************************************************/
bool LAMMPSDataExporter::showSettingsDialog(DataSet* dataset, const PipelineFlowState& state, QWidget* parent)
{
	ParticleExporterSettingsDialog dialog(parent, this, dataset, state);
	return (dialog.exec() == QDialog::Accepted);
}

/******************************************************************************
* Writes the particles of one animation frame to the current output file.
******************************************************************************/
bool LAMMPSDataExporter::exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress)
{
	// Get particle positions.
	ParticlePropertyObject* posProperty = findStandardProperty(ParticleProperty::PositionProperty, state);
	if(!posProperty)
		throw Exception(tr("No particle positions available. Cannot write LAMMPS file."));
	ParticlePropertyObject* velocityProperty = findStandardProperty(ParticleProperty::VelocityProperty, state);
	ParticlePropertyObject* identifierProperty = findStandardProperty(ParticleProperty::IdentifierProperty, state);
	ParticlePropertyObject* periodicImageProperty = findStandardProperty(ParticleProperty::PeriodicImageProperty, state);
	ParticleTypeProperty* particleTypeProperty = dynamic_object_cast<ParticleTypeProperty>(findStandardProperty(ParticleProperty::ParticleTypeProperty, state));

	// Get simulation cell info.
	SimulationCell* simulationCell = state.findObject<SimulationCell>();
	if(!simulationCell)
		throw Exception(tr("No simulation cell available. Cannot write LAMMPS file."));

	AffineTransformation simCell = simulationCell->cellMatrix();

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
		throw Exception(tr("Cannot save simulation cell to a LAMMPS data file. This type of non-orthogonal "
				"cell is not supported by LAMMPS and its file format. See the documentation of LAMMPS for details."));

	textStream() << "# LAMMPS data file written by OVITO" << endl;
	textStream() << posProperty->size() << " atoms" << endl;

	if(particleTypeProperty && particleTypeProperty->size() > 0) {
		int numParticleTypes = std::max(
				particleTypeProperty->particleTypes().size(),
				*std::max_element(particleTypeProperty->constDataInt(), particleTypeProperty->constDataInt() + particleTypeProperty->size()));
		textStream() << numParticleTypes << " atom types" << endl;
	}
	else textStream() << "1 atom types" << endl;

	textStream() << xlo << " " << xhi << " xlo xhi" << endl;
	textStream() << ylo << " " << yhi << " ylo yhi" << endl;
	textStream() << zlo << " " << zhi << " zlo zhi" << endl;
	if(xy != 0 || xz != 0 || yz != 0) {
		textStream() << xy << " " << xz << " " << yz << " xy xz yz" << endl;
	}
	textStream() << endl;

	size_t totalProgressCount = posProperty->size();
	if(velocityProperty) totalProgressCount += posProperty->size();
	size_t currentProgress = 0;

	// Write atomic positions.
	textStream() << "Atoms" << endl << endl;
	const Point3* p = posProperty->constDataPoint3();
	for(size_t i = 0; i < posProperty->size(); i++, ++p) {
		if(identifierProperty)
			textStream() << identifierProperty->getInt(i) << " ";
		else
			textStream() << (i+1) << " ";

		if(particleTypeProperty)
			textStream() << particleTypeProperty->getInt(i) << " ";
		else
			textStream() << "1 ";

		textStream() << p->x() << " " << p->y() << " " << p->z();

		if(periodicImageProperty) {
			const Point3I& pbc = periodicImageProperty->getPoint3I(i);
			textStream() << pbc.x() << " " << pbc.y() << " " << pbc.z();
		}
		textStream() << endl;

		currentProgress++;
		if((currentProgress % 1000) == 0) {
			progress.setPercentage(currentProgress * 100 / totalProgressCount);
			if(progress.wasCanceled())
				return false;
		}
	}

	// Write atomic velocities
	if(velocityProperty) {
		textStream() << endl << "Velocities" << endl << endl;
		const Vector3* v = velocityProperty->constDataVector3();
		for(size_t i = 0; i < velocityProperty->size(); i++, ++v) {
			if(identifierProperty)
				textStream() << identifierProperty->getInt(i) << " ";
			else
				textStream() << (i+1) << " ";

			textStream() << v->x() << " " << v->y() << " " << v->z() << endl;

			currentProgress++;
			if((currentProgress % 1000) == 0) {
				progress.setPercentage(currentProgress * 100 / totalProgressCount);
				if(progress.wasCanceled())
					return false;
			}
		}
	}

	return true;
}

};
