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

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, ModifierApplication, RefTarget)
DEFINE_REFERENCE_FIELD(ModifierApplication, _modifier, "Modifier", Modifier)
SET_PROPERTY_FIELD_LABEL(ModifierApplication, _modifier, "Modifier")

/******************************************************************************
* Constructor.
******************************************************************************/
ModifierApplication::ModifierApplication(Modifier* mod)
{
	INIT_PROPERTY_FIELD(ModifierApplication::_modifier);
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
* Explicitly sets the status of this modifier application. This
* is an internal function.
******************************************************************************/
void ModifierApplication::setStatus(const ObjectStatus& status)
{
	if(status == _evalStatus) return;
	_evalStatus = status;
	notifyDependents(ReferenceEvent::StatusChanged);
}

};
