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

#ifndef __OVITO_DATASET_CONTAINER_H
#define __OVITO_DATASET_CONTAINER_H

#include <core/Core.h>
#include <core/utilities/concurrent/TaskManager.h>
#include "DataSet.h"
#include "importexport/FileImporter.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

/**
 * \brief Manages the DataSet being edited.
 */
class OVITO_CORE_EXPORT DataSetContainer : public RefMaker
{
public:

	/// \brief Constructor.
	DataSetContainer(MainWindow* mainWindow = nullptr);

	/// \brief Destructor.
	virtual ~DataSetContainer() {
		setCurrentSet(nullptr);
		clearAllReferences();
	}

	/// \brief Returns the current dataset being edited by the user.
	/// \return The active dataset.
	DataSet* currentSet() const { return _currentSet; }
	
	/// \brief Sets the current dataset being edited by the user.
	/// \param set The dataset that should be shown in the main window.
	void setCurrentSet(DataSet* set) { _currentSet = set; }

	/// Returns the window this dataset container is linked to (may be NULL).
	MainWindow* mainWindow() const { return _mainWindow; }

	/// \brief Returns the manager of background tasks.
	/// \return Reference to the task manager, which is part of this dataset manager.
	///
	/// Use the task manager to start and control background jobs.
	TaskManager& taskManager() { return _taskManager; }

	/// \brief Imports a given file into the current dataset.
	/// \param url The location of the file to import.
	/// \param importerType The FileImporter type to use. If NULL, the file format will be automatically detected.
	/// \return true if the file was successfully imported; false if operation has been canceled by the user.
	/// \throw Exception on error.
	bool importFile(const QUrl& url, const OvitoObjectType* importerType = nullptr, FileImporter::ImportMode importMode = FileImporter::AskUser);

	/// \brief Creates an empty dataset and makes it the current dataset.
	/// \return \c true if the operation was completed; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	bool fileNew();

	/// \brief Loads the given file and makes it the current dataset.
	/// \return \c true if the file has been successfully loaded; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	bool fileLoad(const QString& filename);

	/// \brief Save the current dataset.
	/// \return \c true, if the dataset has been saved; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	/// 
	/// If the current dataset has not been assigned a file path, then this method
	/// displays a file selector dialog by calling fileSaveAs() to let the user select a file path.
	bool fileSave();

	/// \brief Lets the user select a new destination filename for the current dataset. Then saves the dataset by calling fileSave().
	/// \param filename If \a filename is an empty string that this method asks the user for a filename. Otherwise
	///                 the provided filename is used.
	/// \return \c true, if the dataset has been saved; \c false if the operation has been canceled by the user.
	/// \throw Exception on error.
	bool fileSaveAs(const QString& filename = QString());

	/// \brief Asks the user if changes made to the dataset should be saved.
	/// \return \c false if the operation has been canceled by the user; \c true on success.
	/// \throw Exception on error.
	/// 
	/// If the current dataset has been changed, this method asks the user if changes should be saved.
	/// If yes, then the dataset is saved by calling fileSave().
	bool askForSaveChanges();

	/// \brief This function blocks execution until some operation has been completed.
	///        The function displays a progress dialog to block access to the application main window.
	///        The dialog allows the user to cancel the operation.
	/// \param callback This callback function will be polled to check whether the operation has finished.
	///                 The callback function should return true to indicate that the operation has finished.
	/// \param message The text to be shown to the user while waiting.
	/// \param progressDialog An existing progress dialog to use to show the message.
	///                       If NULL, the function will show its own dialog box.
	/// \return true on success; false if the operation has been canceled by the user.
	bool waitUntil(const std::function<bool()>& callback, const QString& message, QProgressDialog* progressDialog = nullptr);

Q_SIGNALS:

	/// Is emitted when a another dataset has become the active dataset.
	void dataSetChanged(DataSet* newDataSet);
	
	/// \brief Is emitted when nodes have been added or removed from the current selection set.
	/// \param selection The current selection set.
	/// \note This signal is NOT emitted when a node in the selection set has changed.
	/// \note In contrast to the selectionChangeComplete() signal this signal is emitted
	///       for every node that is added to or removed from the selection set. That is,
	///       a call to SelectionSet::addAll() for example will generate multiple selectionChanged()
	///       events but only a single selectionChangeComplete() event.
    void selectionChanged(SelectionSet* selection);
	
	/// \brief This signal is emitted after all changes to the selection set have been completed.
	/// \param selection The current selection set.
	/// \note This signal is NOT emitted when a node in the selection set has changed.
	/// \note In contrast to the selectionChange() signal this signal is emitted
	///       only once after the selection set has been changed. That is,
	///       a call to SelectionSet::addAll() for example will generate multiple selectionChanged()
	///       events but only a single selectionChangeComplete() event.
    void selectionChangeComplete(SelectionSet* selection);

	/// \brief This signal is emitted whenever the current selection set has been replaced by another one.
	/// \note This signal is NOT emitted when nodes are added or removed from the current selection set.
    void selectionSetReplaced(SelectionSet* newSelectionSet);

	/// \brief This signal is emitted whenever the current viewport configuration of current dataset has been replaced by a new one.
	/// \note This signal is NOT emitted when the parameters of the current viewport configuration change.
    void viewportConfigReplaced(ViewportConfiguration* newViewportConfiguration);

	/// \brief This signal is emitted whenever the current animation settings of the current dataset have been replaced by new ones.
	/// \note This signal is NOT emitted when the parameters of the current animation settings object change.
    void animationSettingsReplaced(AnimationSettings* newAnimationSettings);

	/// \brief This signal is emitted whenever the current render settings of this dataset
	///        have been replaced by new ones.
	/// \note This signal is NOT emitted when parameters of the current render settings object change.
    void renderSettingsReplaced(RenderSettings* newRenderSettings);

	/// \brief This signal is emitted when the current animation time has changed or if the current animation settings have been replaced.
    void timeChanged(TimePoint newTime);

	/// \brief This signal is emitted when the scene becomes ready after the current animation time has changed.
	void timeChangeComplete();

protected:

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

protected Q_SLOTS:

	/// This handler is invoked when the current selection set of the current dataset has been replaced.
    void onSelectionSetReplaced(SelectionSet* newSelectionSet);

	/// This handler is invoked when the current animation settings of the current dataset have been replaced.
    void onAnimationSettingsReplaced(AnimationSettings* newAnimationSettings);

private:

	/// The window this dataset container is linked to (may be NULL).
	MainWindow* _mainWindow;

	/// The current dataset being edited by the user.
    ReferenceField<DataSet> _currentSet;

	/// The container for background tasks.
	TaskManager _taskManager;

	QMetaObject::Connection _selectionSetReplacedConnection;
	QMetaObject::Connection _selectionSetChangedConnection;
	QMetaObject::Connection _selectionSetChangeCompleteConnection;
	QMetaObject::Connection _viewportConfigReplacedConnection;
	QMetaObject::Connection _animationSettingsReplacedConnection;
	QMetaObject::Connection _renderSettingsReplacedConnection;
	QMetaObject::Connection _animationTimeChangedConnection;
	QMetaObject::Connection _animationTimeChangeCompleteConnection;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_currentSet);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_DATASET_CONTAINER_H
