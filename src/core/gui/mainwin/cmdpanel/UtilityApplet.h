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
 * \file UtilityApplet.h
 * \brief Contains the definition of the Ovito::UtilityApplet class.
 */

#ifndef __OVITO_UTILITY_APPLET_H
#define __OVITO_UTILITY_APPLET_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/gui/widgets/RolloutContainer.h>

namespace Ovito {

/**
 * \brief Abstract base class for utility applets.
 * 
 * Utility applets are activated and shown on the utility page of the command panel.
 * 
 * For each installed UtilityApplet-derived class
 * a button is shown on the utilities page of the command panel. When the
 * user presses a button, an instance of the corresponding UtilityApplet class
 * is created and its openUtility() method is called.
 * 
 * When the utilities page of the command panel is closed, the current applet is
 * also closed by calling its closeUtility() method.
 */
class UtilityApplet : public RefMaker
{
protected:
	
	/// \brief The default constructor.
	UtilityApplet() {}

public:

	/// \brief Shows the user interface of the utility in the given RolloutContainer.
	/// \param container The widget container into which the utility should put its user interface controls.
	/// \param rolloutParams Specifies how the utility should insert its rollouts into the container.
	/// \throw Exception if an error occurs.
	virtual void openUtility(RolloutContainer* container, const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters()) = 0;

	/// \brief Closes the user interface of the utility.
	/// \param container The widget container from which the utility should remove its user interface controls.
	virtual void closeUtility(RolloutContainer* container) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_UTILITY_APPLET_H
