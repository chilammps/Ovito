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

#ifndef __OVITO_SUBOBJECT_PARAMETER_UI_H
#define __OVITO_SUBOBJECT_PARAMETER_UI_H

#include <core/Core.h>
#include "ParameterUI.h"
#include "PropertiesEditor.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* This parameter UI will open up a sub-editor for an object that is
* referenced by the edit object.
******************************************************************************/
class OVITO_CORE_EXPORT SubObjectParameterUI : public PropertyParameterUI
{
public:

	/// Constructor.
	SubObjectParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& refField, const RolloutInsertionParameters& rolloutParams = RolloutInsertionParameters());

	/// Destructor.
	virtual ~SubObjectParameterUI() {}
    
	/// This method is called when a new sub-object has been assigned to the reference field of the editable object 
	/// this parameter UI is bound to. It is also called when the editable object itself has
	/// been replaced in the editor. 
	virtual void resetUI() override;
	
	/// Returns the current sub-editor or NULL if there is none.
	PropertiesEditor* subEditor() const { return _subEditor; }
	
	/// Sets the enabled state of the UI.
	virtual void setEnabled(bool enabled) override;
	
protected:

	/// The editor for the referenced sub-object.
	OORef<PropertiesEditor> _subEditor;
	
	/// Controls where the sub-editor is opened and whether the sub-editor is opened in a collapsed state.
	RolloutInsertionParameters _rolloutParams;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_SUBOBJECT_PARAMETER_UI_H
