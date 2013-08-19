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

#ifndef __OVITO_PARTICLE_EXPORTER_H
#define __OVITO_PARTICLE_EXPORTER_H

#include <core/Core.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include <core/dataset/importexport/FileExporter.h>

namespace Viz {

/**
 * \brief Abstract base class for export services that write the particles to a file.
 */
class ParticleExporter : public FileExporter
{
protected:

	/// \brief Constructs a new instance of this class.
	ParticleExporter() {
		INIT_PROPERTY_FIELD(ParticleExporter::_outputFilename);
	}

public:

	/////////////////////////// from FileExporter /////////////////////////////

	/// \brief Exports the scene to the given file.
	virtual bool exportToFile(const QString& filePath, DataSet* scene) override;

	/////////////////////////// Specific methods //////////////////////////////

	/// \brief Sets the name of the output file that should be written by this exporter.
	virtual void setOutputFile(const QString& filename) { _outputFilename = filename; }

	/// \brief Returns the path of the output file written by this exporter.
	const QString& outputFile() const { return _outputFilename; }

	/// \brief Opens the export settings dialog for this exporter service.
	/// \param atomsObj The AtomsObject to be exported.
	/// \param parent The widget to be used as parent for the settings dialog.
	/// \return \a true if the dialog has been approved by the user; \a false when the user has canceled the operation.
	///
	/// The default implementation of this method does not show any dialog and always returns \a true.
	///
	/// \note The output file name has to be set via setOutputFile() before this method may be called.
	virtual bool showSettingsDialog(AtomsObject* atomsObj, QWidget* parent) { return true; }

	/// \brief Exports the particles contained in the given scene to the output file.
	/// \param dataset The scene that contains the data to be exported.
	/// \throws Exception on error.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	virtual bool exportParticles(DataSet* dataset) = 0;

protected:

	/// \brief Returns the particles to be exported by evaluating the modification pipeline.
	/// \param dataset The scene to be exported.
	/// \param time The animation time at which to request the particles.
	/// \return The pipeline result containing the particles to be exported.
	///         The returned PipelineFlowState might be empty if there is no particle  object in the scene.
	PipelineFlowState getParticles(DataSet* dataset, TimePoint time);

private:

	/// The output file path.
	PropertyField<QString> _outputFilename;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_outputFilename);
};

};	// End of namespace

#endif // __OVITO_PARTICLE_EXPORTER_H
