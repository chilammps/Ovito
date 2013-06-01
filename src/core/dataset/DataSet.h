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

namespace Ovito {

class ViewportConfigurationSet;	// defined in ViewportConfiguration.h
class AnimationSettings;		// defined in AnimationSettings.h
class SelectionSet;				// defined in SelectionSet.h
class RenderSettings;			// defined in RenderSettings.h

/**
 * \brief This class stores everything that belongs to a scene.
 *
 * A DataSet is the document (or scene) edited by the user.
 * It can be saved to and loaded from a file.
 */
class DataSet : public RefTarget
{
public:

	/// \brief Constructs an empty dataset.
	Q_INVOKABLE DataSet();

	/// \brief Returns a reference to the viewport configuration associated with this dataset.
	/// \return The internal object that contains the configuration of the viewports. This object is saved
	///         to the scene file and is used to restore the original viewport configuration
	///         when the scene file is loaded.
	/// \note Changing the viewport configuration object does not have any effect because
	///       It is automatically regenerated just before the dataset is saved to a file.
	ViewportConfigurationSet* viewportConfig() const { return _viewportConfig; }

	/// \brief Returns the animation settings.
	/// \return The internal object that stores the animation settings for the scene.
	/// \sa AnimManager
	AnimationSettings* animationSettings() const { return _animSettings; }

#if 0
	/// \brief Returns this dataset's root scene node.
	/// \return The root node of the scene tree.
	/// \sa setSceneRoot()
	SceneRoot* sceneRoot() const { return _sceneRoot; }

	/// \brief Sets the dataset's root scene node.
	/// \param newScene The new scene tree. It will completely replace the old
	///                 scene object tree.
	/// \undoable
	/// \sa sceneRoot()
	void setSceneRoot(const SceneRoot::SmartPtr& newScene) { _sceneRoot = newScene; }

	/// \brief Returns the selection set.
	/// \return The internal selection set used to store the set of selected scene nodes.
	SelectionSet* selection() const { return _selection; }

	/// \brief Returns the general rendering settings for this scene.
	/// \return The internal object that stores the rendering settings.
	RenderSettings* renderSettings() const { return _renderSettings; }
#endif

	/// \brief Returns the state of the dirty flag that indicates whether the dataset has
	///        been changed and needs to be saved to disk.
	/// \return \c true if this dataset has been changed since the last save point.
	///
	/// The dirty flag will automatically be set when some subobject of the DataSet
	/// is being changed.
	///
	/// \sa setDirty()
	bool hasBeenChanged() const { return _hasBeenChanged; }

	/// \brief Marks the dataset as dirty or resets the dirty flag.
	/// \param dirty Speicifies whether the dirty flag should be set or reset.
	/// \sa hasBeenChanged()
	void setDirty(bool dirty = true) { _hasBeenChanged = dirty; }

	/// \brief Returns the path where this dataset is stored on disk.
	/// \return The location where the dataset is stored or will be stored on disk.
	/// \sa setFilePath()
	const QString& filePath() const { return _filePath; }

	/// \brief Sets the path where this dataset is stored.
	/// \param path The new path (should be absolute) where the dataset will be stored.
	/// \sa fielPath()
	void setFilePath(const QString& path) { _filePath = path; }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// The configuration of the viewports.
	ReferenceField<ViewportConfigurationSet> _viewportConfig;

	/// Animation settings.
	ReferenceField<AnimationSettings> _animSettings;

#if 0
	/// Scene node tree.
	ReferenceField<SceneRoot> _sceneRoot;

	/// The current node selection set.
	ReferenceField<SelectionSet> _selection;

	/// The settings used when rendering the scene.
	ReferenceField<RenderSettings> _renderSettings;
#endif

	/// The dirty flag.
	bool _hasBeenChanged;

	/// The file where this DataSet is stored.
	QString _filePath;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_viewportConfig)
	DECLARE_REFERENCE_FIELD(_animSettings)
#if 0
	DECLARE_REFERENCE_FIELD(_sceneRoot)
	DECLARE_REFERENCE_FIELD(_selection)
	DECLARE_REFERENCE_FIELD(_renderSettings)
#endif
};

};

#endif // __OVITO_DATASET_H
