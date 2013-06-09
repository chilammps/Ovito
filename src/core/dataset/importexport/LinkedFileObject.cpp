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
#include <core/undo/UndoManager.h>
#include <core/scene/animation/AnimManager.h>
#include <core/data/ObjectLoadStream.h>
#include <core/data/ObjectSaveStream.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/ObjectNode.h>
#include <core/gui/ApplicationManager.h>
#include <core/gui/properties/FilenamePropertyUI.h>
#include <core/gui/properties/BooleanActionPropertyUI.h>
#include <core/gui/dialogs/ImportFileDialog.h>
#include <core/data/importexport/ImportExportManager.h>
#include "AtomsImportObject.h"

namespace AtomViz {

IMPLEMENT_SERIALIZABLE_PLUGIN_CLASS(AtomsImportObject, SceneObject)
DEFINE_FLAGS_REFERENCE_FIELD(AtomsImportObject, AtomsObject, "Atoms", PROPERTY_FIELD_ALWAYS_DEEP_COPY, _atoms)
DEFINE_FLAGS_REFERENCE_FIELD(AtomsImportObject, AtomsFileParser, "File Parser", PROPERTY_FIELD_ALWAYS_DEEP_COPY, _parser)
DEFINE_PROPERTY_FIELD(AtomsImportObject, "FramesPerSnapshot", _framesPerSnapshot)
DEFINE_PROPERTY_FIELD(AtomsImportObject, "AdjustAnimationInterval", _adjustAnimationInterval)
SET_PROPERTY_FIELD_LABEL(AtomsImportObject, _atoms, "Atoms")
SET_PROPERTY_FIELD_LABEL(AtomsImportObject, _parser, "Parser")
SET_PROPERTY_FIELD_LABEL(AtomsImportObject, _framesPerSnapshot, "Animation frames per simulation snapshot")
SET_PROPERTY_FIELD_LABEL(AtomsImportObject, _adjustAnimationInterval, "Adjust animation interval")

/******************************************************************************
* Constructs the object.
******************************************************************************/
AtomsImportObject::AtomsImportObject(bool isLoading)
	: SceneObject(isLoading), _loadedMovieFrame(0), _framesPerSnapshot(1), _adjustAnimationInterval(true)
{
	INIT_PROPERTY_FIELD(AtomsImportObject, _atoms);
	INIT_PROPERTY_FIELD(AtomsImportObject, _parser);
	INIT_PROPERTY_FIELD(AtomsImportObject, _framesPerSnapshot);
	INIT_PROPERTY_FIELD(AtomsImportObject, _adjustAnimationInterval);
	if(!isLoading) {

		// Create the AtomsObject where the imported data will be stored.
		_atoms = new AtomsObject();
		_atoms->setSerializeAtoms(false);

		// Assume periodic boundary conditions by default.
		_atoms->simulationCell()->setPeriodicity(true, true, true);
	}
}

/******************************************************************************
* Asks the object for the result of the geometry pipeline at the given time.
******************************************************************************/
PipelineFlowState AtomsImportObject::evalObject(TimeTicks time)
{
	TimeInterval interval = TimeForever;
	if(!atomsObject() || !parser() || parser()->numberOfMovieFrames() <= 0) return PipelineFlowState(NULL, interval);

	int frame = ANIM_MANAGER.timeToFrame(time);
	int snapshot = frame / framesPerSnapshot();

	if(snapshot < 0) snapshot = 0;
	else if(snapshot >= parser()->numberOfMovieFrames()) snapshot = parser()->numberOfMovieFrames() - 1;
	frame = snapshot * framesPerSnapshot();

	if(snapshot != _loadedMovieFrame) {
		try {
			// Do not record this operation.
			UndoSuspender undoSuspender;
			// Do not create any animation keys.
			AnimationSuspender animSuspender;

			// Call the format specific parser.
			_loadedMovieFrame = snapshot;
			setStatus(parser()->loadAtomsFile(atomsObject(), snapshot, true));
		}
		catch(Exception& ex) {
			// Transfer exception message to evaluation status.
			QString msg = ex.message();
			for(int i=1; i<ex.messages().size(); i++) {
				msg += "\n";
				msg += ex.messages()[i];
			}
			setStatus(EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, msg));

			ex.prependGeneralMessage(tr("Failed to load snapshot %1 of sequence.").arg(snapshot));
			ex.logError();
		}
	}

