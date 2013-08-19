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
#include <core/gui/actions/ActionManager.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/FrameBuffer.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/widgets/FrameBufferWindow.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/app/Application.h>
#include <core/dataset/DataSetManager.h>
#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
#include <video/VideoEncoder.h>
#endif

namespace Ovito {

/******************************************************************************
* Handles the ACTION_RENDER_ACTIVE_VIEWPORT command.
******************************************************************************/
void ActionManager::on_RenderActiveViewport_triggered()
{
	try {
		// Get the selected scene renderer.
		RenderSettings* settings = DataSetManager::instance().currentSet()->renderSettings();
		if(!settings || !settings->renderer()) throw Exception(tr("No renderer has been selected."));
		SceneRenderer* renderer = settings->renderer();

		// Do not update the viewports while rendering.
		ViewportSuspender noVPUpdates;

		// Get active viewport.
		Viewport* viewport = ViewportManager::instance().activeViewport();
		if(!viewport)
			throw Exception(tr("There is no active viewport to render."));

		// Show progress dialog.
		QProgressDialog progressDialog(&MainWindow::instance());
		progressDialog.setWindowModality(Qt::WindowModal);
		progressDialog.setAutoClose(false);
		progressDialog.setAutoReset(false);
		progressDialog.setMinimumDuration(0);

		// Allocate the frame buffer.
		QSharedPointer<FrameBuffer> frameBuffer(new FrameBuffer(settings->outputImageWidth(), settings->outputImageHeight()));

		try {

			// Initialize the renderer.
			if(renderer->startRender(DataSetManager::instance().currentSet(), settings)) {

				VideoEncoder* videoEncoder = nullptr;
#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
				QScopedPointer<VideoEncoder> videoEncoderPtr;
				// Initialize video encoder.
				if(settings->saveToFile() && settings->imageInfo().isMovie()) {

					if(settings->imageFilename().isEmpty())
						throw Exception(tr("Cannot save rendered images to movie file. Output filename has not been specified."));

					videoEncoderPtr.reset(new VideoEncoder());
					videoEncoder = videoEncoderPtr.data();
					videoEncoder->openFile(settings->imageFilename(), settings->outputImageWidth(), settings->outputImageHeight(), AnimManager::instance().framesPerSecond());
				}
#endif

				if(settings->renderingRangeType() == RenderSettings::CURRENT_FRAME) {
					// Render a single frame.
					TimePoint renderTime = AnimManager::instance().time();
					int frameNumber = AnimManager::instance().timeToFrame(renderTime);
					if(renderFrame(renderTime, frameNumber, settings, renderer, viewport, frameBuffer.data(), videoEncoder, progressDialog)) {
						// Open a display window for the rendered frame.
						if(Application::instance().guiMode()) {
							FrameBufferWindow* display = MainWindow::instance().frameBufferWindow();
							display->setFrameBuffer(frameBuffer);
							display->setWindowTitle(tr("Frame %1").arg(frameNumber));
							if(!display->isVisible())
								display->resize(display->sizeHint());
							display->show();
							display->activateWindow();
							display->updateFrame();
						}
					}
				}
				else if(settings->renderingRangeType() == RenderSettings::ANIMATION_INTERVAL || settings->renderingRangeType() == RenderSettings::CUSTOM_INTERVAL) {
					// Render an animation interval.
					TimePoint renderTime;
					int firstFrameNumber, numberOfFrames;
					if(settings->renderingRangeType() == RenderSettings::ANIMATION_INTERVAL) {
						renderTime = AnimManager::instance().animationInterval().start();
						firstFrameNumber = AnimManager::instance().timeToFrame(AnimManager::instance().animationInterval().start());
						numberOfFrames = (AnimManager::instance().timeToFrame(AnimManager::instance().animationInterval().end()) - firstFrameNumber + 1);
					}
					else {
						firstFrameNumber = settings->customRangeStart();
						renderTime = AnimManager::instance().frameToTime(firstFrameNumber);
						numberOfFrames = (settings->customRangeEnd() - firstFrameNumber + 1);
					}
					numberOfFrames = (numberOfFrames + settings->everyNthFrame()-1) / settings->everyNthFrame();
					if(numberOfFrames < 1)
						throw Exception(tr("Invalid rendering range: Frame %1 to %2").arg(settings->customRangeStart()).arg(settings->customRangeEnd()));
					progressDialog.setMaximum(numberOfFrames);

					// Render frames, one by one.
					for(int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++) {
						progressDialog.setValue(frameIndex);

						int frameNumber = firstFrameNumber + frameIndex * settings->everyNthFrame() + settings->fileNumberBase();
						if(renderFrame(renderTime, frameNumber, settings, renderer, viewport, frameBuffer.data(), videoEncoder, progressDialog)) {
							// Open a display window for the rendered frame.
							if(Application::instance().guiMode()) {
								FrameBufferWindow* display = MainWindow::instance().frameBufferWindow();
								display->setWindowTitle(tr("Frame %1").arg(AnimManager::instance().timeToFrame(renderTime)));
								if(frameIndex == 0) {
									display->setFrameBuffer(frameBuffer);
									display->show();
								}
								display->updateFrame();
							}
						}

						if(progressDialog.wasCanceled())
							break;

						// Goto next animation frame.
						renderTime += AnimManager::instance().ticksPerFrame() * settings->everyNthFrame();
					}
				}

#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
				// Finalize movie file.
				if(videoEncoder)
					videoEncoder->closeFile();
#endif
			}

			// Shutdown renderer.
			renderer->endRender();
		}
		catch(...) {
			// Shutdown renderer.
			renderer->endRender();
			throw;
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Renders a single frame and saves the output file.
******************************************************************************/
bool ActionManager::renderFrame(TimePoint renderTime, int frameNumber, RenderSettings* settings, SceneRenderer* renderer, Viewport* viewport,
		FrameBuffer* frameBuffer, VideoEncoder* videoEncoder, QProgressDialog& progressDialog)
{
	// Generate output filename.
	QString imageFilename;
	if(settings->saveToFile() && !videoEncoder) {
		imageFilename = settings->imageFilename();
		if(imageFilename.isEmpty())
			throw Exception(tr("Cannot save rendered image to file. Output filename has not been specified."));

		if(settings->renderingRangeType() != RenderSettings::CURRENT_FRAME) {
			// Append frame number to file name if rendering an animation.
			QFileInfo fileInfo(imageFilename);
			imageFilename = fileInfo.path() + QChar('/') + fileInfo.baseName() + QString("%1.").arg(frameNumber, 4, 10, QChar('0')) + fileInfo.completeSuffix();

			// Check for existing image file and skip.
			if(settings->skipExistingImages() && QFileInfo(imageFilename).isFile())
				return false;
		}
	}

	// Jump to animation time.
	AnimManager::instance().setTime(renderTime);

	// Wait until the scene is ready.
	bool sceneIsReady = false;
	DataSetManager::instance().runWhenSceneIsReady( [&sceneIsReady]() { sceneIsReady = true; } );
	if(!sceneIsReady) {
		progressDialog.setLabelText(tr("Rendering frame %1. Preparing scene...").arg(frameNumber));
		while(!sceneIsReady) {
			if(progressDialog.wasCanceled())
				return false;
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 200);
		}
	}
	progressDialog.setLabelText(tr("Rendering frame %1.").arg(frameNumber));

	// Request scene bounding box.
	Box3 boundingBox = renderer->sceneBoundingBox(renderTime);

	// Setup projection.
	ViewProjectionParameters projParams = viewport->projectionParameters(renderTime, settings->outputImageAspectRatio(), boundingBox);

	// Render one frame.
	frameBuffer->clear();
	renderer->beginFrame(renderTime, projParams, viewport);
	if(!renderer->renderFrame(frameBuffer, &progressDialog) || progressDialog.wasCanceled()) {
		progressDialog.cancel();
		renderer->endFrame();
		return false;
	}
	renderer->endFrame();

	// Save rendered image to disk.
	if(settings->saveToFile()) {
		if(!videoEncoder) {
			if(!frameBuffer->image().save(imageFilename, settings->imageInfo().format()))
				throw Exception(tr("Failed to save rendered image to image file '%1'.").arg(imageFilename));
		}
		else {
#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
			videoEncoder->writeFrame(frameBuffer->image());
#endif
		}
	}

	return !progressDialog.wasCanceled();
}

};
