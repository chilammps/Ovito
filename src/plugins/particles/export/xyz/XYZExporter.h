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

#ifndef __OVITO_XYZ_FILE_EXPORTER_H
#define __OVITO_XYZ_FILE_EXPORTER_H

#include <plugins/particles/Particles.h>
#include "../ParticleExporter.h"
#include "../OutputColumnMapping.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export) OVITO_BEGIN_INLINE_NAMESPACE(Formats)

/**
 * \brief Exporter that writes the particles to a LAMMPS data file.
 */
class OVITO_PARTICLES_EXPORT XYZExporter : public ParticleExporter
{
public:

	/// \brief The supported XYZ sub-formats.
	enum XYZSubFormat {
		ParcasFormat,
		ExtendedFormat
	};
	Q_ENUMS(XYZSubFormat);

public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE XYZExporter(DataSet* dataset) : ParticleExporter(dataset), _subFormat(ExtendedFormat) {}

	/// \brief Returns the file filter that specifies the files that can be exported by this service.
	virtual QString fileFilter() override { return QStringLiteral("*"); }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	virtual QString fileFilterDescription() override { return tr("XYZ File"); }

	/// \brief Opens the export settings dialog for this exporter service.
	virtual bool showSettingsDialog(const PipelineFlowState& state, QWidget* parent) override;

	/// \brief Returns the mapping of particle properties to output file columns.
	const OutputColumnMapping& columnMapping() const { return _columnMapping; }

	/// \brief Sets the mapping of particle properties to output file columns.
	void setColumnMapping(const OutputColumnMapping& mapping) { _columnMapping = mapping; }

	/// Returns the format variant being written by this XYZ file exporter.
	XYZSubFormat subFormat() const { return _subFormat; }

	/// Sets the kind of XYZ to write.
	void setSubFormat(XYZSubFormat subFormat) { _subFormat = subFormat; }

protected:

	/// \brief Writes the particles of one animation frame to the current output file.
	virtual bool exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress) override;

private:

	/// The mapping particle properties to output file columns.
	OutputColumnMapping _columnMapping;

	/// Selects the kind of XYZ to write.
	XYZSubFormat _subFormat;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Particles::XYZExporter::XYZSubFormat);
Q_DECLARE_TYPEINFO(Ovito::Particles::XYZExporter::XYZSubFormat, Q_PRIMITIVE_TYPE);

#endif // __OVITO_XYZ_FILE_EXPORTER_H
