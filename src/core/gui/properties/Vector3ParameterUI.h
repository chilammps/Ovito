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

#ifndef __OVITO_VECTOR3_PARAMETER_UI_H
#define __OVITO_VECTOR3_PARAMETER_UI_H

#include <core/Core.h>
#include "FloatParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* A parameter UI for Vector3 properties. 
* This ParameterUI lets the user edit one of the X, Y and Z components of the vector.
******************************************************************************/
class OVITO_CORE_EXPORT Vector3ParameterUI : public FloatParameterUI
{
public:

	/// Constructor for a Qt property.
	Vector3ParameterUI(QObject* parentEditor, const char* propertyName, size_t vectorComponent, const QString& labelText = QString(), const QMetaObject* parameterUnitType = nullptr);

	/// Constructor for a PropertyField or ReferenceField property.
	Vector3ParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, size_t vectorComponent);
	
	/// This method updates the displayed value of the parameter UI.
	virtual void updateUI() override;
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	virtual void updatePropertyValue() override;
	
private:

	/// The vector component to control (0 - 2).
	size_t _component;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VECTOR3_PARAMETER_UI_H
