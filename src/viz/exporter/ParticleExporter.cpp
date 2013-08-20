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
#include <core/animation/AnimManager.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/objects/SceneObject.h>
#include <core/dataset/DataSetManager.h>
#include <core/gui/mainwin/MainWindow.h>

#include <viz/data/ParticlePropertyObject.h>
#include "ParticleExporter.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticleExporter, FileExporter)
DEFINE_PROPERTY_FIELD(ParticleExporter, _outputFilename, "OutputFile")
DEFINE_PROPERTY_FIELD(ParticleExporter, _exportAnimation, "ExportAnimation")
DEFINE_PROPERTY_FIELD(ParticleExporter, _useWildcardFilename, "UseWildcardFilename")
DEFINE_PROPERTY_FIELD(ParticleExporter, _wildcardFilename, "WildcardFilename")
DEFINE_PROPERTY_FIELD(ParticleExporter, _startFrame, "StartFrame")
DEFINE_PROPERTY_FIELD(ParticleExporter, _endFrame, "EndFrame")
DEFINE_PROPERTY_FIELD(ParticleExporter, _everyNthFrame, "EveryNthFrame")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _outputFilename, "Output filename")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _exportAnimation, "Export animation")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _useWildcardFilename, "Use wildcard filename")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _wildcardFilename, "Wildcard filename")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _startFrame, "Start frame")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _endFrame, "End frame")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _everyNthFrame, "Every Nth frame")

/******************************************************************************
* Constructs a new instance of the class.
******************************************************************************/
ParticleExporter::ParticleExporter() : _exportAnimation(false),
	_useWildcardFilename(false), _startFrame(0), _endFrame(-1), _everyNthFrame(1)
{
	INIT_PROPERTY_FIELD(ParticleExporter::_outputFilename);
	INIT_PROPERTY_FIELD(ParticleExporter::_exportAnimation);
	INIT_PROPERTY_FIELD(ParticleExporter::_useWildcardFilename);
	INIT_PROPERTY_FIELD(ParticleExporter::_wildcardFilename);
	INIT_PROPERTY_FIELD(ParticleExporter::_startFrame);
	INIT_PROPERTY_FIELD(ParticleExporter::_endFrame);
	INIT_PROPERTY_FIELD(ParticleExporter::_everyNthFrame);
}

/******************************************************************************
* Sets the name of the output file that should be written by this exporter.
******************************************************************************/
void ParticleExporter::setOutputFile(const QString& filename)
{
	_outputFilename = filename;

	// Generate a default wildcard pattern from the filename.
	if(wildcardFilename().isEmpty()) {
		if(!filename.contains('*'))
			setWildcardFilename(QFileInfo(filename).fileName() + ".*");
		else
			setWildcardFilename(QFileInfo(filename).fileName());
	}
}

/******************************************************************************
* Exports the scene to the given file.
******************************************************************************/
bool ParticleExporter::exportToFile(const QString& filePath, DataSet* dataset)
{
	// Save the output path.
	setOutputFile(filePath);

	// Get the data to be exported.
	PipelineFlowState flowState = getParticles(dataset, dataset->animationSettings()->time());
	if(flowState.isEmpty())
		throw Exception(tr("The scene does not contain any particles that can be exported."));

	// Use the current time as default export interval if no interval has been set before.
	if(startFrame() > endFrame()) {
		setStartFrame(0);
		int lastFrame = dataset->animationSettings()->animationInterval().end() / dataset->animationSettings()->ticksPerFrame();
		setEndFrame(lastFrame);
	}

	// Show optional export settings dialog.
	if(!showSettingsDialog(dataset, flowState, nullptr))
		return false;

	// Perform the actual export operation.
	return writeOutputFiles(dataset);
}

