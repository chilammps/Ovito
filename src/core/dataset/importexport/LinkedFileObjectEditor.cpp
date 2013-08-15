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
#include <core/gui/properties/FilenameParameterUI.h>
#include <core/gui/properties/BooleanActionParameterUI.h>
#include <core/gui/properties/SubObjectParameterUI.h>
#include "LinkedFileObjectEditor.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, LinkedFileObjectEditor, PropertiesEditor)

/******************************************************************************
* Sets up the UI of the editor.
******************************************************************************/
void LinkedFileObjectEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(rolloutParams.title().isEmpty() ? tr("External file") : rolloutParams.title(), rolloutParams);

	// Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QVBoxLayout* sublayout;

	QToolBar* toolbar = new QToolBar(rollout);
	toolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
	layout->addWidget(toolbar);

	FilenameParameterUI* inputFilePUI = new FilenameParameterUI(this, "sourceUrl", SLOT(showFileSelectionDialog(QWidget*)));

	toolbar->addAction(QIcon(":/core/actions/file/import_object_changefile.png"), tr("Pick new local input file"), inputFilePUI, SLOT(showSelectionDialog()));
	toolbar->addAction(QIcon(":/core/actions/file/file_import_remote.png"), tr("Pick new remote input file"), this, SLOT(onPickRemoteInputFile()));
	toolbar->addAction(QIcon(":/core/actions/file/import_object_reload.png"), tr("Reload current input file"), this, SLOT(onReloadFrame()));
	toolbar->addAction(QIcon(":/core/actions/file/import_object_refresh_animation.png"), tr("Reload animation frames"), this, SLOT(onReloadAnimation()));
	_parserSettingsAction = toolbar->addAction(QIcon(":/core/actions/file/import_object_settings.png"), tr("Import settings"), this, SLOT(onParserSettings()));

	QAction* saveDataWithSceneAction = toolbar->addAction(QIcon(":/core/actions/file/import_object_save_with_scene.png"), tr("Store imported data in scene file"));
	new BooleanActionParameterUI(this, "saveDataWithScene", saveDataWithSceneAction);

	QGroupBox* wildcardBox = new QGroupBox(tr("File sequence pattern"), rollout);
	layout->addWidget(wildcardBox);
	QHBoxLayout* sublayout2 = new QHBoxLayout(wildcardBox);
	sublayout2->setContentsMargins(4,4,4,4);
	_wildcardPatternTextbox = new QLineEdit();
	connect(_wildcardPatternTextbox, SIGNAL(returnPressed()), this, SLOT(onWildcardPatternEntered()));
	sublayout2->addWidget(_wildcardPatternTextbox, 1);
	QPushButton* setPatternButton = new QPushButton(tr("Set"));
	connect(setPatternButton, SIGNAL(clicked(bool)), this, SLOT(onWildcardPatternEntered()));
	sublayout2->addWidget(setPatternButton);

	QGroupBox* sourcePathBox = new QGroupBox(tr("Source path"), rollout);
	layout->addWidget(sourcePathBox);
	sublayout = new QVBoxLayout(sourcePathBox);
	sublayout->setContentsMargins(4,4,4,4);
	_sourcePathLabel = new ElidedTextLabel();
	_sourcePathLabel->setIndent(10);
	_sourcePathLabel->setTextInteractionFlags(Qt::TextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard));
	sublayout->addWidget(_sourcePathLabel);

	QGroupBox* currentFileBox = new QGroupBox(tr("Current file"), rollout);
	layout->addWidget(currentFileBox);
	sublayout = new QVBoxLayout(currentFileBox);
	sublayout->setContentsMargins(4,4,4,4);
	_filenameLabel = new ElidedTextLabel();
	_filenameLabel->setIndent(10);
	_filenameLabel->setTextInteractionFlags(Qt::TextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard));
	sublayout->addWidget(_filenameLabel);

	QGroupBox* statusBox = new QGroupBox(tr("Status"), rollout);
	layout->addWidget(statusBox);
	QGridLayout* layout2 = new QGridLayout(statusBox);
	layout2->setContentsMargins(4,4,4,4);
	layout2->setColumnStretch(1, 1);
	_statusIconLabel = new QLabel(rollout);
	_statusIconLabel->setAlignment(Qt::AlignTop);
	layout2->addWidget(_statusIconLabel, 0, 0, Qt::AlignTop);

	_statusTextLabel = new QLabel(rollout);
	_statusTextLabel->setAlignment(Qt::AlignTop);
	_statusTextLabel->setTextInteractionFlags(Qt::TextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard));
	_statusTextLabel->setWordWrap(true);
	layout2->addWidget(_statusTextLabel, 0, 1);

	_statusWarningIcon.load(":/core/mainwin/status/status_warning.png");
	_statusErrorIcon.load(":/core/mainwin/status/status_error.png");

	// Show settings editor of importer class.
	new SubObjectParameterUI(this, PROPERTY_FIELD(LinkedFileObject::_importer), rolloutParams.after(rollout));

	_subEditorRolloutParams = rolloutParams.collapse();
}

