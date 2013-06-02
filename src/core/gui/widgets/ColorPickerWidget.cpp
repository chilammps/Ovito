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
#include <core/gui/widgets/ColorPickerWidget.h>

namespace Ovito {

/******************************************************************************
* Constructs the control.
******************************************************************************/
ColorPickerWidget::ColorPickerWidget(QWidget* parent)
	: QPushButton(parent), _color(1,1,1)
{
	setAutoFillBackground(true);
	connect(this, SIGNAL(clicked(bool)), this, SLOT(activateColorPicker()));
	setColor(Color(0,0,0));
	setFlat(true);
}

/******************************************************************************
* Sets the current value of the color picker.
******************************************************************************/
void ColorPickerWidget::setColor(const Color& newVal, bool emitChangeSignal)
{
	if(newVal == _color) return;
	
	// Update control.
	_color = newVal;

	QColor col(_color);
	setStyleSheet(QString("QPushButton { "
				   "border-style: solid; "
				   "border-width: 1px; "
				   "border-radius: 0px; "
				   "border-color: black; "
				   "background-color: rgb(%1,%2,%3); "
				   "padding: 1px; "
				   "min-width: 16px; "
				   "}"
				   "QPushButton:pressed { "
				   "border-color: white; "
						   "}")
					   .arg(col.red()).arg(col.green()).arg(col.blue()));
	update();
	
	// Send change message
	if(emitChangeSignal)
		colorChanged();
}

/******************************************************************************
* Is called when the user has clicked on the color picker control.
******************************************************************************/
void ColorPickerWidget::activateColorPicker()
{
	QColor newColor = QColorDialog::getColor((QColor)_color, parentWidget());
	if(newColor.isValid()) {
		setColor(Color(newColor), true);				
	}
}

};
