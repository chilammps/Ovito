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
#include <core/gui/actions/ActionManager.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/utilities/io/FileManager.h>
#include "LinkedFileImporter.h"
#include "LinkedFileObject.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinkedFileImporter, FileImporter)

/******************************************************************************
* Sends a request to the LinkedFileObject owning this importer to reload
* the input file.
******************************************************************************/
void LinkedFileImporter::requestReload(int frame)
{
	// Retrieve the LinkedFileObject that owns this importer by looking it up in the list of dependents.
	for(RefMaker* refmaker : dependents()) {
		LinkedFileObject* obj = dynamic_object_cast<LinkedFileObject>(refmaker);
		if(obj) {
			try {
				obj->refreshFromSource(frame);
			}
			catch(const Exception& ex) {
				ex.showError();
			}
		}
	}
}

/******************************************************************************
* Sends a request to the LinkedFileObject owning this importer to refresh the
* animation frame sequence.
******************************************************************************/
void LinkedFileImporter::requestFramesUpdate()
{
	// Retrieve the LinkedFileObject that owns this importer by looking it up in the list of dependents.
	for(RefMaker* refmaker : dependents()) {
		LinkedFileObject* obj = dynamic_object_cast<LinkedFileObject>(refmaker);
		if(obj) {
			try {
				// Scan input source for animation frames.
				obj->updateFrames();
			}
			catch(const Exception& ex) {
				ex.showError();
			}

			// Adjust the animation length number to match the number of frames in the input data source.
			obj->adjustAnimationInterval();
		}
	}
}

/******************************************************************************
* Imports the given file into the scene.
* Return true if the file has been imported.
* Return false if the import has been aborted by the user.
* Throws an exception when the import has failed.
******************************************************************************/
bool LinkedFileImporter::importFile(const QUrl& sourceUrl, DataSet* dataset)
{
	OORef<LinkedFileObject> existingObj;
	ObjectNode* existingNode = nullptr;

	if(dataset->sceneRoot()->children().empty() == false) {

		// Look for an existing LinkedFileObject in the scene whose
		// input file we can replace with the newly imported file.
		dataset->sceneRoot()->visitObjectNodes([&existingObj, &existingNode] (ObjectNode* node) -> bool {
			SceneObject* sceneObj = node->sceneObject();
			while(sceneObj) {
				LinkedFileObject* linkedFileObj = dynamic_object_cast<LinkedFileObject>(sceneObj);
				if(linkedFileObj) {
					existingObj = linkedFileObj;
					existingNode = node;
					return false;
				}
				sceneObj = (sceneObj->inputObjectCount() > 0) ? sceneObj->inputObject(0) : nullptr;
			}
			return true;
		});

		if(existingObj) {

			// Ask user if the current import node including any applied modifiers should be kept.
			QMessageBox::StandardButton result = QMessageBox::question(&MainWindow::instance(), tr("Import file"),
				tr("Do you want to keep the current modifiers and settings when importing the new file?"),
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);

			if(result == QMessageBox::Cancel)
				return false; // Operation canceled by user.

			if(result == QMessageBox::No) {

				existingObj = nullptr;
				existingNode = nullptr;

				// Clear current scene.
				dataset->clearScene();
				dataset->setFilePath(QString());
			}
		}
		else {

			// Ask user if the current scene should be completely replaced by the imported data.
			QMessageBox::StandardButton result = QMessageBox::question(&MainWindow::instance(), tr("Import file"),
				tr("Do you want to completely replace the current scene with the imported data?"),
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Cancel);

			if(result == QMessageBox::Cancel)
				return false; // Operation canceled by user.

			if(result == QMessageBox::Yes) {

				// Ask user if current scene should be saved before it is replaced by the imported data.
				if(!DataSetManager::instance().askForSaveChanges())
					return false;

				// Clear current scene.
				dataset->clearScene();
				dataset->setFilePath(QString());
			}
		}
	}

	// Do not create any animation keys during import.
	AnimationSuspender animSuspender;

	OORef<LinkedFileObject> obj;

	// Create the object that will insert the imported data into the scene.
	if(existingObj == nullptr)
		obj = new LinkedFileObject();
	else
		obj = existingObj;

	// Set the input location and importer.
	if(!obj->setSource(sourceUrl, this)) {
		return false;
	}

	// Create a new object node in the scene for the linked data.
	SceneRoot* scene = dataset->sceneRoot();
	OORef<ObjectNode> node;
	if(existingNode == nullptr) {
		{
			UndoSuspender unsoSuspender;	// Do not create undo records for this part.

			// Add object to scene.
			node = new ObjectNode();
			node->setSceneObject(obj);
		}

		// Insert node into scene.
		scene->addChild(node);
	}
	else node = existingNode;

	// Select import node.
	dataset->selection()->clear();
	dataset->selection()->add(node);

	// Jump to the right frame to show the originally selected file.
	int jumpToFrame = -1;
	for(int frameIndex = 0; frameIndex < obj->frames().size(); frameIndex++) {
		if(obj->frames()[frameIndex].sourceFile == sourceUrl) {
			jumpToFrame = frameIndex;
			break;
		}
	}

	// Adjust the animation length number to match the number of frames in the input data source.
	obj->adjustAnimationInterval(jumpToFrame);

	// Adjust views to show the newly imported object.
	if(dataset == DataSetManager::instance().currentSet()) {
		DataSetManager::instance().runWhenSceneIsReady([]() {
			ActionManager::instance().getAction(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL)->trigger();
		});
	}

	return true;
}

