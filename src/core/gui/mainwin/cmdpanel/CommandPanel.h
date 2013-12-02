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

namespace Ovito {

class RefTarget;			// defined in RefTarget.h
class DataSetContainer;		// defined in DataSetContainer.h
class ModifyCommandPage;	// defined in ModifyCommandPage.h
class RenderCommandPage;	// defined in RenderCommandPage.h
class UtilityCommandPage;	// defined in UtilityCommandPage.h
class MainWindow;			// defined in MainWindow.h

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
		UTILITIES_PAGE	= 2
	};

	/// \brief Creates the command panel.
	CommandPanel(MainWindow* mainWindow, QWidget* parent);

	/// \brief Activate one of the command pages.
	/// \param newPage The identifier of the page to activate.
	void setCurrentPage(Page newPage) {
		OVITO_ASSERT(newPage < _tabWidget->count());
		_tabWidget->setCurrentIndex((int)newPage);
	}

	/// \brief Returns the active command page.
	/// \return The identifier of the page that is currently active.
	Page currentPage() const { return (Page)_tabWidget->currentIndex(); }

	/// \brief Returns the modification page contained in the command panel.
	ModifyCommandPage* modifyPage() const { return _modifyPage; }

	/// \brief Returns the rendering page contained in the command panel.
	RenderCommandPage* renderPage() const { return _renderPage; }

	/// \brief Returns the utility page contained in the command panel.
	UtilityCommandPage* utilityPage() const { return _utilityPage; }

#if 0
	/// \brief Returns the object that is currently being edited in the command panel.
	/// \return The object being edited in the command panel or \c NULL if no object is selected.
	RefTarget* editObject() const;
#endif

	/// \brief Returns the default size for the command panel.
	virtual QSize sizeHint() const { return QSize(336, 300); }

private:

	QTabWidget* _tabWidget;
	ModifyCommandPage* _modifyPage;
	RenderCommandPage* _renderPage;
	UtilityCommandPage* _utilityPage;
};

};

#endif // __OVITO_COMMAND_PANEL_H
