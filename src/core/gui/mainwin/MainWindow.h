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

/** 
 * \file MainWindow.h
 * \brief Contains the definition of the Ovito::MainWindow class.
 */
 
#ifndef __OVITO_MAIN_WINDOW_H
#define __OVITO_MAIN_WINDOW_H

#include <core/Core.h>
#include <core/gui/app/Application.h>

namespace Ovito {

/**
 * \brief The main window of the application.
 * 
 * Please not that in console mode no instance of the main window class is created.
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:

	/// \brief Returns the one and only instance of this class.
	/// \return The global instance of the MainWindow class.
	inline static MainWindow& instance() {
		OVITO_ASSERT_MSG(Application::instance().guiMode(), "MainWindow::instance()", "No main window available in non-gui mode.");
		OVITO_ASSERT_MSG(_instance, "MainWindow::MainWindow()", "Main window has not been created yet.");
		return *_instance;
	}

	/// Returns the main toolbar of the window.
	QToolBar* mainToolbar() const { return _mainToolbar; }
	
	/// Returns the status bar of the main window.
	QStatusBar* statusBar() const { return _statusBar; }

	/// Returns the recommended size for this window.
	virtual QSize sizeHint() const Q_DECL_OVERRIDE { return QSize(1024,768); }
	
	/// \brief Loads the layout of the docked widgets from the settings store.
	void restoreLayout();

	/// \brief Saves the layout of the docked widgets to the settings store.
	void saveLayout();

private:

	/// Constructor for the main window.
	MainWindow(const QString& title);

	/// Creates the main menu.
	void createMainMenu();

	/// Creates the main toolbar.
	void createMainToolbar();

private:

	/// The upper main toolbar.
	QToolBar* _mainToolbar;
	
	/// The internal status bar widget.
	QStatusBar* _statusBar;
	
	/// The global instance of this class.
	static MainWindow* _instance;

protected:

	/// Is called when the user closes the window.
	virtual void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
	
	/// Is called when the window receives an event.
	virtual bool event(QEvent *event) Q_DECL_OVERRIDE;

	/// Give the Application class access to the main window's private constructor.
	friend class Application;
};

};

#endif // __OVITO_MAIN_WINDOW_H
