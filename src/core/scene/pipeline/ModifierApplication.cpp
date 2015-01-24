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
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/ObjectNode.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ModifierApplication, RefTarget);
DEFINE_REFERENCE_FIELD(ModifierApplication, _modifier, "Modifier", Modifier);
DEFINE_FLAGS_REFERENCE_FIELD(ModifierApplication, _modifierData, "ModifierData", RefTarget, PROPERTY_FIELD_ALWAYS_CLONE);
SET_PROPERTY_FIELD_LABEL(ModifierApplication, _modifier, "Modifier");
SET_PROPERTY_FIELD_LABEL(ModifierApplication, _modifierData, "Modifier data");

/******************************************************************************
* Constructor.
******************************************************************************/
ModifierApplication::ModifierApplication(DataSet* dataset, Modifier* mod) : RefTarget(dataset)
{
	INIT_PROPERTY_FIELD(ModifierApplication::_modifier);
	INIT_PROPERTY_FIELD(ModifierApplication::_modifierData);
	_modifier = mod;
}

/******************************************************************************
* Returns the PipelineObject this application object belongs to.
******************************************************************************/
PipelineObject* ModifierApplication::pipelineObject() const
{
	Q_FOREACH(RefMaker* dependent, dependents()) {
        PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(dependent);
		if(pipelineObj) return pipelineObj;
	}
	return nullptr;
}

/******************************************************************************
* Returns the PipelineObject this application object belongs to.
******************************************************************************/
QSet<ObjectNode*> ModifierApplication::objectNodes() const
{
	return findDependents<ObjectNode>();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
