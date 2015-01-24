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
#include "AnimationKeys.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AnimationKey, RefTarget);
DEFINE_PROPERTY_FIELD(AnimationKey, _time, "Time");
SET_PROPERTY_FIELD_LABEL(AnimationKey, _time, "Time");
SET_PROPERTY_FIELD_UNITS(AnimationKey, _time, TimeParameterUnit);

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FloatAnimationKey, AnimationKey);
DEFINE_PROPERTY_FIELD(FloatAnimationKey, _value, "Value");
SET_PROPERTY_FIELD_LABEL(FloatAnimationKey, _value, "Value");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, IntegerAnimationKey, AnimationKey);
DEFINE_PROPERTY_FIELD(IntegerAnimationKey, _value, "Value");
SET_PROPERTY_FIELD_LABEL(IntegerAnimationKey, _value, "Value");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, Vector3AnimationKey, AnimationKey);
DEFINE_PROPERTY_FIELD(Vector3AnimationKey, _value, "Value");
SET_PROPERTY_FIELD_LABEL(Vector3AnimationKey, _value, "Value");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, PositionAnimationKey, AnimationKey);
DEFINE_PROPERTY_FIELD(PositionAnimationKey, _value, "Value");
SET_PROPERTY_FIELD_LABEL(PositionAnimationKey, _value, "Value");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, RotationAnimationKey, AnimationKey);
DEFINE_PROPERTY_FIELD(RotationAnimationKey, _value, "Value");
SET_PROPERTY_FIELD_LABEL(RotationAnimationKey, _value, "Value");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ScalingAnimationKey, AnimationKey);
DEFINE_PROPERTY_FIELD(ScalingAnimationKey, _value, "Value");
SET_PROPERTY_FIELD_LABEL(ScalingAnimationKey, _value, "Value");

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
