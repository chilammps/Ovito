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

#ifndef __OVITO_UTILITY_APPLET_H
#define __OVITO_UTILITY_APPLET_H

#include <core/Core.h>
#include <core/reference/RefMaker.h>
#include <core/gui/widgets/general/RolloutContainer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(PluginSystem)

/**
 * \brief Abstract base class for utility applets.
 */
class OVITO_CORE_EXPORT UtilityApplet : public RefMaker
{
protected:
	
	/// \brief Constructor.
	UtilityApplet() {}

public:

	/// \brief Shows the user interface of the utility in the given RolloutContainer.
	/// \param mainWindow The main window that hosts the utility.
	/// \param container The widget container into which the utility should insert its user interface
	/// \param rolloutParams Specifies how the utility should insert its rollouts into the container.
	/// \throw Exception if an error occurs.
	virtual void openUtility(MainWindow* mainWindow, RolloutContainer* container, const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters()) = 0;

	/// \brief Closes the user interface of the utility.
	/// \param container The widget container from which the utility should remove its user interface.
	virtual void closeUtility(RolloutContainer* container) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_UTILITY_APPLET_H
