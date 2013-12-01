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
 * \file DataSetContainer.h
 * \brief Contains the definition of the Ovito::DataSetContainer class.
 */

#ifndef __OVITO_DATASET_MANAGER_H
#define __OVITO_DATASET_MANAGER_H

#include <core/Core.h>
#include "DataSet.h"
#include "CurrentSelectionProxy.h"
#include "importexport/FileImporter.h"

namespace Ovito {

/**
 * \brief Manages the current DataSet.
 */
class OVITO_CORE_EXPORT DataSetContainer : public RefMaker
{
public:

	/// \brief Constructor.
	DataSetContainer();

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

	/// \brief Imports a given file into the scene.
	/// \param url The location of the file to import.
	/// \param importerType The type of importer to use. If NULL, the file format will be automatically detected.
	/// \return true if the file was successfully imported; false if operation has been canceled by the user.
	/// \throw Exception on error.
	bool importFile(const QUrl& url, const FileImporterDescription* importerType = nullptr, FileImporter::ImportMode importMode = FileImporter::AskUser);

	/// \brief Loads the given scene file.
	/// \return \c true if the file has been successfully loaded; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	bool fileLoad(const QString& filename);

	/// \brief Save the current scene. 
	/// \return \c true, if the scene has been saved; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	/// 
	/// If the current scene has not been assigned a file path, then this method
	/// displays a file selector dialog by calling fileSaveAs() to let the user select a file path.
	bool fileSave();

	/// \brief Lets the user select a new destination filename for the current scene. Then saves the scene by calling fileSave().
	/// \param filename If \a filename is an empty string that this method asks the user for a filename. Otherwise
	///                 the provided filename is used.
	/// \return \c true, if the scene has been saved; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	bool fileSaveAs(const QString& filename = QString());

	/// \brief Asks the user if changes made to the scene should be saved.
	/// \return \c false if the operation has been canceled by the user; \c true on success.
	/// \throw Exception on error.
	/// 
	/// If the current dataset has been changed, this method asks the user if changes should be saved.
	/// If yes, then the scene is saved by calling fileSave().
	bool askForSaveChanges();

Q_SIGNALS:

	/// Is emitted when a new dataset has become the active dataset.
	void dataSetChanged(DataSet* newDataSet);
	
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

private:

	/// Emits a selectionChanged signal.
	void emitSelectionChanged(SelectionSet* newSelection) { Q_EMIT selectionChanged(newSelection); }

	/// Emits a selectionChangeComplete signal.
	void emitSelectionChangeComplete(SelectionSet* newSelection) { Q_EMIT selectionChangeComplete(newSelection); }

private:

	/// The current data set being edited by the user.
    ReferenceField<DataSet> _currentSet;

	/// This proxy object is used to make the current node selection always available.
	ReferenceField<CurrentSelectionProxy> _selectionSetProxy;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_currentSet);
	DECLARE_REFERENCE_FIELD(_selectionSetProxy);

	friend class CurrentSelectionProxy;
};

};

#endif // __OVITO_DATASET_MANAGER_H
