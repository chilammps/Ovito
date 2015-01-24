///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include <core/dataset/DataSet.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/animation/AnimationSettings.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/FrameBuffer.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include <core/gui/mainwin/MainWindow.h>
#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
	#include <core/utilities/io/video/VideoEncoder.h>
#endif

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, DataSet, RefTarget);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _viewportConfig, "ViewportConfiguration", ViewportConfiguration, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY|PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _animSettings, "AnimationSettings", AnimationSettings, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY|PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _sceneRoot, "SceneRoot", SceneRoot, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _selection, "CurrentSelection", SelectionSet, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY);
DEFINE_FLAGS_REFERENCE_FIELD(DataSet, _renderSettings, "RenderSettings", RenderSettings, PROPERTY_FIELD_NO_CHANGE_MESSAGE|PROPERTY_FIELD_ALWAYS_DEEP_COPY|PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(DataSet, _viewportConfig, "Viewport Configuration");
SET_PROPERTY_FIELD_LABEL(DataSet, _animSettings, "Animation Settings");
SET_PROPERTY_FIELD_LABEL(DataSet, _sceneRoot, "Scene");
SET_PROPERTY_FIELD_LABEL(DataSet, _selection, "Selection");
SET_PROPERTY_FIELD_LABEL(DataSet, _renderSettings, "Render Settings");

/******************************************************************************
* Constructor.
******************************************************************************/
DataSet::DataSet(DataSet* self) : RefTarget(this), _unitsManager(this)
{
	INIT_PROPERTY_FIELD(DataSet::_viewportConfig);
	INIT_PROPERTY_FIELD(DataSet::_animSettings);
	INIT_PROPERTY_FIELD(DataSet::_sceneRoot);
	INIT_PROPERTY_FIELD(DataSet::_selection);
	INIT_PROPERTY_FIELD(DataSet::_renderSettings);

	_viewportConfig = createDefaultViewportConfiguration();
	_animSettings = new AnimationSettings(this);
	_sceneRoot = new SceneRoot(this);
	_selection = new SelectionSet(this);
	_renderSettings = new RenderSettings(this);
}

/******************************************************************************
* Returns a viewport configuration that is used as template for new scenes.
******************************************************************************/
OORef<ViewportConfiguration> DataSet::createDefaultViewportConfiguration()
{
	UndoSuspender noUndo(undoStack());

	OORef<ViewportConfiguration> defaultViewportConfig = new ViewportConfiguration(this);

	OORef<Viewport> topView = new Viewport(this);
	topView->setViewType(Viewport::VIEW_TOP);
	defaultViewportConfig->addViewport(topView);

	OORef<Viewport> frontView = new Viewport(this);
	frontView->setViewType(Viewport::VIEW_FRONT);
	defaultViewportConfig->addViewport(frontView);

	OORef<Viewport> leftView = new Viewport(this);
	leftView->setViewType(Viewport::VIEW_LEFT);
	defaultViewportConfig->addViewport(leftView);

	OORef<Viewport> perspectiveView = new Viewport(this);
	perspectiveView->setViewType(Viewport::VIEW_PERSPECTIVE);
	perspectiveView->setCameraTransformation(ViewportSettings::getSettings().coordinateSystemOrientation() * AffineTransformation::lookAlong({90, -120, 100}, {-90, 120, -100}, {0,0,1}).inverse());
	defaultViewportConfig->addViewport(perspectiveView);

	defaultViewportConfig->setActiveViewport(perspectiveView);
	defaultViewportConfig->setMaximizedViewport(nullptr);

	return defaultViewportConfig;
}

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool DataSet::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSet::referenceEvent", "Reference events may only be processed in the GUI thread.");

	if(event->type() == ReferenceEvent::TargetChanged || event->type() == ReferenceEvent::PendingStateChanged) {

		// Update the viewports whenever something has changed in the current data set.
		if(source != viewportConfig() && source != animationSettings()) {
			// Do not automatically update while in the process of jumping to a new animation frame.
			if(!animationSettings()->isTimeChanging())
				viewportConfig()->updateViewports();

			if(source == sceneRoot() && event->type() == ReferenceEvent::PendingStateChanged) {
				notifySceneReadyListeners();
			}
		}
	}
	return RefTarget::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the value of a reference field of this RefMaker changes.
******************************************************************************/
void DataSet::referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget)
{
	if(field == PROPERTY_FIELD(DataSet::_viewportConfig))
		Q_EMIT viewportConfigReplaced(viewportConfig());
	else if(field == PROPERTY_FIELD(DataSet::_animSettings))
		Q_EMIT animationSettingsReplaced(animationSettings());
	else if(field == PROPERTY_FIELD(DataSet::_renderSettings))
		Q_EMIT renderSettingsReplaced(renderSettings());
	else if(field == PROPERTY_FIELD(DataSet::_selection))
		Q_EMIT selectionSetReplaced(selection());

	// Install a signal/slot connection that updates the viewports every time the animation time changes.
	if(field == PROPERTY_FIELD(DataSet::_viewportConfig) || field == PROPERTY_FIELD(DataSet::_animSettings)) {
		disconnect(_updateViewportOnTimeChangeConnection);
		if(animationSettings() && viewportConfig()) {
			_updateViewportOnTimeChangeConnection = connect(animationSettings(), &AnimationSettings::timeChangeComplete, viewportConfig(), &ViewportConfiguration::updateViewports);
			viewportConfig()->updateViewports();
		}
	}

	RefTarget::referenceReplaced(field, oldTarget, newTarget);
}

