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
#include "ConstantControllers.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ConstFloatController, Controller);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ConstIntegerController, Controller);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ConstVectorController, Controller);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ConstPositionController, Controller);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ConstRotationController, Controller);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ConstScalingController, Controller);
DEFINE_PROPERTY_FIELD(ConstFloatController, _value, "Value");
DEFINE_PROPERTY_FIELD(ConstIntegerController, _value, "Value");
DEFINE_PROPERTY_FIELD(ConstVectorController, _value, "Value");
DEFINE_PROPERTY_FIELD(ConstPositionController, _value, "Value");
DEFINE_PROPERTY_FIELD(ConstRotationController, _value, "Value");
DEFINE_PROPERTY_FIELD(ConstScalingController, _value, "Value");

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
