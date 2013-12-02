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

/**
 * \file DataSet.h
 * \brief Contains definition of the Ovito::DataSet class.
 */

#ifndef __OVITO_DATASET_H
#define __OVITO_DATASET_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/scene/SceneRoot.h>

namespace Ovito {

class ViewportConfiguration;	// defined in ViewportConfiguration.h
class AnimationSettings;		// defined in AnimationSettings.h
class SelectionSet;				// defined in SelectionSet.h
class RenderSettings;			// defined in RenderSettings.h
class SceneRenderer;			// defined in SceneRenderer.h
class FrameBuffer;				// defined in FrameBuffer.h
class FrameBufferWindow;		// defined in FrameBufferWindow.h
class VideoEncoder;				// defined in VideoEncoder.h

/**
 * \brief This class stores everything that belongs to a scene.
 *
 * A DataSet is the document (or scene) edited by the user.
 * It can be saved to and loaded from a file.
 */
class OVITO_CORE_EXPORT DataSet : public RefTarget
{
public:

	/// \brief Constructs an empty dataset.
	Q_INVOKABLE DataSet();

	/// \brief Returns a reference to the viewport configuration associated with this dataset.
	/// \return The internal object that contains the configuration of the viewports. This object is saved
	///         to the scene file and is used to restore the original viewport configuration
	///         when the scene file is loaded.
	ViewportConfiguration* viewportConfig() const { return _viewportConfig; }

	/// \brief Returns the animation settings.
	/// \return The internal object that stores the animation settings for the scene.
	AnimationSettings* animationSettings() const { return _animSettings; }

	/// \brief Returns this dataset's root scene node.
	/// \return The root node of the scene tree.
	SceneRoot* sceneRoot() const { return _sceneRoot; }

	/// \brief Sets the dataset's root scene node.
	/// \param newScene The new scene tree. It will completely replace the old
	///                 scene object tree.
	/// \undoable
	void setSceneRoot(const OORef<SceneRoot>& newScene) { _sceneRoot = newScene; }

	/// \brief Returns the selection set.
	/// \return The internal selection set used to store the set of selected scene nodes.
	SelectionSet* selection() const { return _selection; }

	/// \brief Returns the general rendering settings for this scene.
	/// \return The internal object that stores the rendering settings.
	RenderSettings* renderSettings() const { return _renderSettings; }

	/// \brief Returns the path where this dataset is stored on disk.
	/// \return The location where the dataset is stored or will be stored on disk.
	const QString& filePath() const { return _filePath; }

	/// \brief Sets the path where this dataset is stored.
	/// \param path The new path (should be absolute) where the dataset will be stored.
	void setFilePath(const QString& path) { _filePath = path; }

	/// \brief Deletes all nodes from the scene.
	/// \undoable
	void clearScene();

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
	bool renderScene(RenderSettings* settings, Viewport* viewport, QSharedPointer<FrameBuffer> frameBuffer, FrameBufferWindow* frameBufferWindow = nullptr);

protected:

	/// Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Renders a single frame and saves the output file. This is part of the implementation of the renderScene() method.
	void renderFrame(TimePoint renderTime, int frameNumber, RenderSettings* settings, SceneRenderer* renderer,
			Viewport* viewport, FrameBuffer* frameBuffer, VideoEncoder* videoEncoder, QProgressDialog& progressDialog);

private:

	/// The configuration of the viewports.
	ReferenceField<ViewportConfiguration> _viewportConfig;

	/// Animation settings.
	ReferenceField<AnimationSettings> _animSettings;

	/// Scene node tree.
	ReferenceField<SceneRoot> _sceneRoot;

	/// The current node selection set.
	ReferenceField<SelectionSet> _selection;

	/// The settings used when rendering the scene.
	ReferenceField<RenderSettings> _renderSettings;

	/// The file where this DataSet is stored.
	QString _filePath;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_viewportConfig);
	DECLARE_REFERENCE_FIELD(_animSettings);
	DECLARE_REFERENCE_FIELD(_sceneRoot);
	DECLARE_REFERENCE_FIELD(_selection);
	DECLARE_REFERENCE_FIELD(_renderSettings);
};

};

#endif // __OVITO_DATASET_H
