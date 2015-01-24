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

#ifndef __OVITO_PROPERTIES_PANEL_H
#define __OVITO_PROPERTIES_PANEL_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/gui/widgets/general/RolloutContainer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/******************************************************************************
* This panel lets the user edit the properties of some RefTarget derived object.
******************************************************************************/
class OVITO_CORE_EXPORT PropertiesPanel : public RolloutContainer
{
	Q_OBJECT
	
public:
	
	/// Constructs the panel.
	PropertiesPanel(QWidget* parent);

	/// Destructs the panel.
	virtual ~PropertiesPanel();
	
	/// Returns the target object being edited in the panel.
	RefTarget* editObject() const;
	
	/// Sets the target object being edited in the panel.
	void setEditObject(RefTarget* newEditObject);

	/// Returns the editor that is responsible for the object being edited.
	PropertiesEditor* editor() const { return _editor; }
	
protected:

	/// The editor for the current object.
	OORef<PropertiesEditor> _editor;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PROPERTIES_PANEL_H