/******************************************************************************
* Scans the input source (which can be a directory or a single file) to
* discover all animation frames.
*
* This implementation of this method checks if the source URL contains a wild-card pattern.
* If yes, it scans the directory to find all matching files.
******************************************************************************/
Future<QVector<LinkedFileImporter::FrameSourceInformation>> LinkedFileImporter::findFrames(const QUrl& sourceUrl)
{
	QVector<FrameSourceInformation> frames;

	// Determine whether the filename contains wildcard characters.
	QFileInfo fileInfo(sourceUrl.path());
	QString pattern = fileInfo.fileName();
	if(pattern.contains('*') == false && pattern.contains('?') == false) {

		// It's not a wildcard pattern. Register just a single frame.
		frames.push_back({ sourceUrl, 0, 0, fileInfo.lastModified(), fileInfo.fileName() });

	}
	else {

		QDir directory;
		bool isLocalPath = false;

		// Scan the directory for files matching the wildcard pattern.
		QStringList entries;
		if(sourceUrl.isLocalFile()) {

			isLocalPath = true;
			directory = QFileInfo(sourceUrl.toLocalFile()).dir();
			for(const QString& filename : directory.entryList(QDir::Files|QDir::NoDotAndDotDot, QDir::Name)) {
				if(matchesWildcardPattern(pattern, filename))
					entries << filename;
			}

		}
		else {

			directory = fileInfo.dir();
			QUrl directoryUrl = sourceUrl;
			directoryUrl.setPath(fileInfo.path());

			// Retrieve list of files in remote directory.
			Future<QStringList> fileListFuture = FileManager::instance().listDirectoryContents(directoryUrl);
			if(!ProgressManager::instance().waitForTask(fileListFuture))
				return Future<QVector<FrameSourceInformation>>::createCanceled();

			// Filter file names.
			for(const QString& filename : fileListFuture.result()) {
				if(matchesWildcardPattern(pattern, filename))
					entries << filename;
			}
		}

		// Sorted the files.
		// A file called "abc9.xyz" must come before a file named "abc10.xyz", which is not
		// the default lexicographic ordering.
		QMap<QString, QString> sortedFilenames;
		Q_FOREACH(QString oldName, entries) {
			// Generate a new name from the original filename that yields the correct ordering.
			QString newName;
			QString number;
			for(int index = 0; index < oldName.length(); index++) {
				QChar c = oldName[index];
				if(!c.isDigit()) {
					if(!number.isEmpty()) {
						newName.append(number.rightJustified(10, '0'));
						number.clear();
					}
					newName.append(c);
				}
				else number.append(c);
			}
			if(!number.isEmpty()) newName.append(number.rightJustified(10, '0'));
			sortedFilenames[newName] = oldName;
		}

		// Generate final list of frames.
		for(const auto& iter : sortedFilenames) {
			QFileInfo fileInfo(directory, iter);
			QUrl url = sourceUrl;
			if(isLocalPath)
				url = QUrl::fromLocalFile(fileInfo.filePath());
			else
				url.setPath(fileInfo.filePath());
			frames.push_back({
				url, 0, 0,
				isLocalPath ? fileInfo.lastModified() : QDateTime(),
				iter });
		}
	}

	return Future<QVector<FrameSourceInformation>>::createImmediate(frames);
}

/******************************************************************************
* Checks if a filename matches to the given wildcard pattern.
******************************************************************************/
bool LinkedFileImporter::matchesWildcardPattern(const QString& pattern, const QString& filename)
{
#if 0
	QRegExp patternExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
	return patternExp.exactMatch(filename);
#else
	QString::const_iterator p = pattern.constBegin();
	QString::const_iterator f = filename.constBegin();
	while(p != pattern.constEnd() && f != filename.constEnd()) {
		if(*p == QChar('*')) {
			if(!f->isDigit())
				return false;
			do { ++f; }
			while(f != filename.constEnd() && f->isDigit());
			++p;
			continue;
		}
		else if(*p != *f)
			return false;
		++p;
		++f;
	}
	return p == pattern.constEnd() && f == filename.constEnd();
#endif
}


/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
Future<LinkedFileImporter::ImportTaskPtr> LinkedFileImporter::load(const LinkedFileImporter::FrameSourceInformation& frame)
{
	ImportTaskPtr importTask = createImportTask(frame);

	return runInBackground<ImportTaskPtr>(
			[importTask] (FutureInterface<LinkedFileImporter::ImportTaskPtr>& futureInterface) {

		// Run the task
		importTask->load(futureInterface);

		// Return the importer task object as the result.
		if(!futureInterface.isCanceled())
			futureInterface.setResult(importTask);

		});
}

};
