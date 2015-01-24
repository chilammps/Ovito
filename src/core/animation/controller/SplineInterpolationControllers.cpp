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
#include "SplineInterpolationControllers.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, FloatSplineAnimationKey, FloatAnimationKey);
DEFINE_PROPERTY_FIELD(FloatSplineAnimationKey, _inTangent, "InTangent");
DEFINE_PROPERTY_FIELD(FloatSplineAnimationKey, _outTangent, "OutTangent");
SET_PROPERTY_FIELD_LABEL(FloatSplineAnimationKey, _inTangent, "In Tangent");
SET_PROPERTY_FIELD_LABEL(FloatSplineAnimationKey, _outTangent, "Out Tangent");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, PositionSplineAnimationKey, PositionAnimationKey);
DEFINE_PROPERTY_FIELD(PositionSplineAnimationKey, _inTangent, "InTangent");
DEFINE_PROPERTY_FIELD(PositionSplineAnimationKey, _outTangent, "OutTangent");
SET_PROPERTY_FIELD_LABEL(PositionSplineAnimationKey, _inTangent, "In Tangent");
SET_PROPERTY_FIELD_LABEL(PositionSplineAnimationKey, _outTangent, "Out Tangent");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SplinePositionController, KeyframeController);

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