/******************************************************************************
* Is called when the editor gets associated with an object.
******************************************************************************/
void LinkedFileObjectEditor::setEditObject(RefTarget* newObject)
{
	PropertiesEditor::setEditObject(newObject);

	// Enable/disable button for the settings dialog depending on whether such a dialog box
	// is provided by the selected parser.

	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(newObject);
	if(obj && obj->importer() && obj->importer()->hasSettingsDialog())
		_parserSettingsAction->setEnabled(true);
	else
		_parserSettingsAction->setEnabled(false);

	updateInformationLabel();

	// Close old sub-editors.
	_subEditors.clear();
	if(obj) {
		// Open new sub-editors.
		for(SceneObject* sceneObj : obj->sceneObjects()) {
			OORef<PropertiesEditor> subEditor = sceneObj->createPropertiesEditor();
			if(subEditor) {
				subEditor->initialize(container(), _subEditorRolloutParams);
				subEditor->setEditObject(sceneObj);
				_subEditors.push_back(subEditor);
			}
		}
	}
}

/******************************************************************************
* Is called when the user presses the "Pick remote input file" button.
******************************************************************************/
void LinkedFileObjectEditor::onPickRemoteInputFile()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	if(obj)
		obj->showURLSelectionDialog();
}

/******************************************************************************
* Is called when the user presses the Reload frame button.
******************************************************************************/
void LinkedFileObjectEditor::onReloadFrame()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	if(obj)
		obj->refreshFromSource(obj->loadedFrame());
}

/******************************************************************************
* Is called when the user presses the Reload animation button.
******************************************************************************/
void LinkedFileObjectEditor::onReloadAnimation()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	OVITO_CHECK_OBJECT_POINTER(obj);
	try {
		obj->updateFrames();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
	// Adjust the animation length number to match the number of frames in the input data source.
	obj->adjustAnimationInterval();
}

/******************************************************************************
* This is called when the user has changed the source URL.
******************************************************************************/
void LinkedFileObjectEditor::onWildcardPatternEntered()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	OVITO_CHECK_OBJECT_POINTER(obj);

	UndoableTransaction::handleExceptions(tr("Change wildcard pattern"), [this, obj]() {

		QString pattern = _wildcardPatternTextbox->text().trimmed();
		if(pattern.isEmpty())
			return;

		QUrl newUrl = obj->sourceUrl();
		QFileInfo fileInfo(newUrl.path());
		fileInfo.setFile(fileInfo.dir(), pattern);
		newUrl.setPath(fileInfo.filePath());
		if(!newUrl.isValid())
			throw Exception(tr("URL is not valid."));

		obj->setSource(newUrl, obj->importer());
	});
	updateInformationLabel();
}

