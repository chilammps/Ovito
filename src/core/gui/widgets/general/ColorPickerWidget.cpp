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
#include "ColorPickerWidget.h"

namespace Ovito {

/******************************************************************************
* Constructs the control.
******************************************************************************/
ColorPickerWidget::ColorPickerWidget(QWidget* parent)
	: QAbstractButton(parent), _color(1,1,1)
{
	setAutoFillBackground(true);
	setColor(Color(0,0,0));
	connect(this, &ColorPickerWidget::clicked, this, &ColorPickerWidget::activateColorPicker);
}

/******************************************************************************
* Sets the current value of the color picker.
******************************************************************************/
void ColorPickerWidget::setColor(const Color& newVal, bool emitChangeSignal)
{
	if(newVal == _color) return;
	
	// Update control.
	_color = newVal;

	QPalette pal = palette();
	pal.setColor(QPalette::Button, QColor(_color));
	pal.setColor(QPalette::Active, QPalette::WindowText, Qt::white);
	setPalette(pal);
	update();
	
	// Send change message
	if(emitChangeSignal)
		colorChanged();
}

/******************************************************************************
* Paints the widget.
******************************************************************************/
void ColorPickerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setPen(palette().color(
			isEnabled() ? (isDown() ? QPalette::Active : QPalette::Inactive) : QPalette::Disabled,
					QPalette::WindowText));
	painter.drawRect(0,0,width()-1,height()-1);
}

/******************************************************************************
* Returns the preferred size of the widget.
******************************************************************************/
QSize ColorPickerWidget::sizeHint() const
{
    int w = 16;
	int h = fontMetrics().xHeight();

	QStyleOptionButton opt;
	opt.initFrom(this);
	opt.features |= QStyleOptionButton::Flat;
	return (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).expandedTo(QApplication::globalStrut()));
}

/******************************************************************************
* Is called when the user has clicked on the color picker control.
******************************************************************************/
void ColorPickerWidget::activateColorPicker()
{
	QColor newColor = QColorDialog::getColor((QColor)_color, window());
	if(newColor.isValid()) {
		setColor(Color(newColor), true);				
	}
}

};
