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

#include <plugins/pyscript/PyScript.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/RenderSettings.h>
#include "PythonViewportOverlay.h"

#ifndef signals
#define signals Q_SIGNALS
#endif
#ifndef slots
#define slots Q_SLOTS
#endif
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerpython.h>

namespace PyScript {

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(PyScript, PythonViewportOverlayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(PyScript, PythonViewportOverlay, ViewportOverlay);
SET_OVITO_OBJECT_EDITOR(PythonViewportOverlay, PythonViewportOverlayEditor);
DEFINE_PROPERTY_FIELD(PythonViewportOverlay, _script, "Script");
SET_PROPERTY_FIELD_LABEL(PythonViewportOverlay, _script, "Script");

/******************************************************************************
* Constructor.
******************************************************************************/
PythonViewportOverlay::PythonViewportOverlay(DataSet* dataset) : ViewportOverlay(dataset),
		_scriptEngine(dataset, nullptr, false), _isCompiled(false)
{
	INIT_PROPERTY_FIELD(PythonViewportOverlay::_script);

	connect(&_scriptEngine, &ScriptEngine::scriptOutput, this, &PythonViewportOverlay::onScriptOutput);
	connect(&_scriptEngine, &ScriptEngine::scriptError, this, &PythonViewportOverlay::onScriptOutput);

	// Load example script.
	setScript("import ovito\n"
			"# The following function is called by OVITO to let the script\n"
			"# draw arbitrary graphics content into the viewport.\n"
			"# It is passed a QPainter (see http://qt-project.org/doc/qt-5/qpainter.html).\n"
			"def render(painter, **args):\n"
			"\t# This demo code prints the current animation frame\n"
			"\t# into the upper left corner of the viewport.\n"
			"\txpos = 10\n"
			"\typos = 10 + painter.fontMetrics().ascent()\n"
			"\ttext = \"Frame {}\".format(ovito.dataset.anim.current_frame)\n"
			"\tpainter.drawText(xpos, ypos, text)\n"
			"\t# The following code prints the current number of particles\n"
			"\t# into the lower left corner of the viewport.\n"
			"\txpos = 10\n"
			"\typos = painter.window().height() - 10\n"
			"\tif ovito.dataset.selected_node:\n"
			"\t\tpositions = ovito.dataset.selected_node.compute().position\n"
			"\t\ttext = \"{} particles\".format(positions.size)\n"
			"\telse:\n"
			"\t\ttext = \"no particles\"\n"
			"\tpainter.drawText(xpos, ypos, text)\n");
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void PythonViewportOverlay::propertyChanged(const PropertyFieldDescriptor& field)
{
	ViewportOverlay::propertyChanged(field);
	if(field == PROPERTY_FIELD(PythonViewportOverlay::_script)) {
		compileScript();
	}
}

/******************************************************************************
* Compiles the script entered by the user.
******************************************************************************/
void PythonViewportOverlay::compileScript()
{
	_scriptOutput.clear();
	try {
		_scriptEngine.execute(script());
		_isCompiled = true;
	}
	catch(Exception& ex) {
		_scriptOutput += ex.message();
		_isCompiled = false;
	}
	notifyDependents(ReferenceEvent::ObjectStatusChanged);
}

/******************************************************************************
* Is called when the script generates some output.
******************************************************************************/
void PythonViewportOverlay::onScriptOutput(const QString& text)
{
	_scriptOutput += text;
}

/******************************************************************************
* This method asks the overlay to paint its contents over the given viewport.
******************************************************************************/
void PythonViewportOverlay::render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings)
{
	if(!_isCompiled)
		return;

	_scriptOutput.clear();
	try {
		// Pass viewport, QPainter, and other information to Python script.
		// The QPainter pointer will have to be converted to the representation used by PyQt.
		_scriptEngine.mainNamespace()["__painter_pointer"] = reinterpret_cast<unsigned long>(&painter);
		_scriptEngine.mainNamespace()["__viewport"] = boost::python::ptr(viewport);
		_scriptEngine.mainNamespace()["__projParams"] = projParams;
		_scriptEngine.mainNamespace()["__renderSettings"] = boost::python::ptr(renderSettings);
		// Execute the script's render() function.
		_scriptEngine.execute(
				"import sip\n"
				"import numpy\n"
				"import PyQt5.QtGui\n"
				"render(sip.wrapinstance(__painter_pointer, PyQt5.QtGui.QPainter), "
				"   viewport=__viewport, "
				"   render_settings=__renderSettings, "
				"   is_perspective=__projParams.isPerspective, "
				"   fov=__projParams.fieldOfView, "
				"   view_tm=numpy.asarray(__projParams.viewMatrix), "
				"   proj_tm=numpy.asarray(__projParams.projectionMatrix)"
				")");
	}
	catch(const Exception& ex) {
		_scriptOutput += ex.message();
	}
	notifyDependents(ReferenceEvent::ObjectStatusChanged);
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void PythonViewportOverlayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Python script"), rolloutParams, "viewport_overlays.python_script.html");

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	int row = 0;

	QFont font("Courier");
	font.setStyleHint(QFont::Monospace);
	font.setFixedPitch(true);

	layout->addWidget(new QLabel(tr("Python script:")), row++, 0);
	_codeEditor = new QsciScintilla();
	_codeEditor->setEnabled(false);
	_codeEditor->setAutoIndent(true);
	_codeEditor->setTabWidth(4);
	_codeEditor->setFont(font);
	QsciLexerPython* lexer = new QsciLexerPython(_codeEditor);
	lexer->setDefaultFont(font);
	_codeEditor->setLexer(lexer);
	_codeEditor->setMarginsFont(font);
	_codeEditor->setMarginWidth(0, QFontMetrics(font).width(QString::number(123)));
	_codeEditor->setMarginWidth(1, 0);
	_codeEditor->setMarginLineNumbers(0, true);
	layout->addWidget(_codeEditor, row++, 0);

	QPushButton* applyButton = new QPushButton(tr("Apply changes"));
	layout->addWidget(applyButton, row++, 0);

	layout->addWidget(new QLabel(tr("Script output:")), row++, 0);
	_errorDisplay = new QsciScintilla();
	_errorDisplay->setTabWidth(_codeEditor->tabWidth());
	_errorDisplay->setFont(font);
	_errorDisplay->setReadOnly(true);
	_errorDisplay->setMarginWidth(1, 0);
	layout->addWidget(_errorDisplay, row++, 0);

	connect(this, &PropertiesEditor::contentsChanged, this, &PythonViewportOverlayEditor::onContentsChanged);
	connect(applyButton, &QPushButton::clicked, this, &PythonViewportOverlayEditor::onApplyChanges);
}

/******************************************************************************
* Is called when the current edit object has generated a change
* event or if a new object has been loaded into editor.
******************************************************************************/
void PythonViewportOverlayEditor::onContentsChanged(RefTarget* editObject)
{
	PythonViewportOverlay* overlay = static_object_cast<PythonViewportOverlay>(editObject);
	if(editObject) {
		_codeEditor->setText(overlay->script());
		_codeEditor->setEnabled(true);
	}
	else {
		_codeEditor->setEnabled(false);
		_codeEditor->clear();
		_errorDisplay->clear();
	}
}

/******************************************************************************
* Is called when the user presses the 'Apply' button to commit the Python script.
******************************************************************************/
void PythonViewportOverlayEditor::onApplyChanges()
{
	PythonViewportOverlay* overlay = static_object_cast<PythonViewportOverlay>(editObject());
	if(!overlay) return;

	undoableTransaction(tr("Change script"), [this, overlay]() {
		overlay->setScript(_codeEditor->text());
	});
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool PythonViewportOverlayEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::ObjectStatusChanged) {
		PythonViewportOverlay* overlay = static_object_cast<PythonViewportOverlay>(editObject());
		if(overlay)
			_errorDisplay->setText(overlay->scriptOutput());
	}
	return PropertiesEditor::referenceEvent(source, event);
}

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
