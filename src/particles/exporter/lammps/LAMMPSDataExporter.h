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

#ifndef __OVITO_LAMMPS_DATA_FILE_EXPORTER_H
#define __OVITO_LAMMPS_DATA_FILE_EXPORTER_H

#include <core/Core.h>
#include "../ParticleExporter.h"

namespace Particles {

using namespace Ovito;

/**
 * Exporter service that writes the particles to a LAMMPS data file.
 */
class LAMMPSDataExporter : public ParticleExporter
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE LAMMPSDataExporter() {}

	/// \brief Returns the file filter that specifies the files that can be exported by this service.
	virtual QString fileFilter() override { return "*"; }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	virtual QString fileFilterDescription() override { return tr("LAMMPS Data File"); }

	/// \brief Opens the export settings dialog for this exporter service.
	virtual bool showSettingsDialog(DataSet* dataset, const PipelineFlowState& state, QWidget* parent) override;

protected:

	/// \brief Writes the particles of one animation frame to the current output file.
	virtual bool exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_LAMMPS_DATA_FILE_EXPORTER_H
