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

#ifndef __OVITO_COMMAND_PANEL_H
#define __OVITO_COMMAND_PANEL_H

#include <core/Core.h>
#include <core/gui/widgets/general/RolloutContainer.h>
#include <core/dataset/DataSetManager.h>

namespace Ovito {

class RefTarget;			// defined in RefTarget.h
class CommandPanelPage;		// defined below
class ModifyCommandPage;	// defined in ModifyCommandPage.h
class CreationCommandPage;	// defined in CreationCommandPage.h
class RenderCommandPage;	// defined in RenderCommandPage.h
class UtilityCommandPage;	// defined in UtilityCommandPage.h

/******************************************************************************
* The command panel in the main window.
******************************************************************************/
class OVITO_CORE_EXPORT CommandPanel : public QWidget
{
	Q_OBJECT

public:

	/// The set of pages of the command panel.
	enum Page {
		MODIFY_PAGE		= 0,
		RENDER_PAGE		= 1,
		UTILITIES_PAGE	= 2,
		CREATION_PAGE	= 3,
	};

	/// \brief Creates the command panel.
	/// \param parent The parent widget.
	CommandPanel(QWidget* parent = nullptr);

	/// \brief Activate one of the command pages.
	/// \param newPage The identifier of the page to activate.
	/// \sa currentPage()
	void setCurrentPage(Page newPage);

	/// \brief Returns the active command page.
	/// \return The identifier of the page that is currently active.
	Page currentPage() const { return (Page)tabWidget->currentIndex(); }

	/// \brief Returns the object creation page contained in the command panel.
	CreationCommandPage* creationPage() const { return _creationPage; }

	/// \brief Returns the modification page contained in the command panel.
	ModifyCommandPage* modifyPage() const { return _modifyPage; }

	/// \brief Returns the rendering page contained in the command panel.
	RenderCommandPage* renderPage() const { return _renderPage; }

	/// \brief Returns the utility page contained in the command panel.
	UtilityCommandPage* utilityPage() const { return _utilityPage; }

	/// \brief Returns the object that is currently being edited in the command panel.
	/// \return The object being edited in the command panel or \c NULL if no object is selected.
	RefTarget* editObject() const;

	/// \brief Returns the default size for the command panel.
	virtual QSize sizeHint() const { return QSize(336,300); }

protected Q_SLOTS:

	/// This is called after all changes to the selection set have been completed.
	void onSelectionChangeComplete(SelectionSet* newSelection);

	/// Is called when the user has switched to another tab in the command panel.
	void onTabSwitched();

	/// Resets the command panel to the initial state.
	void reset();

private:

	QTabWidget* tabWidget;
	int lastPage;

	ModifyCommandPage* _modifyPage;
	CreationCommandPage* _creationPage;
	RenderCommandPage* _renderPage;
	UtilityCommandPage* _utilityPage;

	friend class MainFrame;
};

/******************************************************************************
* Base class for all pages in the command panel.
******************************************************************************/
class CommandPanelPage : public QWidget
{
	Q_OBJECT

public:

	/// Constructor.
	CommandPanelPage() : QWidget() {}

	/// Resets the page to its initial state.
	virtual void reset() {}

	/// Is called when the user selects the page.
	virtual void onEnter() {}

	/// Is called when the user selects another page.
	virtual void onLeave() {}

	/// This is called after all changes to the selection set have been completed.
	virtual void onSelectionChangeComplete(SelectionSet* newSelection) {}
};

};

#endif // __OVITO_COMMAND_PANEL_H
