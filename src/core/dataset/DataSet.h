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

#ifndef __OVITO_DATASET_H
#define __OVITO_DATASET_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/utilities/units/UnitsManager.h>
#include "UndoStack.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/**
 * \brief Stores the current state including the three-dimensional scene, viewport configuration,
 *        render settings etc.
 *
 * A DataSet represents the current document being edited by the user.
 * It can be completely saved to a file (.ovito extension) and loaded again at a later time.
 *
 * The DataSet class consists of various sub-objects that store different aspects. The
 * ViewportConfiguration object returned by viewportConfig(), for example, stores the list
 * of viewports.
 */
class OVITO_CORE_EXPORT DataSet : public RefTarget
{
public:

	/// \brief Constructs an empty dataset.
	/// \param self This parameter is not used and is there to provide a constructor signature that is compatible
	///             with the RefTarget base class.
	Q_INVOKABLE DataSet(DataSet* self = nullptr);

	/// \brief Returns a reference to the viewport configuration associated with this dataset.
	/// \return The internal object managing the viewports.
	ViewportConfiguration* viewportConfig() const { return _viewportConfig; }

	/// \brief Returns the animation settings.
	/// \return The internal object storing the animation settings of the scene.
	AnimationSettings* animationSettings() { return _animSettings; }

	/// \brief Returns this dataset's root scene node.
	/// \return The root node of the scene tree.
	SceneRoot* sceneRoot() const { return _sceneRoot; }

	/// \brief Returns the selection set.
	/// \return The current selection set storing the list of selected scene nodes.
	SelectionSet* selection() const { return _selection; }

	/// \brief Returns the rendering settings for this scene.
	/// \return The current rendering settings.
	RenderSettings* renderSettings() const { return _renderSettings; }

	/// \brief Returns the path where this dataset is stored on disk.
	/// \return The location where the dataset is stored or will be stored on disk.
	const QString& filePath() const { return _filePath; }

	/// \brief Sets the path where this dataset is stored.
	/// \param path The new path (should be absolute) where the dataset will be stored.
	void setFilePath(const QString& path) { _filePath = path; }

	/// \brief Returns the undo stack that keeps track of changes made to this dataset.
	UndoStack& undoStack() {
		OVITO_CHECK_OBJECT_POINTER(this);
		return _undoStack;
	}

	/// \brief Returns the manager of ParameterUnit objects.
	UnitsManager& unitsManager() { return _unitsManager; }

	/// \brief Returns a pointer to the main window in which this dataset is being edited.
	/// \return The main window, or NULL if this data set is not being edited in any window.
	MainWindow* mainWindow() const;

	/// \brief Returns the container this dataset belongs to.
	DataSetContainer* container() const;

	/// \brief Deletes all nodes from the scene.
	/// \undoable
	Q_INVOKABLE void clearScene();

	/// \brief Rescales the animation keys of all controllers in the scene.
	/// \param oldAnimationInterval The old animation interval, which will be mapped to the new animation interval.
	/// \param newAnimationInterval The new animation interval.
	///
	/// This method calls Controller::rescaleTime() for all controllers in the scene.
	/// For keyed controllers this will rescale the key times of all keys from the
	/// old animation interval to the new interval using a linear mapping.
	///
	/// Keys that lie outside of the old active animation interval will also be rescaled
	/// according to a linear extrapolation.
	///
	/// \undoable
	void rescaleTime(const TimeInterval& oldAnimationInterval, const TimeInterval& newAnimationInterval);

	/// \brief This is the high-level rendering function, which invokes the renderer to generate one or more
	///        output images of the scene. All rendering parameters are specified in the RenderSettings object.
	/// \param settings A RenderSettings object that specifies output image size, animation range to render etc.
	/// \param viewport The viewport to render. This determines the camera orientation.
	/// \param frameBuffer The frame buffer that will receive the rendered image. When rendering an animation
	///        sequence, the buffer will contain only the last rendered frame when the function returns.
	/// \param frameBufferWindow An optional pointer to a frame buffer window displaying the output frame buffer.
	///        The method will update the title and the size of the window while rendering the image.
	/// \return true on success; false if operation has been canceled by the user.
	/// \throw Exception on error.
	bool renderScene(RenderSettings* settings, Viewport* viewport, QSharedPointer<FrameBuffer> frameBuffer = QSharedPointer<FrameBuffer>(), FrameBufferWindow* frameBufferWindow = nullptr);

