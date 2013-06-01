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
#include "DataSet.h"
#if 0
#include "CurrentSelectionProxy.h"
#endif

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
		if(!_instance) _instance.reset(new DataSetManager());
		return *_instance.data();
	}

	/// \brief Returns the current dataset being edited by the user.
	/// \return The active dataset.
	DataSet* currentSet() const { return _currentSet; }
	
	/// \brief Sets the current dataset being edited by the user.
	/// \param set The new dataset that should be dhown in the main window.
	/// \note This operation cannot be undone. Make sure that the UndoManager is not recordings operations.
	void setCurrentSet(const OORef<DataSet>& set);

#if 0
	/// \brief Returns the current selection set. 
	/// \note The object returned by this method is a special proxy object (CurrentSelectionProxy) that mirrors the selection 
	///       set of the current DataSet. The proxy makes it possible to reference it during the whole session even when
	///       a new DataSet becomes active.
	/// \sa CurrentSelectionProxy, DataSet::selection()
	SelectionSet* currentSelection() const { return _selectionSetProxy; }
#endif

public Q_SLOTS:
	
	/// \brief Replaces the current data set with a new one and resets the 
	///        application to its initial state.
	void fileReset();

	/// \brief Save the current scene. 
	/// \return \c true, if the scene has been saved; \c false if the operation has been canceled by the user.
	/// 
	/// If the current scene has not been assigned a file path than thhis method
	/// shows a file dialog to let the user select a destination path for the scene file.
	/// \sa fileSaveAs()
	bool fileSave();

	/// \brief Lets the user select a enw destination filename for the current scene and then saves the scene. 
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

Q_SIGNALS:

	/// Is emitted when a new dataset has become the active dataset.
	void dataSetReset(DataSet* newDataSet);
	
#if 0
	/// \brief Is emitted when nodes have been added or removed from the current selection set.
	/// \param newSelection The current selection set.
	/// \note This signal is NOT emitted when a node in the selection set has changed.
	/// \note In contrast to the selectionChangeComplete() signal this signal is emitted
	///       for every node that is added to or removed from the selection set. That is,
	///       a call to SelectionSet::addAll() for example will generate multiple selectionChanged()
	///       events but only a single selectionChangeComplete() event.
	/// \sa selectionChangeComplete()
	void selectionChanged(SelectionSet* newSelection);
	
	/// \brief This signal is emitted after all changes to the selection set have been completed.
	/// \param newSelection The current selection set.
	/// \note This signal is NOT emitted when a node in the selection set has changed.
	/// \note In contrast to the selectionChange() signal this signal is emitted
	///       only once after the selection set has been changed. That is,
	///       a call to SelectionSet::addAll() for example will generate multiple selectionChanged()
	///       events but only a single selectionChangeComplete() event.
	/// \sa selectionChanged()
	void selectionChangeComplete(SelectionSet* newSelection);
#endif

private:

	/// The current data set being edited by the user.
    ReferenceField<DataSet> _currentSet;

#if 0
	/// This proxy object is used to make the current node selection always available.
	ReferenceField<CurrentSelectionProxy> _selectionSetProxy;
	
	/// Emits a selectionChanged signal.
	void emitSelectionChanged(SelectionSet* newSelection) { selectionChanged(newSelection); }

	/// Emits a selectionChangeComplete signal.
	void emitSelectionChangeComplete(SelectionSet* newSelection) { selectionChangeComplete(newSelection); }
#endif

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_currentSet)
#if 0
	DECLARE_REFERENCE_FIELD(_selectionSetProxy)
#endif

private:

	/////////////////////////// Maintainance ////////////////////////////////
    
	/// Private constructor.
	/// This is a singleton class; no public instances are allowed.
	DataSetManager();

	/// The singleton instance of this class.
	static QScopedPointer<DataSetManager> _instance;
};

};

#endif // __OVITO_DATASET_MANAGER_H
