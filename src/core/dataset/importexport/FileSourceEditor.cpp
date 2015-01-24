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
#include <core/gui/properties/FilenameParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanActionParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/animation/AnimationSettings.h>
#include "FileSourceEditor.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Core, FileSourceEditor, PropertiesEditor);

/******************************************************************************
* Sets up the UI of the editor.
******************************************************************************/
void FileSourceEditor::createUI(const RolloutInsertionParameters& rolloutParams)
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

	toolbar->addAction(QIcon(":/core/actions/file/import_object_changefile.png"), tr("Pick new local input file"), this, SLOT(onPickLocalInputFile()));
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
	gridlayout->addWidget(new QLabel(tr("Path:")), 1, 0);
	gridlayout->addWidget(_sourcePathLabel, 1, 1);

	QGroupBox* statusBox = new QGroupBox(tr("Status"), rollout);
	layout->addWidget(statusBox);
	sublayout = new QVBoxLayout(statusBox);
	sublayout->setContentsMargins(4,4,4,4);
	_statusLabel = new StatusWidget(rollout);
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
	connect(_wildcardPatternTextbox, &QLineEdit::returnPressed, this, &FileSourceEditor::onWildcardPatternEntered);
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
	connect(_framesListBox, (void (QComboBox::*)(int))&QComboBox::activated, this, &FileSourceEditor::onFrameSelected);
	subsublayout->addWidget(_framesListBox, 1);
	sublayout->addLayout(subsublayout);

	subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	subsublayout->setSpacing(2);
	IntegerParameterUI* playbackSpeedNumeratorUI = new IntegerParameterUI(this, PROPERTY_FIELD(FileSource::_playbackSpeedNumerator));
	playbackSpeedNumeratorUI->setMinValue(1);
	subsublayout->addWidget(new QLabel(tr("Playback speed:")));
	subsublayout->addWidget(playbackSpeedNumeratorUI->textBox());
	subsublayout->addWidget(playbackSpeedNumeratorUI->spinner());
	subsublayout->addWidget(new QLabel(tr("/")));
	IntegerParameterUI* playbackSpeedDenominatorUI = new IntegerParameterUI(this, PROPERTY_FIELD(FileSource::_playbackSpeedDenominator));
	playbackSpeedDenominatorUI->setMinValue(1);
	subsublayout->addWidget(playbackSpeedDenominatorUI->textBox());
	subsublayout->addWidget(playbackSpeedDenominatorUI->spinner());
	sublayout->addLayout(subsublayout);

	subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	IntegerParameterUI* playbackStartUI = new IntegerParameterUI(this, PROPERTY_FIELD(FileSource::_playbackStartTime));
	subsublayout->addWidget(new QLabel(tr("Start at animation frame:")));
	subsublayout->addLayout(playbackStartUI->createFieldLayout());
	sublayout->addLayout(subsublayout);

	BooleanParameterUI* adjustAnimIntervalUI = new BooleanParameterUI(this, PROPERTY_FIELD(FileSource::_adjustAnimationIntervalEnabled));
	sublayout->addWidget(adjustAnimIntervalUI->checkBox());

	// Show settings editor of importer class.
	new SubObjectParameterUI(this, PROPERTY_FIELD(FileSource::_importer), rolloutParams.after(rollout));

	_subEditorRolloutParams = rolloutParams.collapse();
}

/******************************************************************************
* Is called when a new object has been loaded into the editor.
******************************************************************************/
void FileSourceEditor::onEditorContentsReplaced(RefTarget* newObject)
{
	updateInformationLabel();

	// Close old sub-editors.
	_subEditors.clear();
	if(newObject) {
		FileSource* obj = static_object_cast<FileSource>(newObject);
		// Open new sub-editors.
		for(DataObject* dataObj : obj->dataObjects()) {
			OORef<PropertiesEditor> subEditor = dataObj->createPropertiesEditor();
			if(subEditor) {
				subEditor->initialize(container(), mainWindow(), _subEditorRolloutParams);
				subEditor->setEditObject(dataObj);
				_subEditors.push_back(subEditor);
			}
		}
	}
}

/******************************************************************************
* Is called when the user presses the "Pick local input file" button.
******************************************************************************/
void FileSourceEditor::onPickLocalInputFile()
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
	if(obj)
		obj->showFileSelectionDialog(container()->window());
}

/******************************************************************************
* Is called when the user presses the "Pick remote input file" button.
******************************************************************************/
void FileSourceEditor::onPickRemoteInputFile()
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
	if(obj)
		obj->showURLSelectionDialog(container()->window());
}

/******************************************************************************
* Is called when the user presses the Reload frame button.
******************************************************************************/
void FileSourceEditor::onReloadFrame()
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
	if(obj) {
		obj->refreshFromSource(obj->loadedFrameIndex());
		obj->notifyDependents(ReferenceEvent::TargetChanged);
	}
}

/******************************************************************************
* Is called when the user presses the Reload animation button.
******************************************************************************/
void FileSourceEditor::onReloadAnimation()
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
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
void FileSourceEditor::onWildcardPatternEntered()
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
	OVITO_CHECK_OBJECT_POINTER(obj);

	undoableTransaction(tr("Change wildcard pattern"), [this, obj]() {
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
void FileSourceEditor::updateInformationLabel()
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
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

	int frameIndex = obj->loadedFrameIndex();
	if(frameIndex >= 0) {
		const FileSourceImporter::Frame& frameInfo = obj->frames()[frameIndex];
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
void FileSourceEditor::onFrameSelected(int index)
{
	FileSource* obj = static_object_cast<FileSource>(editObject());
	if(!obj) return;

	dataset()->animationSettings()->setTime(obj->inputFrameToAnimationTime(index));
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool FileSourceEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject()) {
		if(event->type() == ReferenceEvent::ObjectStatusChanged || event->type() == ReferenceEvent::TitleChanged) {
			updateInformationLabel();
		}
		else if(event->type() == ReferenceEvent::ReferenceAdded || event->type() == ReferenceEvent::ReferenceRemoved) {
			ReferenceFieldEvent* refEvent = static_cast<ReferenceFieldEvent*>(event);
			if(refEvent->field() == PROPERTY_FIELD(FileSource::_dataObjects)) {
				DataObject* dataObj = dynamic_object_cast<DataObject>(event->type() == ReferenceEvent::ReferenceAdded ? refEvent->newTarget() : refEvent->oldTarget());
				if(dataObj) {
					if(event->type() == ReferenceEvent::ReferenceAdded) {
						// Open a new sub-editor.
						OORef<PropertiesEditor> subEditor = dataObj->createPropertiesEditor();
						if(subEditor) {
							subEditor->initialize(container(), mainWindow(), _subEditorRolloutParams);
							subEditor->setEditObject(dataObj);
							_subEditors.push_back(subEditor);
						}
					}
					else {
						// Close sub-editor.
						for(int i = (int)_subEditors.size() - 1; i >= 0; i--) {
							if(_subEditors[i]->editObject() == dataObj)
								_subEditors.erase(_subEditors.begin() + i);
						}
					}
				}
			}
		}
	}
	return PropertiesEditor::referenceEvent(source, event);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
