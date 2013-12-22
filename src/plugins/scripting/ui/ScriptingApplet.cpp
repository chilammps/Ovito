///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski, Tobias Brink
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

#include <plugins/scripting/Scripting.h>
#include <core/gui/mainwin/MainWindow.h>
#include <plugins/scripting/bindings/ScriptBindings.h>
#include "ScriptingApplet.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, ScriptingApplet, UtilityApplet);

/******************************************************************************
* Initializes the utility applet.
******************************************************************************/
ScriptingApplet::ScriptingApplet() : panel_(nullptr) {
}

/******************************************************************************
* Shows the UI of the utility in the given RolloutContainer.
******************************************************************************/
void ScriptingApplet::openUtility(MainWindow* mainWindow,
								  RolloutContainer* container,
								  const RolloutInsertionParameters& rolloutParams) {
	mainWindow_ = mainWindow;

	// Create main panel widget.
	panel_ = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(panel_);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	// Create code editor widget.
	editor_ = new CodeEdit(panel_);
	editor_->setPlainText(QStringLiteral("m=modifier(\"ColorCodingModifier\");\nm.colorGradient = \"ColorCodingHotGradient\";\nm.colorGradient;\n"));
	connect(editor_, &CodeEdit::ctrlEnterPressed,
			this, &ScriptingApplet::runScript);
	layout->addWidget(editor_, 1);

	// Create output widget.
	output_ = new QLabel(panel_);
	output_->setTextFormat(Qt::PlainText);
	output_->setText("<output goes here>");
	layout->addWidget(output_, 1);

	// Create run button.
	QPushButton* runScriptBtn = new QPushButton(tr("Run"), panel_);
	connect(runScriptBtn, &QPushButton::clicked, this,
			&ScriptingApplet::runScript);
	layout->addWidget(runScriptBtn);

	// Create rollout around panel widget.
	container->addRollout(panel_, tr("Scripting"),
						  rolloutParams.useAvailableSpace());
}

/******************************************************************************
* Removes the UI of the utility from the rollout container.
******************************************************************************/
void ScriptingApplet::closeUtility(RolloutContainer* container) {
	delete panel_;
}

/******************************************************************************
* Runs the current script in the editor.
******************************************************************************/
void ScriptingApplet::runScript() {
	QObject parent; // <- for memory management.
	DataSetContainer& container = mainWindow_->datasetContainer();
	DataSet* data = container.currentSet();

	// Start recording for undo stack.
	UndoStack& undo = data->undoStack();
	UndoableTransaction transaction(undo, tr("Script execution"));

	// Set up engine.
	QScriptEngine* engine = prepareEngine(data, &parent);

	// Evaluate.
	QScriptValue result = engine->evaluate(editor_->toPlainText());
	if(result.isError()) {
		output_->setStyleSheet("QLabel { color: red; }");
		output_->setText(result.toString());
	} else {
		output_->setStyleSheet("QLabel { }");
		output_->setText(result.toString());
		if (result.isArray()) {
			QString s;
			s += "ARRAY: [";
			int length = result.property("length").toInteger();
			for (int i = 0; i != length; ++i)
				s += result.property(i).toString() + ",\n";
			s += "]";
			output_->setText(s);
		}
	}

	// If no C++ exceptions reach to here, commit the transaction.
	transaction.commit();
}

};


// Local variables:
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
