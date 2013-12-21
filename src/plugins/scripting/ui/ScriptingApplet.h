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

#ifndef __OVITO_SCRIPTING_UTILITY_APPLET_H
#define __OVITO_SCRIPTING_UTILITY_APPLET_H

#include <plugins/scripting/Scripting.h>
#include <core/gui/mainwin/cmdpanel/UtilityApplet.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief A QTextEdit that captures ctrl+enter.
 */
class CodeEdit : public QTextEdit {
public:
  CodeEdit(QWidget* parent = 0) : QTextEdit(parent) {}
protected:
  void keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Return && event->modifiers().testFlag(Qt::ControlModifier))
	  ctrlEnterPressed();
	else
	  QTextEdit::keyPressEvent(event);
  }

Q_SIGNALS:
  void ctrlEnterPressed();

private:
  Q_OBJECT
};


/**
 * \brief The utility applet that integrates scripting into Ovito's user interface.
 */
class OVITO_SCRIPTING_EXPORT ScriptingApplet : public UtilityApplet
{
public:

	/// Default constructor.
	Q_INVOKABLE ScriptingApplet();

	/// Shows the UI of the utility in the given RolloutContainer.
	virtual void openUtility(MainWindow* mainWindow,
							 RolloutContainer* container,
							 const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters()
							 ) override;

	/// Removes the UI of the utility from the RolloutContainer.
	virtual void closeUtility(RolloutContainer* container) override;

public Q_SLOTS:

	/// Runs the current script in the editor.
	void runScript();

private:

	/// The main widget of the applet.
	QWidget* panel_;

	/// The script editor widget.
	CodeEdit* editor_;

	/// The output of the script.
	QLabel* output_;

	/// Reference to the main window.
	MainWindow* mainWindow_;

	Q_CLASSINFO("DisplayName", "Scripting");

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif


// Local variables:
// mode: c++
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
