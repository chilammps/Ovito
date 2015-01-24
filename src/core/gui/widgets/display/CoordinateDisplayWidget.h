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

#ifndef __OVITO_COORDINATE_DISPLAY_WIDGET_H
#define __OVITO_COORDINATE_DISPLAY_WIDGET_H

#include <core/Core.h>
#include <core/gui/widgets/general/SpinnerWidget.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * The coordinate display widget at the bottom of the main window,
 * which displays the current mouse coordinates and the transform of the selected object.
 */
class OVITO_CORE_EXPORT CoordinateDisplayWidget : public QFrame
{
	Q_OBJECT
	
public:
	
	/// \brief Constructs the widget.
	CoordinateDisplayWidget(DataSetContainer& datasetContainer, QWidget* parent = nullptr);

	/// \brief Shows the coordinate display widget.
	void activate(const QString& undoOperationName);

	/// \brief Deactivates the coordinate display widget.
	void deactivate();

	/// Sets the values displayed by the coordinate display widget.
	void setValues(const Vector3& xyz) {
		if(!_spinners[0]->isDragging()) _spinners[0]->setFloatValue(xyz.x());
		if(!_spinners[1]->isDragging()) _spinners[1]->setFloatValue(xyz.y());
		if(!_spinners[2]->isDragging()) _spinners[2]->setFloatValue(xyz.z());
	}

	/// Returns the values displayed by the coordinate display widget.
	Vector3 getValues() const {
		return Vector3(
				_spinners[0]->floatValue(),
				_spinners[1]->floatValue(),
				_spinners[2]->floatValue());
	}

	/// Sets the units of the displayed values.
	void setUnit(ParameterUnit* unit) {
		_spinners[0]->setUnit(unit);
		_spinners[1]->setUnit(unit);
		_spinners[2]->setUnit(unit);
	}

Q_SIGNALS:

	/// This signal is emitted when the user has changed the value of one of the vector components.
	void valueEntered(int component, FloatType value);

protected Q_SLOT:

	/// Is called when a spinner value has been changed by the user.
	void onSpinnerValueChanged();

	/// Is called when the user has started a spinner drag operation.
	void onSpinnerDragStart();

	/// Is called when the user has finished the spinner drag operation.
	void onSpinnerDragStop();

	/// Is called when the user has aborted the spinner drag operation.
	void onSpinnerDragAbort();

private:
	
	DataSetContainer& _datasetContainer;
	SpinnerWidget* _spinners[3];
	QString _undoOperationName;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COORDINATE_DISPLAY_WIDGET_H