/******************************************************************************
* Is called when the user presses the Parser Settings button.
******************************************************************************/
void LinkedFileObjectEditor::onParserSettings()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	OVITO_CHECK_OBJECT_POINTER(obj);

	try {
		if(obj->importer() == nullptr)
			throw Exception(tr("There is no parser available."));

		// Show settings dialog.
		obj->importer()->showSettingsDialog(container(), obj);
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Updates the contents of the status label.
******************************************************************************/
void LinkedFileObjectEditor::updateInformationLabel()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	if(!obj) {
		_wildcardPatternTextbox->clear();
		_wildcardPatternTextbox->setEnabled(false);
		_sourcePathLabel->setText(QString());
		_filenameLabel->setText(QString());
		return;
	}

	QFileInfo fileInfo(obj->sourceUrl().path());
	QUrl url = obj->sourceUrl();
	url.setPath(fileInfo.path());
	_sourcePathLabel->setText(url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded));

	QString wildcardPattern = fileInfo.fileName();
	if(!wildcardPattern.contains('*') && !wildcardPattern.contains('?')) {
		// Generate a default wildcard pattern.
		// Replace last sequence of numbers in the filename with a wildcard character.
		int startIndex, endIndex;
		for(endIndex = wildcardPattern.length()-1; endIndex >= 0; endIndex--)
			if(wildcardPattern.at(endIndex).isNumber()) break;
		if(endIndex >= 0) {
			for(startIndex = endIndex-1; startIndex >= 0; startIndex--)
				if(!wildcardPattern.at(startIndex).isNumber()) break;
			wildcardPattern = wildcardPattern.left(startIndex+1) + '*' + wildcardPattern.mid(endIndex+1);
		}
	}
	_wildcardPatternTextbox->setText(wildcardPattern);
	_wildcardPatternTextbox->setEnabled(true);

	int frameIndex = obj->loadedFrame();
	if(frameIndex >= 0) {
		const LinkedFileImporter::FrameSourceInformation& frameInfo = obj->frames()[frameIndex];
		if(frameInfo.sourceFile.isLocalFile()) {
			_filenameLabel->setText(QFileInfo(frameInfo.sourceFile.toLocalFile()).fileName());
		}
		else {
			_filenameLabel->setText(QFileInfo(frameInfo.sourceFile.path()).fileName());
		}
	}
	else _filenameLabel->setText(QString());

	_statusTextLabel->setText(obj->status().longText());
	if(obj->status().type() == ObjectStatus::Warning)
		_statusIconLabel->setPixmap(_statusWarningIcon);
	else if(obj->status().type() == ObjectStatus::Error)
		_statusIconLabel->setPixmap(_statusErrorIcon);
	else
		_statusIconLabel->clear();

	container()->updateRolloutsLater();
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool LinkedFileObjectEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject()) {
		if(event->type() == ReferenceEvent::StatusChanged || event->type() == ReferenceEvent::TitleChanged) {
			updateInformationLabel();
		}
		else if(event->type() == ReferenceEvent::ReferenceAdded || event->type() == ReferenceEvent::ReferenceRemoved) {
			ReferenceFieldEvent* refEvent = static_cast<ReferenceFieldEvent*>(event);
			if(refEvent->field() == PROPERTY_FIELD(LinkedFileObject::_sceneObjects)) {
				SceneObject* sceneObj = dynamic_object_cast<SceneObject>(event->type() == ReferenceEvent::ReferenceAdded ? refEvent->newTarget() : refEvent->oldTarget());
				if(sceneObj) {
					if(event->type() == ReferenceEvent::ReferenceAdded) {
						// Open a new sub-editor.
						OORef<PropertiesEditor> subEditor = sceneObj->createPropertiesEditor();
						if(subEditor) {
							subEditor->initialize(container(), _subEditorRolloutParams);
							subEditor->setEditObject(sceneObj);
							_subEditors.push_back(subEditor);
						}
					}
					else {
						// Close sub-editor.
						for(int i = _subEditors.size() - 1; i >= 0; i--) {
							if(_subEditors[i]->editObject() == sceneObj)
								_subEditors.erase(_subEditors.begin() + i);
						}
					}
				}
			}
		}
	}
	return PropertiesEditor::referenceEvent(source, event);
}

};