	// Calculate the validity interval of the current simulation snapshot.
	interval.intersect(atomsObject()->objectValidity(time));

	if(snapshot > 0) interval.setStart(max(interval.start(), ANIM_MANAGER.frameToTime(frame)));
	if(snapshot < parser()->numberOfMovieFrames() - 1) interval.setEnd(min(interval.end(), ANIM_MANAGER.frameToTime(frame+1)-1));

	return PipelineFlowState(atomsObject(), interval);
}

/******************************************************************************
* Sets the parser used by this object.
******************************************************************************/
void AtomsImportObject::setParser(AtomsFileParser* parser)
{
	this->_parser = parser;
}

/******************************************************************************
* This will reload the current movie frame.
* Note: Throws an exception on error.
* Returns false when the operation has been canceled by the user.
******************************************************************************/
bool AtomsImportObject::reloadInputFile()
{
	try {
		if(!parser() || !atomsObject())
			throw tr("No parser has been specified.");

		// Do not create any animation keys.
		AnimationSuspender animSuspender;
		// Do not record this operation.
		UndoSuspender undoSuspender;


		if(parser()->numberOfMovieFrames() <= 0)
			throw Exception(tr("Atomic input file does not contain any atoms."));
		if(_loadedMovieFrame < 0) _loadedMovieFrame = 0;
		else if(_loadedMovieFrame >= parser()->numberOfMovieFrames()) _loadedMovieFrame = parser()->numberOfMovieFrames() - 1;

		// Now load the atoms.
		setStatus(parser()->loadAtomsFile(atomsObject(), _loadedMovieFrame));
		// Check if operation has been canceled by the user.
		if(status().type() == EvaluationStatus::EVALUATION_ERROR)
			throw tr("Loading operation canceled by the user.");

		// Adjust the animation interval.
		if(_adjustAnimationInterval) {
			if(parser()->numberOfMovieFrames() > 1) {
				TimeInterval interval(0, ANIM_MANAGER.frameToTime((parser()->numberOfMovieFrames()-1) * framesPerSnapshot()));
				ANIM_MANAGER.setAnimationInterval(interval);
			}
			else {
				ANIM_MANAGER.setAnimationInterval(TimeInterval(ANIM_MANAGER.frameToTime(0)));
				ANIM_MANAGER.setTime(ANIM_MANAGER.frameToTime(0));
			}
		}

		return true;
	}
	catch(const QString& msg) {
		setStatus(EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, msg));
		return false;
	}
	catch(const Exception& ex) {
		// Transfer exception message to evaluation status.
		QString msg = ex.message();
		for(int i=1; i<ex.messages().size(); i++) {
			msg += "\n";
			msg += ex.messages()[i];
		}
		setStatus(EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, msg));
		throw ex;
	}
}

/******************************************************************************
* Stores the parser status and sends a notification message.
******************************************************************************/
void AtomsImportObject::setStatus(const EvaluationStatus& status)
{
	if(status == _loadStatus) return;
	_loadStatus = status;
	notifyDependents(REFTARGET_STATUS_CHANGED);
}

