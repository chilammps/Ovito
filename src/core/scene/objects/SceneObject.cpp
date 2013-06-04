///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2008) Alexander Stukowski
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
#include <core/scene/objects/SceneObject.h>
#include <core/scene/objects/AbstractCameraObject.h>
#include <core/viewport/Viewport.h>

namespace Core {

IMPLEMENT_ABSTRACT_PLUGIN_CLASS(SceneObject, RefTarget)
#if 0
DEFINE_VECTOR_REFERENCE_FIELD(SceneObject, AttachedObjectRenderer, "AttachedRenderers", _attachedRenderers)
SET_PROPERTY_FIELD_LABEL(SceneObject, _attachedRenderers, "Attached Renderers")
#endif
IMPLEMENT_ABSTRACT_PLUGIN_CLASS(AbstractCameraObject, SceneObject)

/******************************************************************************
* Performs a hit test on the object.
* Returns the distance of the hit from the viewer, or HIT_TEST_NONE if no hit
* was found.
******************************************************************************/
FloatType SceneObject::hitTest(TimeTicks time, Viewport* vp, ObjectNode* contextNode, const PickRegion& pickRegion)
{
	vp->setPickingRegion(&pickRegion);
	renderObject(time, contextNode, vp);
	vp->setPickingRegion(NULL); 
	return vp->closestHit();
}


};
