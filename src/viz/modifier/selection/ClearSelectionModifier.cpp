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

#include <core/Core.h>
#include "ClearSelectionModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ClearSelectionModifier, ParticleModifier)

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
ObjectStatus ClearSelectionModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	ParticlePropertyObject* selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);
	if(selProperty) removeOutputProperty(selProperty);

	return ObjectStatus::Success;
}

};	// End of namespace
