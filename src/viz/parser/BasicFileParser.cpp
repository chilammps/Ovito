///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
#include <core/scene/animation/AnimManager.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/objects/SceneObject.h>
#include <core/data/DataSetManager.h>
#include <core/gui/mainwnd/MainFrame.h>
#include <core/gui/panels/CommandPanel.h>
#include <core/actions/ActionManager.h>

#include "AtomsFileParser.h"
#include "AtomsImportObject.h"
#include <atomviz/atoms/AtomsObject.h>

namespace AtomViz {

IMPLEMENT_ABSTRACT_PLUGIN_CLASS(AtomsFileParser, ImporterExporter)
DEFINE_PROPERTY_FIELD(AtomsFileParser, "InputFile", _inputFilename)
DEFINE_PROPERTY_FIELD(AtomsFileParser, "SourceFile", _sourceFilename)
SET_PROPERTY_FIELD_LABEL(AtomsFileParser, _inputFilename, "Input filename")
SET_PROPERTY_FIELD_LABEL(AtomsFileParser, _sourceFilename, "Source filename")

/******************************************************************************
* Imports the given file into the scene.
* Return true if the file has been imported.
* Return false if the import has been aborted by the user.
* Throws an exception when the import has failed.
******************************************************************************/
bool AtomsFileParser::importFile(const QString& filePath, DataSet* dataset, bool suppressDialogs)
{
	AtomsImportObject::SmartPtr obj;

	// Save old scene.
	if(!DATASET_MANAGER.askForSaveChanges())
		return false;

	{
		UndoSuspender noUndo;

		// Set the input filename.
		if(!setInputFile(filePath))
			return false;

		// Show settings dialog.
		if(!suppressDialogs) {
			if(!showSettingsDialog(NULL))
				return false;
		}

		// Create the scene object that will hold the imported atoms.
		obj = new AtomsImportObject();
		// Assign parser to the import object.
		obj->setParser(this);
		// Load atoms.
		if(!obj->reloadInputFile())
			return false;
	}

	// Make the import undoable.
	UNDO_MANAGER.beginCompoundOperation(tr("Import atoms file"));
	try {
		// Do not create any animation keys.
		AnimationSuspender animSuspender;

		// Clear scene first.
		dataset->clearScene();

		SceneRoot* scene = dataset->sceneRoot();
		CHECK_OBJECT_POINTER(scene);

		ObjectNode::SmartPtr node;
		{
			UndoSuspender unsoSuspender;	// Do not create undo records for this.

			// Add object to scene.
			node = new ObjectNode();
			node->setSceneObject(obj);
			node->setDisplayColor(Color(0.5, 0.5, 1.0));

			// Give the new node a name.
			node->setName(tr("Atoms"));
		}

		// Insert node into scene.
		scene->addChild(node);

		// Select new node.
		dataset->selection()->clear();
		dataset->selection()->add(node);

		UNDO_MANAGER.endCompoundOperation();
	}
	catch(...) {
		UNDO_MANAGER.endCompoundOperation();
		throw;
	}

	// Show the newly created object in the viewports.
	if(dataset == DATASET_MANAGER.currentSet())
		ACTION_MANAGER.findActionProxy(ACTION_VIEWPORT_ZOOM_SELECTION_EXTENTS_ALL)->trigger();

	// Switch to the modification page of the command panel.
	if(APPLICATION_MANAGER.guiMode())
		MAIN_FRAME->commandPanel()->setCurrentPage(CommandPanel::MODIFY_PAGE);

	return true;
}

/******************************************************************************
* Returns the title of this object.
******************************************************************************/
QString AtomsFileParser::schematicTitle()
{
	if(!sourceFile().isEmpty()) {
		QString filename = QFileInfo(sourceFile()).fileName();
		if(!filename.isEmpty())
			return filename;
	}
	return ImporterExporter::schematicTitle();
}

};	// End of namespace AtomViz
