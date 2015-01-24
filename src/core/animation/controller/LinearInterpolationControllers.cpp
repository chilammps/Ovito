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
#include "LinearInterpolationControllers.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Anim)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinearFloatController, KeyframeController);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinearIntegerController, KeyframeController);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinearVectorController, KeyframeController);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinearPositionController, KeyframeController);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinearRotationController, KeyframeController);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, LinearScalingController, KeyframeController);

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