/******************************************************************************
* Retrieves the particles to be exported by evaluating the modification pipeline.
******************************************************************************/
PipelineFlowState ParticleExporter::getParticles(DataSet* dataset, TimePoint time)
{
	// Iterate over all scene nodes.
	for(SceneNodesIterator iter(dataset->sceneRoot()); !iter.finished(); iter.next()) {

		ObjectNode* node = dynamic_object_cast<ObjectNode>(iter.current());
		if(!node) continue;

		// Check if the node's pipeline evaluates to something that contains particles.
		const PipelineFlowState& state = node->evalPipeline(time);
		for(const auto& o : state.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
			if(property && property->type() == ParticleProperty::PositionProperty) {
				return state;
			}
		}
	}

	// Nothing to export.
	return PipelineFlowState();
}

/******************************************************************************
 * Exports the particles contained in the given scene to the output file(s).
 *****************************************************************************/
bool ParticleExporter::writeOutputFiles(DataSet* dataset)
{
	OVITO_ASSERT_MSG(!outputFile().isEmpty(), "ParticleExporter::exportParticles()", "Output filename has not been set. ParticleExporter::setOutputFile() must be called first.");
	OVITO_ASSERT_MSG(startFrame() <= endFrame(), "ParticleExporter::exportParticles()", "Export interval has not been set. ParticleExporter::setStartFrame() and ParticleExporter::setEndFrame() must be called first.");

	if(startFrame() > endFrame())
		throw Exception(tr("The animation interval to be exported is empty or has not been set."));

	// Show progress dialog.
	QProgressDialog progressDialog(&MainWindow::instance());
	progressDialog.setWindowModality(Qt::WindowModal);
	progressDialog.setAutoClose(false);
	progressDialog.setAutoReset(false);
	progressDialog.setMinimumDuration(0);

	// Compute the number of frames that need to be exported.
	TimePoint exportTime;
	int firstFrameNumber, numberOfFrames;
	if(_exportAnimation) {
		firstFrameNumber = startFrame();
		exportTime = firstFrameNumber * dataset->animationSettings()->ticksPerFrame();
		numberOfFrames = (endFrame() - startFrame() + everyNthFrame()) / everyNthFrame();
		if(numberOfFrames < 1 || everyNthFrame() < 1)
			throw Exception(tr("Invalid export animation range: Frame %1 to %2").arg(startFrame()).arg(endFrame()));
	}
	else {
		firstFrameNumber = dataset->animationSettings()->time() * dataset->animationSettings()->ticksPerFrame();
		exportTime = dataset->animationSettings()->time();
		numberOfFrames = 1;
	}

	// Validate export settings.
	if(_exportAnimation && useWildcardFilename()) {
		if(wildcardFilename().isEmpty())
			throw Exception(tr("Cannot write animation frame to separate files. No wildcard pattern has been specified."));
		if(wildcardFilename().contains(QChar('*')) == false)
			throw Exception(tr("Cannot write animation frames to separate files. The filename must contain the '*' wildcard character, which gets replaced by the frame number."));
	}

	progressDialog.setMaximum(numberOfFrames);
	QDir dir = QFileInfo(outputFile()).dir();
	QString filename = outputFile();

	// Open output file for writing.
	if(!_exportAnimation || !useWildcardFilename()) {
		if(!openOutputFile(filename, numberOfFrames))
			return false;
	}

	try {

		// Export animation frames.
		for(int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++) {
			progressDialog.setValue(frameIndex);

			int frameNumber = firstFrameNumber + frameIndex * everyNthFrame();

			if(_exportAnimation && useWildcardFilename()) {
				// Generate an output filename based on the wildcard pattern.
				filename = dir.absoluteFilePath(wildcardFilename());
				filename.replace(QChar('*'), QString::number(frameNumber));
				if(!openOutputFile(filename, 1))
					return false;
			}

			if(!exportParticles(dataset, frameNumber, exportTime, filename))
				progressDialog.cancel();

			if(_exportAnimation && useWildcardFilename())
				closeOutputFile(true);

			if(progressDialog.wasCanceled())
				break;

			// Go to next animation frame.
			exportTime += dataset->animationSettings()->ticksPerFrame() * everyNthFrame();
		}
	}
	catch(...) {
		closeOutputFile(false);
		throw;
	}

	// Close output file.
	if(!_exportAnimation || !useWildcardFilename()) {
		closeOutputFile(true);
	}

	return true;
}

};