	/// \brief Checks all scene nodes if their geometry pipeline is fully evaluated at the given animation time.
	bool isSceneReady(TimePoint time) const;

	/// \brief Calls the given slot as soon as the geometry pipelines of all scene nodes has been
	///        completely evaluated.
	void runWhenSceneIsReady(const std::function<void()>& fn);

	/// \brief This function blocks until the scene has become ready.
	/// \param message The text to be shown to the user while waiting.
	/// \param progressDialog An existing progress dialog to use to show the message.
	///                       If NULL, the function will show its own dialog box.
	/// \return true on success; false if the operation has been canceled by the user.
	bool waitUntilSceneIsReady(const QString& message, QProgressDialog* progressDialog = nullptr);

	/// \brief Saves the dataset to the given file.
	/// \throw Exception on error.
	///
	/// Note that this method does NOT invoke setFilePath().
	void saveToFile(const QString& filePath);

Q_SIGNALS:

	/// \brief This signal is emitted whenever the current viewport configuration of this dataset
	///        has been replaced by a new one.
	/// \note This signal is NOT emitted when parameters of the current viewport configuration change.
    void viewportConfigReplaced(ViewportConfiguration* newViewportConfiguration);

	/// \brief This signal is emitted whenever the current animation settings of this dataset
	///        have been replaced by new ones.
	/// \note This signal is NOT emitted when parameters of the current animation settings object change.
    void animationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// \brief This signal is emitted whenever the current render settings of this dataset
	///        have been replaced by new ones.
	/// \note This signal is NOT emitted when parameters of the current render settings object change.
    void renderSettingsReplaced(RenderSettings* newRenderSettings);

	/// \brief This signal is emitted whenever the current selection set of this dataset
	///        has been replaced by another one.
	/// \note This signal is NOT emitted when nodes are added or removed from the current selection set.
    void selectionSetReplaced(SelectionSet* newSelectionSet);

protected:

	/// Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

private:

	/// Renders a single frame and saves the output file. This is part of the implementation of the renderScene() method.
	bool renderFrame(TimePoint renderTime, int frameNumber, RenderSettings* settings, SceneRenderer* renderer,
			Viewport* viewport, FrameBuffer* frameBuffer, VideoEncoder* videoEncoder, QProgressDialog* progressDialog);

	/// Returns a viewport configuration that is used as template for new scenes.
	OORef<ViewportConfiguration> createDefaultViewportConfiguration();

	/// Checks if the scene is ready and calls all registered listeners.
	void notifySceneReadyListeners();

private:

	/// The configuration of the viewports.
	ReferenceField<ViewportConfiguration> _viewportConfig;

	/// Current animation settings.
	ReferenceField<AnimationSettings> _animSettings;

	/// Root node of the scene node tree.
	ReferenceField<SceneRoot> _sceneRoot;

	/// The current node selection set.
	ReferenceField<SelectionSet> _selection;

	/// The settings used when rendering the scene.
	ReferenceField<RenderSettings> _renderSettings;

	/// The file path this DataSet has been saved to.
	QString _filePath;

	/// The undo stack that keeps track of changes made to this dataset.
	UndoStack _undoStack;

	/// The manager of ParameterUnit objects.
	UnitsManager _unitsManager;

	/// List of listener objects that want to get notified when the scene is ready.
	QVector<std::function<void()>> _sceneReadyListeners;

	/// This signal/slot connection updates the viewports when the animation time changes.
	QMetaObject::Connection _updateViewportOnTimeChangeConnection;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_viewportConfig);
	DECLARE_REFERENCE_FIELD(_animSettings);
	DECLARE_REFERENCE_FIELD(_sceneRoot);
	DECLARE_REFERENCE_FIELD(_selection);
	DECLARE_REFERENCE_FIELD(_renderSettings);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_DATASET_H
