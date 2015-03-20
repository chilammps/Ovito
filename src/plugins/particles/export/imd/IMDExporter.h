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

#ifndef __OVITO_IMD_FILE_EXPORTER_H
#define __OVITO_IMD_FILE_EXPORTER_H

#include <plugins/particles/Particles.h>
#include "../ParticleExporter.h"
#include "../OutputColumnMapping.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

/**
 * \brief Exporter that writes the particles to an IMD file.
 */
class OVITO_PARTICLES_EXPORT IMDExporter : public ParticleExporter
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE IMDExporter(DataSet* dataset) : ParticleExporter(dataset) {}

	/// \brief Returns the file filter that specifies the files that can be exported by this service.
	virtual QString fileFilter() override { return QStringLiteral("*"); }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	virtual QString fileFilterDescription() override { return tr("IMD File"); }

	/// \brief Opens the export settings dialog for this exporter service.
	virtual bool showSettingsDialog(const PipelineFlowState& state, QWidget* parent) override;

	/// \brief Returns the mapping of particle properties to output file columns.
	const OutputColumnMapping& columnMapping() const { return _columnMapping; }

	/// \brief Sets the mapping of particle properties to output file columns.
	void setColumnMapping(const OutputColumnMapping& mapping) { _columnMapping = mapping; }

public:

	Q_PROPERTY(Ovito::Particles::OutputColumnMapping columnMapping READ columnMapping WRITE setColumnMapping);

protected:

	/// \brief Writes the particles of one animation frame to the current output file.
	virtual bool exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress) override;

private:

	/// The mapping particle properties to output file columns.
	OutputColumnMapping _columnMapping;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_IMD_FILE_EXPORTER_H
