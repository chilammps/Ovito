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
 * \file BondsObject.h
 * \brief Contains the definition of the Ovito::BondsObject class.
 */

#ifndef __OVITO_BONDS_OBJECT_H
#define __OVITO_BONDS_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/SceneObject.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Stores the bonds between particles.
 */
class BondsObject : public SceneObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE BondsObject() {}

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Bonds"); }

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_BONDS_OBJECT_H
