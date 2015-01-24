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

#ifndef __OVITO_AFFINE_TRANSFORMATION_PARAMETER_UI_H
#define __OVITO_AFFINE_TRANSFORMATION_PARAMETER_UI_H

#include <core/Core.h>
#include "FloatParameterUI.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Params)

/******************************************************************************
* A parameter UI for AffineTransformation type properties.
* This ParameterUI lets the user edit the individual matrix components.
******************************************************************************/
class OVITO_CORE_EXPORT AffineTransformationParameterUI : public FloatParameterUI
{
public:

	/// Constructor for a Qt property.
	AffineTransformationParameterUI(QObject* parentEditor, const char* propertyName, size_t row, size_t column, const QString& labelText = QString(), const QMetaObject* parameterUnitType = nullptr);

	/// Constructor for a PropertyField property.
	AffineTransformationParameterUI(QObject* parentEditor, const PropertyFieldDescriptor& propField, size_t row, size_t column);
	
	/// This method updates the displayed value of the parameter UI.
	virtual void updateUI() override;
	
	/// Takes the value entered by the user and stores it in the property field 
	/// this property UI is bound to. 
	virtual void updatePropertyValue() override;
	
private:

	/// The matrix component to control.
	size_t row, column;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_AFFINE_TRANSFORMATION_PARAMETER_UI_H