/******************************************************************************
* Returns the container to which this dataset belongs.
******************************************************************************/
DataSetContainer* DataSet::container() const
{
	for(RefMaker* refmaker : dependents()) {
		if(DataSetContainer* c = dynamic_object_cast<DataSetContainer>(refmaker)) {
			return c;
		}
	}
	OVITO_ASSERT_MSG(false, "DataSet::container()", "DataSet is not in a DataSetContainer.");
	return nullptr;
}

/******************************************************************************
* Returns a pointer to the main window in which this dataset is being edited.
******************************************************************************/
MainWindow* DataSet::mainWindow() const
{
	return container()->mainWindow();
}

/******************************************************************************
* Deletes all nodes from the scene.
******************************************************************************/
void DataSet::clearScene()
{
	while(!sceneRoot()->children().empty())
		sceneRoot()->children().back()->deleteNode();
}

/******************************************************************************
* Rescales the animation keys of all controllers in the scene.
******************************************************************************/
void DataSet::rescaleTime(const TimeInterval& oldAnimationInterval, const TimeInterval& newAnimationInterval)
{
	// Iterate over all controllers in the scene.
	for(RefTarget* reftarget : getAllDependencies()) {
		if(Controller* ctrl = dynamic_object_cast<Controller>(reftarget)) {
			ctrl->rescaleTime(oldAnimationInterval, newAnimationInterval);
		}
	}
}

/******************************************************************************
* Checks all scene nodes if their geometry pipeline is fully evaluated at the
* given animation time.
******************************************************************************/
bool DataSet::isSceneReady(TimePoint time) const
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSet::isSceneReady", "This function may only be called from the GUI thread.");
	OVITO_CHECK_OBJECT_POINTER(sceneRoot());

	// Iterate over all object nodes and request an evaluation of their geometry pipeline.
	bool isReady = sceneRoot()->visitObjectNodes([time](ObjectNode* node) {
		return (node->evalPipeline(time).status().type() != PipelineStatus::Pending);
	});

	return isReady;
}

/******************************************************************************
* Calls the given slot as soon as the geometry pipelines of all scene nodes has been
* completely evaluated.
******************************************************************************/
void DataSet::runWhenSceneIsReady(const std::function<void()>& fn)
{
	OVITO_ASSERT_MSG(QThread::currentThread() == QApplication::instance()->thread(), "DataSet::runWhenSceneIsReady", "This function may only be called from the GUI thread.");
	OVITO_CHECK_OBJECT_POINTER(sceneRoot());

	TimePoint time = animationSettings()->time();

	// Iterate over all object nodes and request an evaluation of their geometry pipeline.
	bool isReady = sceneRoot()->visitObjectNodes([time](ObjectNode* node) {
		return (node->evalPipeline(time).status().type() != PipelineStatus::Pending);
	});

	if(isReady)
		fn();
	else
		_sceneReadyListeners.push_back(fn);
}

/******************************************************************************
* Checks if the scene is ready and calls all registered listeners.
******************************************************************************/
void DataSet::notifySceneReadyListeners()
{
	if(!_sceneReadyListeners.empty() && isSceneReady(animationSettings()->time())) {
		auto oldListenerList = _sceneReadyListeners;
		_sceneReadyListeners.clear();
		for(const auto& listener : oldListenerList) {
			listener();
		}
	}
}