/******************************************************************************
* Asks the object for its validity interval at the given time.
******************************************************************************/
TimeInterval AtomsImportObject::objectValidity(TimeTicks time)
{
	return TimeForever;
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void AtomsImportObject::saveToStream(ObjectSaveStream& stream)
{
	SceneObject::saveToStream(stream);
	stream.beginChunk(0x68725A1);
	stream << (quint32)_loadedMovieFrame;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void AtomsImportObject::loadFromStream(ObjectLoadStream& stream)
{
	SceneObject::loadFromStream(stream);
	stream.expectChunk(0x68725A1);
	quint32 loadedFrame;
	stream >> loadedFrame;
	this->_loadedMovieFrame = loadedFrame;
	stream.closeChunk();
}

/******************************************************************************
* This method is called once for this object after it has been loaded from the input stream
******************************************************************************/
void AtomsImportObject::loadFromStreamComplete()
{
	SceneObject::loadFromStreamComplete();

	CHECK_POINTER(atomsObject());
	if(!storeAtomsWithScene() && atomsObject() && parser()) {
		// Load atomic data from external file.
		try {
			reloadInputFile();
		}
		catch(Exception& ex) {
			ex.prependGeneralMessage(tr("Failed to restore atom data from external file. Sorry, your atoms are gone. Non-existing external data file: %1").arg(inputFile()));
			ex.showError();
		}
	}
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
RefTarget::SmartPtr AtomsImportObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	AtomsImportObject::SmartPtr clone = static_object_cast<AtomsImportObject>(SceneObject::clone(deepCopy, cloneHelper));

	// Copy internal data.
	clone->_loadedMovieFrame = this->_loadedMovieFrame;

	return clone;
}

/******************************************************************************
* From RefMaker.
* This method is called when an object referenced by this object
* sends a notification message.
******************************************************************************/
bool AtomsImportObject::onRefTargetMessage(RefTarget* source, RefTargetMessage* msg)
{
	// Generate SUBOBJECT_LIST_CHANGED message if a data channel is added or removed from our AtomsObject
	// since we replicate its list of sub-objects.
	if((msg->type() == REFERENCE_FIELD_ADDED || msg->type() == REFERENCE_FIELD_REMOVED || msg->type() == REFERENCE_FIELD_CHANGED) &&
			msg->sender() == atomsObject()) {
		notifyDependents(SUBOBJECT_LIST_CHANGED);
	}
	return SceneObject::onRefTargetMessage(source, msg);
}

/******************************************************************************
* Returns the title of this object.
******************************************************************************/
QString AtomsImportObject::schematicTitle()
{
	if(!parser()) return SceneObject::schematicTitle();
	return tr("Data source - %1").arg(parser()->schematicTitle());
}

/******************************************************************************
* Returns the number of sub-objects that should be displayed in the modifier stack.
******************************************************************************/
int AtomsImportObject::editableSubObjectCount()
{
	if(atomsObject()) {
		return atomsObject()->dataChannels().size() + 1;
	}
	return 0;
}

/******************************************************************************
* Returns a sub-object that should be listed in the modifier stack.
******************************************************************************/
RefTarget* AtomsImportObject::editableSubObject(int index)
{
	OVITO_ASSERT(atomsObject());
	if(index == 0)
		return atomsObject()->simulationCell();
	else if(index <= atomsObject()->dataChannels().size())
		return atomsObject()->dataChannels()[index - 1];
	OVITO_ASSERT(false);
	return NULL;
}

/******************************************************************************
* Displays the file selection dialog and lets the user select a new input file.
******************************************************************************/
void AtomsImportObject::showSelectionDialog(QWidget* parent)
{
	try {
		ImporterExporter::SmartPtr importer;
		QString importFile;

		// Put code in a block: Need to release dialog before loading new input file.
		{
			// Let the user select a file.
			ImportFileDialog dialog(parent, tr("Import"));
			if(!dialog.exec())
				return;

			// Create a parser object based on the selected filename filter.
			importFile = dialog.fileToImport();
			importer = dialog.createParser();
			if(!importer) return;
		}
		AtomsFileParser::SmartPtr newParser = dynamic_object_cast<AtomsFileParser>(importer);
		if(!newParser)
			throw Exception(tr("You did not select a file that contains an atomistic dataset."));

		// Try to re-use the existing parser.
		if(parser() && parser()->pluginClassDescriptor() == newParser->pluginClassDescriptor())
			newParser = parser();
		CHECK_OBJECT_POINTER(newParser.get());

		ViewportSuspender noVPUpdate;

		// Scan the input file.
		if(!newParser->setInputFile(importFile))
			return;

		// Show settings dialog.
		if(!newParser->showSettingsDialog(parent))
			return;

		setParser(newParser.get());
		reloadInputFile();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}


IMPLEMENT_PLUGIN_CLASS(AtomsImportObjectEditor, PropertiesEditor)

/******************************************************************************
* Sets up the UI of the editor.
******************************************************************************/
void AtomsImportObjectEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Data source"), rolloutParams, "atomviz.objects.import_atoms_object", "atomviz.objects.import_atoms_object.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);

	QToolBar* toolbar = new QToolBar(rollout);
	toolbar->setStyleSheet("QToolBar { padding: 0px; margin: 0px; border: 0px none black; }");
	layout->addWidget(toolbar);

	FilenamePropertyUI* inputFilePUI = new FilenamePropertyUI(this, "inputFile", SLOT(showSelectionDialog(QWidget*)));

	toolbar->addAction(QIcon(":/atomviz/icons/import_newfile.png"), tr("Change input file"), inputFilePUI, SLOT(showSelectionDialog()));
	toolbar->addAction(QIcon(":/atomviz/icons/import_reload.png"), tr("Reload input file"), this, SLOT(onReload()));
	parserSettingsAction = toolbar->addAction(QIcon(":/atomviz/icons/import_settings.png"), tr("Settings"), this, SLOT(onParserSettings()));

	QAction* storeAtomsWithSceneAction = toolbar->addAction(QIcon(":/atomviz/icons/store_with_scene.png"), tr("Store imported data with scene"));
	new BooleanActionPropertyUI(this, "storeAtomsWithScene", storeAtomsWithSceneAction);

	animationSettingsAction = toolbar->addAction(QIcon(":/atomviz/icons/animation_settings.png"), tr("Animation settings"), this, SLOT(onAnimationSettings()));
	animationSettingsAction->setVisible(APPLICATION_MANAGER.experimentalMode());

	layout->addWidget(new QLabel(tr("<b>File:</b>"), rollout));
	filenameLabel = new ElidedTextLabel(rollout);
	filenameLabel->setIndent(10);
	filenameLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	layout->addWidget(filenameLabel);

	layout->addWidget(new QLabel(tr("<b>Directory:</b>"), rollout));
	filepathLabel = new ElidedTextLabel(rollout);
	filepathLabel->setIndent(10);
	filepathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	layout->addWidget(filepathLabel);

	layout->addWidget(new QLabel(tr("<b>Info:</b>"), rollout));

	QGridLayout* layout2 = new QGridLayout();
	layout2->setContentsMargins(0,0,0,0);
	layout2->setColumnStretch(1, 1);
	_statusIconLabel = new QLabel(rollout);
	_statusIconLabel->setAlignment(Qt::AlignTop);
	layout2->addWidget(_statusIconLabel, 0, 0, Qt::AlignTop);

	_statusTextLabel = new QLabel(rollout);
	_statusTextLabel->setAlignment(Qt::AlignTop);
	_statusTextLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	_statusTextLabel->setWordWrap(true);
	layout2->addWidget(_statusTextLabel, 0, 1);

	statusWarningIcon.load(":/atomviz/icons/modifier_status_warning.png");
	statusErrorIcon.load(":/atomviz/icons/modifier_status_error.png");

	layout->addLayout(layout2);
}

/******************************************************************************
* Is called when the editor gets associated with an object.
******************************************************************************/
void AtomsImportObjectEditor::setEditObject(RefTarget* newObject)
{
	PropertiesEditor::setEditObject(newObject);

	// Enable/disable button for the settings dialog depending on whether such a dialog box
	// is provided by the selected parser.

	AtomsImportObject* obj = static_object_cast<AtomsImportObject>(newObject);
	if(obj && obj->parser() && obj->parser()->hasSettingsDialog())
		parserSettingsAction->setEnabled(true);
	else
		parserSettingsAction->setEnabled(false);

	updateInformationLabel();
}

/******************************************************************************
* Is called when the user presses the Reload button.
******************************************************************************/
void AtomsImportObjectEditor::onReload()
{
	AtomsImportObject* obj = static_object_cast<AtomsImportObject>(editObject());
	CHECK_OBJECT_POINTER(obj);
	try {
		ViewportSuspender noVPUpdate;
		obj->reloadInputFile();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Is called when the user presses the Parser Settings button.
******************************************************************************/
void AtomsImportObjectEditor::onParserSettings()
{
	AtomsImportObject* obj = static_object_cast<AtomsImportObject>(editObject());
	CHECK_OBJECT_POINTER(obj);

	try {
		if(obj->parser() == NULL)
			throw Exception(tr("There is no parser object available."));

		// Show settings dialog.
		if(!obj->parser()->showSettingsDialog(container()))
			return;

		ViewportSuspender noVPUpdate;
		obj->reloadInputFile();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Updates the contents of the status label.
******************************************************************************/
void AtomsImportObjectEditor::updateInformationLabel()
{
	AtomsImportObject* obj = static_object_cast<AtomsImportObject>(editObject());
	if(!obj) return;

	QFileInfo fileInfo(obj->sourceFile());

	filenameLabel->setText(fileInfo.fileName());
	filepathLabel->setText(fileInfo.absolutePath());

	_statusTextLabel->setText(obj->status().longMessage());
	if(obj->status().type() == EvaluationStatus::EVALUATION_WARNING)
		_statusIconLabel->setPixmap(statusWarningIcon);
	else if(obj->status().type() == EvaluationStatus::EVALUATION_ERROR)
		_statusIconLabel->setPixmap(statusErrorIcon);
	else
		_statusIconLabel->clear();

	animationSettingsAction->setEnabled(obj->parser() && obj->parser()->numberOfMovieFrames() > 1);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool AtomsImportObjectEditor::onRefTargetMessage(RefTarget* source, RefTargetMessage* msg)
{
	if(source == editObject() && (msg->type() == REFTARGET_STATUS_CHANGED || msg->type() == SCHEMATIC_TITLE_CHANGED)) {
		updateInformationLabel();
	}
	return PropertiesEditor::onRefTargetMessage(source, msg);
}

/******************************************************************************
* Is called when the user presses the Animation Settings button.
******************************************************************************/
void AtomsImportObjectEditor::onAnimationSettings()
{
	AtomsImportObject* obj = static_object_cast<AtomsImportObject>(editObject());
	CHECK_OBJECT_POINTER(obj);

	AtomsImportObjectAnimationSettingsDialog dialog(obj, container());
	dialog.exec();
}

/******************************************************************************
* Dialog box constructor.
******************************************************************************/
AtomsImportObjectAnimationSettingsDialog::AtomsImportObjectAnimationSettingsDialog(AtomsImportObject* importObject, QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("Animation settings"));
	this->importObject = importObject;

	QVBoxLayout* layout1 = new QVBoxLayout(this);

	// Time steps group
	QGroupBox* playbackGroupBox = new QGroupBox(tr("Playback speed"), this);
	layout1->addWidget(playbackGroupBox);

	QGridLayout* contentLayout = new QGridLayout(playbackGroupBox);
	contentLayout->setSpacing(0);
	contentLayout->setColumnStretch(1, 1);
	contentLayout->addWidget(new QLabel(tr("Animation frames per snapshot:"), this), 0, 0);
	QLineEdit* framePerSnapshotBox = new QLineEdit(this);
	contentLayout->addWidget(framePerSnapshotBox, 0, 1);
	framePerSnapshotSpinner = new SpinnerWidget(this);
	framePerSnapshotSpinner->setTextBox(framePerSnapshotBox);
	framePerSnapshotSpinner->setMinValue(1);
	framePerSnapshotSpinner->setIntValue(importObject->framesPerSnapshot());
	framePerSnapshotSpinner->setUnit(UNITS_MANAGER.integerIdentity());
	contentLayout->addWidget(framePerSnapshotSpinner, 0, 2);

	adjustAnimationIntervalBox = new QCheckBox(tr("Adjust animation interval"), this);
	adjustAnimationIntervalBox->setChecked(true);
	layout1->addWidget(adjustAnimationIntervalBox);

	// Ok and Cancel buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* This is called when the user has pressed the OK button.
******************************************************************************/
void AtomsImportObjectAnimationSettingsDialog::onOk()
{
	UNDO_MANAGER.beginCompoundOperation(tr("Change animation settings"));
	try {
		// Write settings back to the import object.
		importObject->setFramesPerSnapshot(framePerSnapshotSpinner->intValue());

		if(adjustAnimationIntervalBox->isChecked()) {
			// Adjust the animation interval.
			if(importObject->parser() && importObject->parser()->numberOfMovieFrames() > 1) {
				TimeInterval interval(0, ANIM_MANAGER.frameToTime((importObject->parser()->numberOfMovieFrames()-1) * importObject->framesPerSnapshot()));
				ANIM_MANAGER.setAnimationInterval(interval);
			}
		}

		// Close dialog box.
		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
		UNDO_MANAGER.currentCompoundOperation()->clear();
	}
	UNDO_MANAGER.endCompoundOperation();
}

};
