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
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/AnimManager.h>

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, Modifier, RefTarget)
DEFINE_PROPERTY_FIELD(Modifier, _isEnabled, "IsEnabled")
SET_PROPERTY_FIELD_LABEL(Modifier, _isEnabled, "Enabled")

/******************************************************************************
* Constructor.
******************************************************************************/
Modifier::Modifier() : _isEnabled(true)
{
	INIT_PROPERTY_FIELD(Modifier::_isEnabled);
}

/******************************************************************************
* Returns the list of applications associated with this modifier. 
******************************************************************************/
QVector<ModifierApplication*> Modifier::modifierApplications() const
{
	QVector<ModifierApplication*> apps;
	Q_FOREACH(RefMaker* dependent, dependents()) {
        ModifierApplication* modApp = dynamic_object_cast<ModifierApplication>(dependent);
		if(modApp != NULL && modApp->modifier() == this) 
			apps.push_back(modApp);
	}
	return apps;	
}

/******************************************************************************
* Returns the input object of this modifier for each application of the modifier. 
* This method evaluates the modifier stack up this modifier.
* Note: This method might return empty result objects in some cases when the modifier stack
* cannot be evaluated because of an invalid modifier.
******************************************************************************/
QMap<ModifierApplication*, PipelineFlowState> Modifier::getModifierInputs(TimePoint time) const
{	
	UndoSuspender noUndo;
		
	QMap<ModifierApplication*, PipelineFlowState> result;	
	Q_FOREACH(ModifierApplication* app, modifierApplications()) {
		PipelineObject* pipelineObj = app->pipelineObject();
		if(!pipelineObj) continue;
		
		result[app] = pipelineObj->evalObject(time, app, false);
	}

	return result;
}

/******************************************************************************
* Same function as above but using the current animation time as 
* evaluation time and only returning the input object for the first application
* of this modifier.
******************************************************************************/
PipelineFlowState Modifier::getModifierInput() const 
{
	UndoSuspender noUndo;
		
	Q_FOREACH(ModifierApplication* app, modifierApplications()) {
		PipelineObject* pipelineObj = app->pipelineObject();
		if(!pipelineObj) continue;
		return pipelineObj->evalObject(AnimManager::instance().time(), app, false);
	}

	return PipelineFlowState();
}


};
