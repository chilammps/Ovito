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
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/objects/DataObject.h>
#include <core/scene/SelectionSet.h>
#include <core/animation/AnimationSettings.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/utilities/io/FileManager.h>
#include "FileSourceImporter.h"
#include "FileSource.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FileSourceImporter, FileImporter);

/******************************************************************************
* Sends a request to the FileSource owning this importer to reload
* the input file.
******************************************************************************/
void FileSourceImporter::requestReload(int frame)
{
	// Retrieve the FileSource that owns this importer by looking it up in the list of dependents.
	for(RefMaker* refmaker : dependents()) {
		FileSource* obj = dynamic_object_cast<FileSource>(refmaker);
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
* Sends a request to the FileSource owning this importer to refresh the
* animation frame sequence.
******************************************************************************/
void FileSourceImporter::requestFramesUpdate()
{
	// Retrieve the FileSource that owns this importer by looking it up in the list of dependents.
	for(RefMaker* refmaker : dependents()) {
		FileSource* obj = dynamic_object_cast<FileSource>(refmaker);
		if(obj) {
			try {
				// If wildcard pattern seach has been disabled, replace
				// wildcard pattern URL with an actual filename first.
				if(!autoGenerateWildcardPattern()) {
					QFileInfo fileInfo(obj->sourceUrl().path());
					if(fileInfo.fileName().contains('*') || fileInfo.fileName().contains('?')) {
						if(obj->loadedFrameIndex() >= 0 && obj->loadedFrameIndex() < obj->frames().size()) {
							QUrl currentUrl = obj->frames()[obj->loadedFrameIndex()].sourceFile;
							if(currentUrl != obj->sourceUrl()) {
								obj->setSource(currentUrl, this);
								continue;
							}
						}
					}
				}

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
bool FileSourceImporter::importFile(const QUrl& sourceUrl, ImportMode importMode)
{
	OORef<FileSource> existingFileSource;
	ObjectNode* existingNode = nullptr;

	if(dataset()->sceneRoot()->children().empty() == false) {

		if(importMode != AddToScene) {
			// Look for an existing FileSource in the scene whose
			// data source we can replace with the newly imported file.
			for(SceneNode* node : dataset()->selection()->nodes()) {
				if(ObjectNode* objNode = dynamic_object_cast<ObjectNode>(node)) {
					existingFileSource = dynamic_object_cast<FileSource>(objNode->sourceObject());
					if(existingFileSource) {
						existingNode = objNode;
						break;
					}
				}
			}
		}

		if(existingFileSource) {
			if(importMode == AskUser) {
				// Ask user if the current import node including any applied modifiers should be kept.
				QMessageBox msgBox(QMessageBox::Question, tr("Import file"),
						tr("When importing the selected file, do you want to keep the existing objects?"),
						QMessageBox::NoButton, dataset()->mainWindow());

				QPushButton* cancelButton = msgBox.addButton(QMessageBox::Cancel);
				QPushButton* resetSceneButton = msgBox.addButton(tr("No"), QMessageBox::NoRole);
				QPushButton* addToSceneButton = msgBox.addButton(tr("Add to scene"), QMessageBox::YesRole);
				QPushButton* replaceSourceButton = msgBox.addButton(tr("Replace selected"), QMessageBox::AcceptRole);
				msgBox.setDefaultButton(resetSceneButton);
				msgBox.setEscapeButton(cancelButton);
				msgBox.exec();

				if(msgBox.clickedButton() == cancelButton) {
					return false; // Operation canceled by user.
				}
				else if(msgBox.clickedButton() == resetSceneButton) {
					importMode = ResetScene;
					// Ask user if current scene should be saved before it is replaced by the imported data.
					if(!dataset()->container()->askForSaveChanges())
						return false;
				}
				else if(msgBox.clickedButton() == addToSceneButton) {
					importMode = AddToScene;
				}
				else {
					importMode = ReplaceSelected;
				}
			}
		}
		else {
			if(importMode == AskUser) {
				// Ask user if the current scene should be completely replaced by the imported data.
				QMessageBox::StandardButton result = QMessageBox::question(dataset()->mainWindow(), tr("Import file"),
					tr("Do you want to keep the existing objects in the current scene?"),
					QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Cancel);

				if(result == QMessageBox::Cancel) {
					return false; // Operation canceled by user.
				}
				else if(result == QMessageBox::No) {
					importMode = ResetScene;

					// Ask user if current scene should be saved before it is replaced by the imported data.
					if(!dataset()->container()->askForSaveChanges())
						return false;
				}
				else {
					importMode = AddToScene;
				}
			}
		}
	}
	else {
		if(importMode == AddToScene)
			importMode = ResetScene;
	}

	if(importMode == ResetScene) {
		existingFileSource = nullptr;
		existingNode = nullptr;
		dataset()->clearScene();
		if(!dataset()->undoStack().isRecording())
			dataset()->undoStack().clear();
	}
	else if(importMode == AddToScene) {
		existingFileSource = nullptr;
		existingNode = nullptr;
	}

	UndoableTransaction transaction(dataset()->undoStack(), tr("Import '%1'").arg(QFileInfo(sourceUrl.path()).fileName()));

	// Do not create any animation keys during import.
	AnimationSuspender animSuspender(this);

	OORef<FileSource> fileSource;

	// Create the object that will insert the imported data into the scene.
	if(existingFileSource == nullptr) {
		fileSource = new FileSource(dataset());

		// When adding the imported data to an existing scene,
		// do not auto-adjust animation interval.
		if(importMode == AddToScene)
			fileSource->setAdjustAnimationIntervalEnabled(false);
	}
	else
		fileSource = existingFileSource;

	// Set the input location and importer.
	if(!fileSource->setSource(sourceUrl, this)) {
		return false;
	}

	// Create a new object node in the scene for the linked data.
	SceneRoot* scene = dataset()->sceneRoot();
	OORef<ObjectNode> node;
	if(existingNode == nullptr) {
		{
			UndoSuspender unsoSuspender(this);	// Do not create undo records for this part.

			// Add object to scene.
			node = new ObjectNode(dataset());
			node->setDataProvider(fileSource);

			// Let the import subclass customize the node.
			prepareSceneNode(node, fileSource);
		}

		// Insert node into scene.
		scene->addChild(node);
	}
	else node = existingNode;

	// Select import node.
	dataset()->selection()->setNode(node);

	// Jump to the right frame to show the originally selected file.
	int jumpToFrame = -1;
	for(int frameIndex = 0; frameIndex < fileSource->frames().size(); frameIndex++) {
		if(fileSource->frames()[frameIndex].sourceFile == sourceUrl) {
			jumpToFrame = frameIndex;
			break;
		}
	}

	// Adjust the animation length number to match the number of frames in the input data source.
	fileSource->adjustAnimationInterval(jumpToFrame);

	// Adjust views to completely show the newly imported object.
	OORef<DataSet> ds(dataset());
	ds->runWhenSceneIsReady([ds]() {
		ds->viewportConfig()->zoomToSelectionExtents();
	});

	transaction.commit();
	return true;
}

/******************************************************************************
* Returns the list of files that match the given wildcard pattern.
******************************************************************************/
Future<QVector<FileSourceImporter::Frame>> FileSourceImporter::findWildcardMatches(const QUrl& sourceUrl, DataSetContainer* datasetContainer)
{
	QVector<Frame> frames;

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
			if(!datasetContainer->taskManager().waitForTask(fileListFuture))
				return Future<QVector<Frame>>::createCanceled();

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

	return Future<QVector<Frame>>::createImmediate(frames);
}

/******************************************************************************
* Checks if a filename matches to the given wildcard pattern.
******************************************************************************/
bool FileSourceImporter::matchesWildcardPattern(const QString& pattern, const QString& filename)
{
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
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
