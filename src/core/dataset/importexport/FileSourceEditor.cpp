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
	QWidget* rollout = createRollout(tr("External file"), rolloutParams);

	// Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QVBoxLayout* sublayout;

	QToolBar* toolbar = new QToolBar(rollout);
	toolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
	layout->addWidget(toolbar);

	toolbar->addAction(QIcon(":/core/actions/file/import_object_changefile.png"), tr("Pick new file"), this, SLOT(onPickLocalInputFile()));
	toolbar->addAction(QIcon(":/core/actions/file/file_import_remote.png"), tr("Pick new remote file"), this, SLOT(onPickRemoteInputFile()));
	toolbar->addAction(QIcon(":/core/actions/file/import_object_reload.png"), tr("Reload data from external file"), this, SLOT(onReloadFrame()));
	toolbar->addAction(QIcon(":/core/actions/file/import_object_refresh_animation.png"), tr("Update time series"), this, SLOT(onReloadAnimation()));

	QAction* saveDataWithSceneAction = toolbar->addAction(QIcon(":/core/actions/file/import_object_save_with_scene.png"), tr("Store copy of loaded data in state file"));
	new BooleanActionParameterUI(this, "saveWithScene", saveDataWithSceneAction);

	QGroupBox* sourceBox = new QGroupBox(tr("Data source"), rollout);
	layout->addWidget(sourceBox);
	QGridLayout* gridlayout = new QGridLayout(sourceBox);
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1,1);
	gridlayout->setVerticalSpacing(2);
	gridlayout->setHorizontalSpacing(6);
	_filenameLabel = new QLineEdit();
	_filenameLabel->setReadOnly(true);
	_filenameLabel->setFrame(false);
	gridlayout->addWidget(new QLabel(tr("Current file:")), 0, 0);
	gridlayout->addWidget(_filenameLabel, 0, 1);
	_sourcePathLabel = new QLineEdit();
	_sourcePathLabel->setReadOnly(true);
	_sourcePathLabel->setFrame(false);
	gridlayout->addWidget(new QLabel(tr("Directory:")), 1, 0);
	gridlayout->addWidget(_sourcePathLabel, 1, 1);

	QGroupBox* wildcardBox = new QGroupBox(tr("Time series"), rollout);
	layout->addWidget(wildcardBox);
	gridlayout = new QGridLayout(wildcardBox);
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setVerticalSpacing(2);
	gridlayout->setColumnStretch(1, 1);
	_wildcardPatternTextbox = new QLineEdit();
	connect(_wildcardPatternTextbox, &QLineEdit::returnPressed, this, &FileSourceEditor::onWildcardPatternEntered);
	gridlayout->addWidget(new QLabel(tr("File pattern:")), 0, 0);
	gridlayout->addWidget(_wildcardPatternTextbox, 0, 1);
	_fileSeriesLabel = new QLabel();
	QFont smallFont = _fileSeriesLabel->font();
	smallFont.setPointSize(std::max(6, smallFont.pointSize() - 3));
	_fileSeriesLabel->setFont(smallFont);
	gridlayout->addWidget(_fileSeriesLabel, 1, 1);

	gridlayout->addWidget(new QLabel(tr("Current frame:")), 2, 0);
	_framesListBox = new QComboBox();
	_framesListBox->setEditable(false);
	_framesListBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	connect(_framesListBox, (void (QComboBox::*)(int))&QComboBox::activated, this, &FileSourceEditor::onFrameSelected);
	gridlayout->addWidget(_framesListBox, 2, 1);
	_timeSeriesLabel = new QLabel();
	_timeSeriesLabel->setFont(smallFont);
	gridlayout->addWidget(_timeSeriesLabel, 3, 1);

	QGroupBox* statusBox = new QGroupBox(tr("Status"), rollout);
	layout->addWidget(statusBox);
	sublayout = new QVBoxLayout(statusBox);
	sublayout->setContentsMargins(4,4,4,4);
	_statusLabel = new StatusWidget(rollout);
	sublayout->addWidget(_statusLabel);

	// Create another rollout for time series control.
	rollout = createRollout(tr("Animation"), rolloutParams.after(rollout).collapse());

	// Create the rollout contents.
	layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QHBoxLayout* subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	subsublayout->setSpacing(2);
	IntegerParameterUI* playbackSpeedNumeratorUI = new IntegerParameterUI(this, PROPERTY_FIELD(FileSource::_playbackSpeedNumerator));
	playbackSpeedNumeratorUI->setMinValue(1);
	subsublayout->addWidget(new QLabel(tr("Playback rate:")));
	subsublayout->addWidget(playbackSpeedNumeratorUI->textBox());
	subsublayout->addWidget(playbackSpeedNumeratorUI->spinner());
	subsublayout->addWidget(new QLabel(tr("/")));
	IntegerParameterUI* playbackSpeedDenominatorUI = new IntegerParameterUI(this, PROPERTY_FIELD(FileSource::_playbackSpeedDenominator));
	playbackSpeedDenominatorUI->setMinValue(1);
	subsublayout->addWidget(playbackSpeedDenominatorUI->textBox());
	subsublayout->addWidget(playbackSpeedDenominatorUI->spinner());
	layout->addLayout(subsublayout);

	subsublayout = new QHBoxLayout();
	subsublayout->setContentsMargins(0,0,0,0);
	IntegerParameterUI* playbackStartUI = new IntegerParameterUI(this, PROPERTY_FIELD(FileSource::_playbackStartTime));
	subsublayout->addWidget(new QLabel(tr("Start at animation frame:")));
	subsublayout->addLayout(playbackStartUI->createFieldLayout());
	layout->addLayout(subsublayout);

	BooleanParameterUI* adjustAnimIntervalUI = new BooleanParameterUI(this, PROPERTY_FIELD(FileSource::_adjustAnimationIntervalEnabled));
	layout->addWidget(adjustAnimIntervalUI->checkBox());

	// Show settings editor of importer class.
	new SubObjectParameterUI(this, PROPERTY_FIELD(FileSource::_importer), rolloutParams.after(rollout));

#if 0
	_subEditorRolloutParams = rolloutParams.collapse();
#endif
}

/******************************************************************************
* Is called when a new object has been loaded into the editor.
******************************************************************************/
void FileSourceEditor::onEditorContentsReplaced(RefTarget* newObject)
{
	updateInformationLabel();

#if 0
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
#endif
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
		_fileSeriesLabel->setText(QString());
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

	// Count the number of files matching the wildcard pattern.
	int fileSeriesCount = 0;
	QUrl lastUrl;
	for(const FileSourceImporter::Frame& frame : obj->frames()) {
		if(frame.sourceFile != lastUrl) {
			fileSeriesCount++;
			lastUrl = frame.sourceFile;
		}
	}
	if(fileSeriesCount == 0)
		_fileSeriesLabel->setText(tr("Found no matching file"));
	else if(fileSeriesCount == 1)
		_fileSeriesLabel->setText(tr("Found %1 matching file").arg(fileSeriesCount));
	else
		_fileSeriesLabel->setText(tr("Found %1 matching files").arg(fileSeriesCount));

	if(!obj->frames().empty())
		_timeSeriesLabel->setText(tr("Showing frame %1 of %2").arg(obj->loadedFrameIndex()+1).arg(obj->frames().count()));
	else
		_timeSeriesLabel->setText(tr("No frames available"));

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
	_framesListBox->setEnabled(_framesListBox->count() > 1);

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
#if 0
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
#endif
	}
	return PropertiesEditor::referenceEvent(source, event);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
