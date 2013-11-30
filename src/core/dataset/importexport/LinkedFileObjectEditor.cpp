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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanActionParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
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
	QWidget* rollout = createRollout(tr("External data source"), rolloutParams);

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

	QAction* saveDataWithSceneAction = toolbar->addAction(QIcon(":/core/actions/file/import_object_save_with_scene.png"), tr("Store imported data in scene file"));
	new BooleanActionParameterUI(this, "saveWithScene", saveDataWithSceneAction);

	QGroupBox* sourceBox = new QGroupBox(tr("Source"), rollout);
	layout->addWidget(sourceBox);
	QGridLayout* gridlayout = new QGridLayout(sourceBox);
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1,1);
	gridlayout->setVerticalSpacing(2);
	gridlayout->setHorizontalSpacing(6);
	_filenameLabel = new QLineEdit();
	_filenameLabel->setReadOnly(true);
	_filenameLabel->setFrame(false);
	gridlayout->addWidget(new QLabel(tr("File:")), 0, 0);
	gridlayout->addWidget(_filenameLabel, 0, 1);
	_sourcePathLabel = new QLineEdit();
	_sourcePathLabel->setReadOnly(true);
	_sourcePathLabel->setFrame(false);
	gridlayout->addWidget(new QLabel(tr("Dir:")), 1, 0);
	gridlayout->addWidget(_sourcePathLabel, 1, 1);

	QGroupBox* statusBox = new QGroupBox(tr("Status"), rollout);
	layout->addWidget(statusBox);
	sublayout = new QVBoxLayout(statusBox);
	sublayout->setContentsMargins(4,4,4,4);
	_statusLabel = new ObjectStatusWidget(rollout);
	sublayout->addWidget(_statusLabel);

	// Create another rollout for animation settings.
	rollout = createRollout(tr("Frame sequence"), rolloutParams.after(rollout).collapse());

	// Create the rollout contents.
	layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGroupBox* wildcardBox = new QGroupBox(tr("File wildcard pattern"), rollout);
	layout->addWidget(wildcardBox);
	sublayout = new QVBoxLayout(wildcardBox);
	sublayout->setContentsMargins(4,4,4,4);
	_wildcardPatternTextbox = new QLineEdit();
	connect(_wildcardPatternTextbox, SIGNAL(returnPressed()), this, SLOT(onWildcardPatternEntered()));
	sublayout->addWidget(_wildcardPatternTextbox);

	QGroupBox* frameSequenceBox = new QGroupBox(tr("Input frames"), rollout);
	layout->addWidget(frameSequenceBox);
	sublayout = new QVBoxLayout(frameSequenceBox);
	sublayout->setContentsMargins(4,4,4,4);

	QHBoxLayout* subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	subsublayout->setSpacing(2);
	subsublayout->addWidget(new QLabel(tr("Current:")));
	_framesListBox = new QComboBox();
	_framesListBox->setEditable(false);
	_framesListBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	connect(_framesListBox, SIGNAL(activated(int)), this, SLOT(onFrameSelected(int)));
	subsublayout->addWidget(_framesListBox, 1);
	sublayout->addLayout(subsublayout);

	subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	subsublayout->setSpacing(2);
	IntegerParameterUI* playbackSpeedNumeratorUI = new IntegerParameterUI(this, PROPERTY_FIELD(LinkedFileObject::_playbackSpeedNumerator));
	playbackSpeedNumeratorUI->setMinValue(1);
	subsublayout->addWidget(new QLabel(tr("Playback speed:")));
	subsublayout->addWidget(playbackSpeedNumeratorUI->textBox());
	subsublayout->addWidget(playbackSpeedNumeratorUI->spinner());
	subsublayout->addWidget(new QLabel(tr("/")));
	IntegerParameterUI* playbackSpeedDenominatorUI = new IntegerParameterUI(this, PROPERTY_FIELD(LinkedFileObject::_playbackSpeedDenominator));
	playbackSpeedDenominatorUI->setMinValue(1);
	subsublayout->addWidget(playbackSpeedDenominatorUI->textBox());
	subsublayout->addWidget(playbackSpeedDenominatorUI->spinner());
	sublayout->addLayout(subsublayout);

	subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	IntegerParameterUI* playbackStartUI = new IntegerParameterUI(this, PROPERTY_FIELD(LinkedFileObject::_playbackStartTime));
	subsublayout->addWidget(new QLabel(tr("Start at animation frame:")));
	subsublayout->addLayout(playbackStartUI->createFieldLayout());
	sublayout->addLayout(subsublayout);

	BooleanParameterUI* adjustAnimIntervalUI = new BooleanParameterUI(this, PROPERTY_FIELD(LinkedFileObject::_adjustAnimationIntervalEnabled));
	sublayout->addWidget(adjustAnimIntervalUI->checkBox());

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

	updateInformationLabel();

	// Close old sub-editors.
	_subEditors.clear();
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(newObject);
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
	if(obj) {
		obj->refreshFromSource(obj->loadedFrame());
		obj->notifyDependents(ReferenceEvent::TargetChanged);
	}
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
		if(!obj->importer())
			return;

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
* Updates the displayed status informations.
******************************************************************************/
void LinkedFileObjectEditor::updateInformationLabel()
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	if(!obj) {
		_wildcardPatternTextbox->clear();
		_wildcardPatternTextbox->setEnabled(false);
		_sourcePathLabel->setText(QString());
		_filenameLabel->setText(QString());
		_statusLabel->clearStatus();
		_framesListBox->clear();
		_framesListBox->setEnabled(false);
		return;
	}

	QString wildcardPattern;
	if(obj->sourceUrl().isLocalFile()) {
		QFileInfo fileInfo(obj->sourceUrl().toLocalFile());
		_sourcePathLabel->setText(fileInfo.dir().path());
		wildcardPattern = fileInfo.fileName();
	}
	else {
		QFileInfo fileInfo(obj->sourceUrl().path());
		QUrl url = obj->sourceUrl();
		url.setPath(fileInfo.path());
		_sourcePathLabel->setText(url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded));
		wildcardPattern = fileInfo.fileName();
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
	else {
		_filenameLabel->setText(QString());
	}

	_framesListBox->setEnabled(true);
	for(int index = 0; index < obj->frames().size(); index++) {
		if(_framesListBox->count() <= index) {
			_framesListBox->addItem(obj->frames()[index].label);
		}
		else {
			if(_framesListBox->itemText(index) != obj->frames()[index].label)
				_framesListBox->setItemText(index, obj->frames()[index].label);
		}
	}
	for(int index = _framesListBox->count() - 1; index >= obj->frames().size(); index--) {
		_framesListBox->removeItem(index);
	}
	_framesListBox->setCurrentIndex(frameIndex);

	_statusLabel->setStatus(obj->status());
}

/******************************************************************************
* Is called when the user has selected a certain frame in the frame list box.
******************************************************************************/
void LinkedFileObjectEditor::onFrameSelected(int index)
{
	LinkedFileObject* obj = static_object_cast<LinkedFileObject>(editObject());
	if(!obj) return;

	AnimManager::instance().setTime(obj->inputFrameToAnimationTime(index));
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool LinkedFileObjectEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject()) {
		if(event->type() == ReferenceEvent::ObjectStatusChanged || event->type() == ReferenceEvent::TitleChanged) {
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
