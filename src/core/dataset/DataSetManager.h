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
 * \file DataSetManager.h 
 * \brief Contains the definition of the Ovito::DataSetManager class.
 */

#ifndef __OVITO_DATASET_MANAGER_H
#define __OVITO_DATASET_MANAGER_H

#include <core/Core.h>
#include <core/viewport/ViewportConfiguration.h>
#include "DataSet.h"
#include "CurrentSelectionProxy.h"

namespace Ovito {

/**
 * \brief Manages the current DataSet.
 * 
 * This is a singleton class with only one predefined instance of this class. 
 */
class DataSetManager : public RefMaker
{
public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the DataSetManager singleton class.
	inline static DataSetManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "DataSetManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Returns the current dataset being edited by the user.
	/// \return The active dataset.
	DataSet* currentSet() const { return _currentSet; }
	
	/// \brief Sets the current dataset being edited by the user.
	/// \param set The new dataset that should be shown in the main window.
	/// \note This operation cannot be undone. Make sure that the UndoManager is not recording operations.
	void setCurrentSet(const OORef<DataSet>& set);

	/// \brief Returns the current selection set. 
	/// \note The object returned by this method is a special proxy object (CurrentSelectionProxy) that mirrors the selection 
	///       set of the current DataSet. The proxy makes it possible to reference it during the whole session even when
	///       a new DataSet becomes active.
	SelectionSet* currentSelection() const { return _selectionSetProxy; }

	/// \brief Checks all scene nodes if their geometry pipeline is fully evaluated at the given animation time.
	bool isSceneReady(TimePoint time) const;

	/// \brief Calls the given slot as soon as the geometry pipelines of all scene nodes has been
	///        completely evaluated.
	void runWhenSceneIsReady(std::function<void ()> fn);

public Q_SLOTS:
	
	/// \brief Replaces the current data set with a new one and resets the 
	///        application to its initial state.
	void fileReset();

	/// \brief Save the current scene. 
	/// \return \c true, if the scene has been saved; \c false if the operation has been canceled by the user.
	/// 
	/// If the current scene has not been assigned a file path than this method
	/// shows a file dialog to let the user select a destination path for the scene file.
	/// \sa fileSaveAs()
	bool fileSave();

	/// \brief Lets the user select a new destination filename for the current scene and then saves the scene.
	/// \param filename If \a filename is an empty string that this method asks the user for a filename. Otherwise
	///                 The provided filename will be used.
	/// \return \c true, if the scene has been saved; \c false if the operation has been canceled by the user.
	/// \sa fileSave()
	bool fileSaveAs(const QString& filename = QString());

	/// \brief Asks the user if any changes made to the scene should be saved.
	/// 
	/// If the current dataset has been changed this method will ask the user if he wants
	/// to save the changes. If he answers yes then the scene is saved.
	///
	/// \return \c false if the operation has been canceled by the user; \c true on success.
	bool askForSaveChanges();

	/// \brief Loads the given scene file.
	/// \return \c true if the file has been successfully loaded; \c false if an error has occurred
	///         or the operation has been canceled by the user.
	bool fileLoad(const QString& filename);

	/// \brief Returns the default ViewportConfiguration used for new scene files.
	/// \return Pointer to the default viewport configuration stored by the DataSetManager.
	/// \note You may change the returned ViewportConfiguration object to modify the
	///       default configuration used for new scene files.
	/// \note Instead of using the returned object in a scene file you should make a copy of it first
	///       and leave the original object untouched.
	OORef<ViewportConfiguration> defaultViewportConfiguration();

Q_SIGNALS:

	/// Is emitted when a new dataset has become the active dataset.
	void dataSetReset(DataSet* newDataSet);
	
	/// \brief Is emitted when nodes have been added or removed from the current selection set.
	/// \param newSelection The current selection set.
	/// \note This signal is NOT emitted when a node in the selection set has changed.
	/// \note In contrast to the selectionChangeComplete() signal this signal is emitted
	///       for every node that is added to or removed from the selection set. That is,
	///       a call to SelectionSet::addAll() for example will generate multiple selectionChanged()
	///       events but only a single selectionChangeComplete() event.
	void selectionChanged(SelectionSet* newSelection);
	
	/// \brief This signal is emitted after all changes to the selection set have been completed.
	/// \param newSelection The current selection set.
	/// \note This signal is NOT emitted when a node in the selection set has changed.
	/// \note In contrast to the selectionChange() signal this signal is emitted
	///       only once after the selection set has been changed. That is,
	///       a call to SelectionSet::addAll() for example will generate multiple selectionChanged()
	///       events but only a single selectionChangeComplete() event.
	void selectionChangeComplete(SelectionSet* newSelection);

protected:

	/// Is called when a target referenced by this object generated an event.
	bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// Emits a selectionChanged signal.
	void emitSelectionChanged(SelectionSet* newSelection) { Q_EMIT selectionChanged(newSelection); }

	/// Emits a selectionChangeComplete signal.
	void emitSelectionChangeComplete(SelectionSet* newSelection) { Q_EMIT selectionChangeComplete(newSelection); }

	/// Checks if the scene is ready and calls all registered listeners.
	void notifySceneReadyListeners();

private:

	/// The current data set being edited by the user.
    ReferenceField<DataSet> _currentSet;

	/// This proxy object is used to make the current node selection always available.
	ReferenceField<CurrentSelectionProxy> _selectionSetProxy;
	
	/// This holds the default configuration of viewports to use.
	OORef<ViewportConfiguration> _defaultViewportConfig;

	/// List of listener objects that want to get notified when the scene is ready.
	QVector<std::function<void ()>> _sceneReadyListeners;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_currentSet);
	DECLARE_REFERENCE_FIELD(_selectionSetProxy);

	friend class CurrentSelectionProxy;

private:

	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	DataSetManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new DataSetManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static DataSetManager* _instance;

	friend class Application;
};

};

#endif // __OVITO_DATASET_MANAGER_H