/******************************************************************************
* This is the high-level rendering function, which invokes the renderer to generate one or more
* output images of the scene. All rendering parameters are specified in the RenderSettings object.
******************************************************************************/
bool DataSet::renderScene(RenderSettings* settings, Viewport* viewport, QSharedPointer<FrameBuffer> frameBuffer, FrameBufferWindow* frameBufferWindow)
{
	OVITO_CHECK_OBJECT_POINTER(settings);
	OVITO_CHECK_OBJECT_POINTER(viewport);

	// If the caller did not supply a frame buffer, get the default frame buffer for the output image, or create a temporary one if necessary.
	if(!frameBuffer) {
		if(Application::instance().guiMode()) {
			OVITO_ASSERT(mainWindow());
			frameBufferWindow = mainWindow()->frameBufferWindow();
			frameBuffer = frameBufferWindow->frameBuffer();
		}
		if(!frameBuffer) {
			frameBuffer.reset(new FrameBuffer(settings->outputImageWidth(), settings->outputImageHeight()));
		}
	}

	// Get the selected scene renderer.
	SceneRenderer* renderer = settings->renderer();
	if(!renderer) throw Exception(tr("No renderer has been selected."));

	bool wasCanceled = false;
	try {

		// Set up the frame buffer for the output image.
		if(frameBufferWindow && frameBufferWindow->frameBuffer() != frameBuffer) {
			frameBufferWindow->setFrameBuffer(frameBuffer);
			frameBufferWindow->resize(frameBufferWindow->sizeHint());
		}
		if(frameBuffer->size() != QSize(settings->outputImageWidth(), settings->outputImageHeight())) {
			frameBuffer->setSize(QSize(settings->outputImageWidth(), settings->outputImageHeight()));
			frameBuffer->clear();
			if(frameBufferWindow)
				frameBufferWindow->resize(frameBufferWindow->sizeHint());
		}
		if(frameBufferWindow) {
			if(frameBufferWindow->isHidden()) {
				// Center frame buffer window in main window.
				if(frameBufferWindow->parentWidget()) {
					QSize s = frameBufferWindow->frameGeometry().size();
					frameBufferWindow->move(frameBufferWindow->parentWidget()->geometry().center() - QPoint(s.width() / 2, s.height() / 2));
				}
				frameBufferWindow->show();
			}
			frameBufferWindow->activateWindow();
		}

		// Show progress dialog.
		std::unique_ptr<QProgressDialog> progressDialog;
		if(Application::instance().guiMode()) {
			progressDialog.reset(new QProgressDialog(frameBufferWindow ? (QWidget*)frameBufferWindow : (QWidget*)mainWindow()));
			progressDialog->setWindowModality(Qt::WindowModal);
			progressDialog->setAutoClose(false);
			progressDialog->setAutoReset(false);
			progressDialog->setMinimumDuration(0);
			progressDialog->setValue(0);
		}

		// Initialize the renderer.
		if(renderer->startRender(this, settings)) {

			VideoEncoder* videoEncoder = nullptr;
#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
			QScopedPointer<VideoEncoder> videoEncoderPtr;
			// Initialize video encoder.
			if(settings->saveToFile() && settings->imageInfo().isMovie()) {

				if(settings->imageFilename().isEmpty())
					throw Exception(tr("Cannot save rendered images to movie file. Output filename has not been specified."));

				videoEncoderPtr.reset(new VideoEncoder());
				videoEncoder = videoEncoderPtr.data();
				videoEncoder->openFile(settings->imageFilename(), settings->outputImageWidth(), settings->outputImageHeight(), animationSettings()->framesPerSecond());
			}
#endif

			if(settings->renderingRangeType() == RenderSettings::CURRENT_FRAME) {
				// Render a single frame.
				TimePoint renderTime = animationSettings()->time();
				int frameNumber = animationSettings()->timeToFrame(renderTime);
				if(frameBufferWindow)
					frameBufferWindow->setWindowTitle(tr("Frame %1").arg(frameNumber));
				if(!renderFrame(renderTime, frameNumber, settings, renderer, viewport, frameBuffer.data(), videoEncoder, progressDialog.get()))
					wasCanceled = true;
			}
			else if(settings->renderingRangeType() == RenderSettings::ANIMATION_INTERVAL || settings->renderingRangeType() == RenderSettings::CUSTOM_INTERVAL) {
				// Render an animation interval.
				TimePoint renderTime;
				int firstFrameNumber, numberOfFrames;
				if(settings->renderingRangeType() == RenderSettings::ANIMATION_INTERVAL) {
					renderTime = animationSettings()->animationInterval().start();
					firstFrameNumber = animationSettings()->timeToFrame(animationSettings()->animationInterval().start());
					numberOfFrames = (animationSettings()->timeToFrame(animationSettings()->animationInterval().end()) - firstFrameNumber + 1);
				}
				else {
					firstFrameNumber = settings->customRangeStart();
					renderTime = animationSettings()->frameToTime(firstFrameNumber);
					numberOfFrames = (settings->customRangeEnd() - firstFrameNumber + 1);
				}
				numberOfFrames = (numberOfFrames + settings->everyNthFrame() - 1) / settings->everyNthFrame();
				if(numberOfFrames < 1)
					throw Exception(tr("Invalid rendering range: Frame %1 to %2").arg(settings->customRangeStart()).arg(settings->customRangeEnd()));
				if(progressDialog)
					progressDialog->setMaximum(numberOfFrames);

				// Render frames, one by one.
				for(int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++) {
					if(progressDialog)
						progressDialog->setValue(frameIndex);

					int frameNumber = firstFrameNumber + frameIndex * settings->everyNthFrame() + settings->fileNumberBase();
					if(frameBufferWindow)
						frameBufferWindow->setWindowTitle(tr("Frame %1").arg(animationSettings()->timeToFrame(renderTime)));
					if(!renderFrame(renderTime, frameNumber, settings, renderer, viewport, frameBuffer.data(), videoEncoder, progressDialog.get())) {
						wasCanceled = true;
						break;
					}
					if(progressDialog && progressDialog->wasCanceled())
						break;

					// Go to next animation frame.
					renderTime += animationSettings()->ticksPerFrame() * settings->everyNthFrame();
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

		if(progressDialog && progressDialog->wasCanceled())
			wasCanceled = true;
	}
	catch(...) {
		// Shutdown renderer.
		renderer->endRender();
		throw;
	}

	return !wasCanceled;
}

/******************************************************************************
* Renders a single frame and saves the output file.
******************************************************************************/
bool DataSet::renderFrame(TimePoint renderTime, int frameNumber, RenderSettings* settings, SceneRenderer* renderer, Viewport* viewport,
		FrameBuffer* frameBuffer, VideoEncoder* videoEncoder, QProgressDialog* progressDialog)
{
	// Determine output filename for this frame.
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
				return true;
		}
	}

	// Jump to animation frame.
	animationSettings()->setTime(renderTime);

	// Wait until the scene is ready.
	if(!waitUntilSceneIsReady(tr("Preparing frame %1").arg(frameNumber), progressDialog))
		return false;

	if(progressDialog)
		progressDialog->setLabelText(tr("Rendering frame %1").arg(frameNumber));

	// Request scene bounding box.
	Box3 boundingBox = renderer->sceneBoundingBox(renderTime);

	// Setup projection.
	ViewProjectionParameters projParams = viewport->projectionParameters(renderTime, settings->outputImageAspectRatio(), boundingBox);

	// Render one frame.
	frameBuffer->clear();
	renderer->beginFrame(renderTime, projParams, viewport);
	if(!renderer->renderFrame(frameBuffer, progressDialog) || (progressDialog && progressDialog->wasCanceled())) {
		renderer->endFrame();
		return false;
	}
	renderer->endFrame();

	// Apply viewport overlays.
	for(ViewportOverlay* overlay : viewport->overlays()) {
		{
			QPainter painter(&frameBuffer->image());
			overlay->render(viewport, painter, projParams, settings);
		}
		frameBuffer->update();
	}

	// Save rendered image to disk.
	if(settings->saveToFile()) {
		if(!videoEncoder) {
			OVITO_ASSERT(!imageFilename.isEmpty());
			if(!frameBuffer->image().save(imageFilename, settings->imageInfo().format()))
				throw Exception(tr("Failed to save rendered image to output file '%1'.").arg(imageFilename));
		}
		else {
#ifdef OVITO_VIDEO_OUTPUT_SUPPORT
			videoEncoder->writeFrame(frameBuffer->image());
#endif
		}
	}

	return true;
}

/******************************************************************************
* This function blocks until the scene has become ready.
******************************************************************************/
bool DataSet::waitUntilSceneIsReady(const QString& message, QProgressDialog* progressDialog)
{
	std::atomic_flag keepWaiting;
	keepWaiting.test_and_set();
	runWhenSceneIsReady( [&keepWaiting]() { keepWaiting.clear(); } );

	return container()->waitUntil([&keepWaiting]() {
		return !keepWaiting.test_and_set();
	}, message, progressDialog);
}

/******************************************************************************
* Saves the dataset to the given file.
******************************************************************************/
void DataSet::saveToFile(const QString& filePath)
{
	QFile fileStream(filePath);
    if(!fileStream.open(QIODevice::WriteOnly))
		throw Exception(tr("Failed to open output file '%1' for writing.").arg(filePath));

	QDataStream dataStream(&fileStream);
	ObjectSaveStream stream(dataStream);
	stream.saveObject(this);
	stream.close();

	if(fileStream.error() != QFile::NoError)
		throw Exception(tr("Failed to write output file '%1'.").arg(filePath));
	fileStream.close();
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
