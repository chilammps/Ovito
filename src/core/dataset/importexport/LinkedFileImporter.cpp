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
#include "LinkedFileImporter.h"
#include "LinkedFileObject.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinkedFileImporter, FileImporter)

/******************************************************************************
* Imports the given file into the scene.
* Return true if the file has been imported.
* Return false if the import has been aborted by the user.
* Throws an exception when the import has failed.
******************************************************************************/
bool LinkedFileImporter::importFile(const QUrl& sourceUrl, DataSet* dataset)
{
	// Do not create any animation keys during import.
	AnimationSuspender animSuspender;

	OORef<LinkedFileObject> obj;
	{
		UndoSuspender noUndo;		// Do not create undo records for this part of the operation.

		// Create the object that will feed the imported data into the scene.
		obj = new LinkedFileObject();

		// Makes this importer part of the scene object.
		obj->setImporter(this);

		// Set the input location.
		if(!obj->setSourceUrl(sourceUrl))
			return false;

		// Scan the input source for animation frames.
		if(!obj->updateFrames())
			return false;
	}

	// Make the import processes reversible.
	UndoManager::instance().beginCompoundOperation(tr("Import file"));

	try {

		// Clear scene first if requested by the caller.
		if(shouldReplaceScene())
			dataset->clearScene();

		// Create a new object node to the scene for the linked data.
		SceneRoot* scene = dataset->sceneRoot();
		OORef<ObjectNode> node;
		{
			UndoSuspender unsoSuspender;	// Do not create undo records for this part.

			// Add object to scene.
			node = new ObjectNode();
			node->setSceneObject(obj);
		}

		// Insert node into scene.
		scene->addChild(node);

		// Select new node.
		dataset->selection()->clear();
		dataset->selection()->add(node);

		// Adjust the animation length number to match the number of frames in the input data source.
		obj->adjustAnimationInterval();

		UndoManager::instance().endCompoundOperation();
	}
	catch(...) {
		UndoManager::instance().endCompoundOperation();
		throw;
	}

	// Adjust viewports to show the newly imported object.
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
	if(sourceUrl.isLocalFile()) {

		// Check if filename is a wild-card pattern.
		QFileInfo fileInfo(sourceUrl.toLocalFile());
		QString pattern = fileInfo.fileName();
		if(pattern.contains('*') == false && pattern.contains('?') == false) {
			// It's not a wild-card pattern.
			// Register only a single frame.
			frames.push_back({ sourceUrl, 0, 0, fileInfo.lastModified() });
		}
		else{

			// Scan the directory for matching files.
			QDir dir = fileInfo.dir();
			QStringList entries = dir.entryList(QStringList(pattern), QDir::Files|QDir::NoDotAndDotDot, QDir::Name);

			// Now the file names have to be sorted.
			// This is a little bit tricky since a file called "abc9.xyz" must come before
			// a file named "abc10.xyz" which is not the default lexicographical ordering.
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
				QString filename = dir.absoluteFilePath(iter);
				frames.push_back({ QUrl::fromLocalFile(filename), 0, 0, QFileInfo(filename).lastModified() });
			}
		}
	}
	else {
		// It's not a file URL.
		// Register only a single frame.
		frames.push_back({ sourceUrl, 0, 0, QDateTime() });
	}

	return Future<QVector<FrameSourceInformation>>(frames);
}

/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
Future<LinkedFileImporter::ImportedDataPtr> LinkedFileImporter::load(LinkedFileImporter::FrameSourceInformation frame)
{
	Future<LinkedFileImporter::ImportedDataPtr> future = runInBackground<ImportedDataPtr>(std::bind(&LinkedFileImporter::loadImplementation, this, std::placeholders::_1, frame));
	ProgressManager::instance().addTask(future);
	return future;
}

};
