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

#include <plugins/particles/Particles.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include <core/dataset/importexport/FileExporter.h>
#include <core/utilities/io/CompressedTextWriter.h>
#include <plugins/particles/data/ParticleProperty.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export)

/**
 * \brief Abstract base class for export services that write the particles to a file.
 */
class OVITO_PARTICLES_EXPORT ParticleExporter : public FileExporter
{
protected:

	/// \brief Constructs a new instance of this class.
	ParticleExporter(DataSet* dataset);

public:

	/////////////////////////// from FileExporter /////////////////////////////

	/// \brief Exports the scene nodes to the given file.
	virtual bool exportToFile(const QVector<SceneNode*>& nodes, const QString& filePath, bool noninteractive) override;

	/////////////////////////// Specific methods //////////////////////////////

	/// \brief Sets the name of the output file that should be written by this exporter.
	virtual void setOutputFilename(const QString& filename);

	/// \brief Returns the path of the output file written by this exporter.
	const QString& outputFilename() const { return _outputFilename; }

	/// \brief Opens the export settings dialog for this exporter service.
	///
	/// \param state The result of the pipeline evaluation. Contains the particle data to be exported.
	/// \param parent The widget to be used as parent for the settings dialog.
	/// \return \a true if the dialog has been approved by the user; \a false when the user has canceled the operation.
	///
	/// The default implementation of this method does not show any dialog and always returns \a true.
	///
	/// \note The output file name has to be set via setOutputFile() before this method may be called.
	virtual bool showSettingsDialog(const PipelineFlowState& state, QWidget* parent) { return true; }

	/// \brief Exports the particles to the output file(s).
	/// \param nodes The selected scene nodes to be exported.
	/// \throws Exception on error.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	virtual bool writeOutputFiles(const QVector<SceneNode*>& nodes);

	/// Returns whether only the current animation frame or an entire animation interval should be exported.
	bool exportAnimation() const { return _exportAnimation; }

	/// Sets whether only the current animation frame or an entire animation interval should be exported.
	void setExportAnimation(bool exportAnim) { _exportAnimation = exportAnim; }

	/// \brief Controls whether the exporter should produce separate files for each exported frame.
	void setUseWildcardFilename(bool enable) { _useWildcardFilename = enable; }

	/// \brief Returns whether the exporter produces separate files for each exported frame.
	bool useWildcardFilename() const { return _useWildcardFilename; }

	/// \brief Sets the wildcard pattern used to generate filenames when writing
	///        a separate file for each exported frame.
	///
	/// The wildcard filename must contain the character '*', which will be replaced by the
	/// frame number.
	void setWildcardFilename(const QString& filename) { _wildcardFilename = filename; }

	/// \brief Returns the wild-card pattern used to generate filenames when writing
	///        a separate file for each exported frame.
	const QString& wildcardFilename() const { return _wildcardFilename; }

	/// \brief Sets the start of the animation interval that should be exported to the output file.
	void setStartFrame(int frame) { _startFrame = frame; }

	/// \brief Returns the first frame of the animation interval that will be exported to the output file.
	TimePoint startFrame() const { return _startFrame; }

	/// \brief Sets the end of the animation interval that should be exported to the output file.
	void setEndFrame(int frame) { _endFrame = frame; }

	/// \brief Returns the last frame of the animation interval that will be exported to the output file.
	TimePoint endFrame() const { return _endFrame; }

	/// \brief Returns the interval between exported frames.
	int everyNthFrame() const { return _everyNthFrame; }

	/// \brief Sets the interval between exported frames.
	void setEveryNthFrame(int n) { _everyNthFrame = n; }

protected:

	class ProgressInterface {
	public:
		ProgressInterface(QProgressDialog* dialog) : _dialog(dialog), _baseValue(dialog ? dialog->value() : 0) {}
		void setPercentage(int progress) { if(_dialog) _dialog->setValue(_baseValue + progress); }
		bool wasCanceled() const { return _dialog ? _dialog->wasCanceled() : false; }
	protected:
		QProgressDialog* _dialog;
		int _baseValue;
	};

	/// \brief Retrieves the particles to be exported by evaluating the modification pipeline.
	/// \param nodes The selection of scene nodes to be exported.
	/// \param time The animation time at which to request the particles.
	/// \return The pipeline result containing the particles to be exported.
	///         The returned PipelineFlowState will be empty if the nodes list does not contain a particle object.
	PipelineFlowState getParticles(const QVector<SceneNode*>& nodes, TimePoint time);

	/// \brief This is called once for every output file to be written and before exportParticles() is called.
	virtual bool openOutputFile(const QString& filePath, int numberOfFrames);

	/// \brief This is called once for every output file written after exportParticles() has been called.
	virtual void closeOutputFile(bool exportCompleted);

	/// \brief Exports a single animation frame to the current output file.
	virtual bool exportFrame(const QVector<SceneNode*>& nodes, int frameNumber, TimePoint time, const QString& filePath, QProgressDialog* progressDialog);

	/// \brief Writes the particles of one animation frame to the current output file.
	/// \param state The pipeline results containing the particles to be exported.
	/// \param frameNumber The animation frame to be written to the output file.
	/// \param time The animation time to be written to the output file.
	/// \param filePath The path of the output file.
	/// \throws Exception on error.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	virtual bool exportParticles(const PipelineFlowState& state, int frameNumber, TimePoint time, const QString& filePath, ProgressInterface& progress) = 0;

	/// Returns the current file this exporter is writing to.
	QFile& outputFile() { return _outputFile; }

	/// Returns the text stream used to write into the current output file.
	CompressedTextWriter& textStream() { return *_outputStream; }

private:

	/// The output file path.
	PropertyField<QString> _outputFilename;

	/// Controls whether only the current animation frame or an entire animation interval should be exported.
	PropertyField<bool> _exportAnimation;

	/// Indicates that the exporter should produce a separate file for each timestep.
	PropertyField<bool> _useWildcardFilename;

	/// The wildcard name that is used to generate the output filenames.
	PropertyField<QString> _wildcardFilename;

	/// The first animation frame that should be exported.
	PropertyField<int> _startFrame;

	/// The last animation frame that should be exported.
	PropertyField<int> _endFrame;

	/// Controls the interval between exported frames.
	PropertyField<int> _everyNthFrame;

	/// The output file stream.
	QFile _outputFile;

	/// The stream object used to write into the output file.
	std::unique_ptr<CompressedTextWriter> _outputStream;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_outputFilename);
	DECLARE_PROPERTY_FIELD(_exportAnimation);
	DECLARE_PROPERTY_FIELD(_useWildcardFilename);
	DECLARE_PROPERTY_FIELD(_wildcardFilename);
	DECLARE_PROPERTY_FIELD(_startFrame);
	DECLARE_PROPERTY_FIELD(_endFrame);
	DECLARE_PROPERTY_FIELD(_everyNthFrame);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_EXPORTER_H
