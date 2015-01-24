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
#include <core/gui/mainwin/MainWindow.h>
#include "SpinnerWidget.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/******************************************************************************
* Constructs the spinner.
******************************************************************************/
SpinnerWidget::SpinnerWidget(QWidget* parent, QLineEdit* textBox) : QWidget(parent),
	_textBox(NULL), _value(0), _minValue(FLOATTYPE_MIN), _maxValue(FLOATTYPE_MAX),
	_upperBtnPressed(false), _lowerBtnPressed(false), _unit(nullptr)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum, QSizePolicy::SpinBox));
	setTextBox(textBox);
}

/******************************************************************************
* Paint event handler.
******************************************************************************/
void SpinnerWidget::paintEvent(QPaintEvent* event)
{
	QStylePainter p(this);
    QStyleOptionSpinBox sboption;
    
    sboption.initFrom(this);
    sboption.state |= _upperBtnPressed ? QStyle::State_Sunken : QStyle::State_Raised;
    sboption.rect.setHeight(sboption.rect.height() / 2);
    p.drawPrimitive(QStyle::PE_PanelButtonTool, sboption);
    p.drawPrimitive(QStyle::PE_IndicatorSpinUp, sboption);

    sboption.initFrom(this);
    sboption.state |= _lowerBtnPressed ? QStyle::State_Sunken : QStyle::State_Raised;
    sboption.rect.setTop(sboption.rect.top() + sboption.rect.height() / 2);
    p.drawPrimitive(QStyle::PE_PanelButtonTool, sboption);
    p.drawPrimitive(QStyle::PE_IndicatorSpinDown, sboption);
}

/******************************************************************************
* Returns the natural size of the spinner widget.
******************************************************************************/
QSize SpinnerWidget::sizeHint() const
{
	if(textBox() == nullptr) return QSize(16, 30);
	else return QSize(16, textBox()->sizeHint().height());	
}

/******************************************************************************
* Connects this spinner with the given textbox control.
******************************************************************************/
void SpinnerWidget::setTextBox(QLineEdit* box)
{
	if(box == textBox()) return;
	if(_textBox.isNull() == false) {
		disconnect(_textBox.data(), &QLineEdit::editingFinished, this, &SpinnerWidget::onTextChanged);
	}
	_textBox = box;
	if(_textBox.isNull() == false) {
		connect(box, &QLineEdit::editingFinished, this, &SpinnerWidget::onTextChanged);
		box->setEnabled(isEnabled());
		updateTextBox();
	}
}

/******************************************************************************
* Will be called when the user has entered a new text into the text box.
* The text will be parsed and taken as the new value of the spinner.
******************************************************************************/
void SpinnerWidget::onTextChanged()
{
	OVITO_CHECK_POINTER(textBox());
    
	FloatType newValue;
	try {
		if(textBox()->text() == _originalText) return;
		if(unit()) {
			newValue = unit()->parseString(textBox()->text());
			setFloatValue(unit()->userToNative(newValue), true);
		}
		else {
			bool ok;
			newValue = textBox()->text().toDouble(&ok);
			if(!ok)
				throw Exception(tr("Invalid floating-point value: %1").arg(textBox()->text()));
			setFloatValue(newValue, true);
		}
	}
	catch(const Exception&) {
		// Ignore invalid value and restore old text content.
		updateTextBox();
	}	
}

/******************************************************************************
* Updates the text of the connected text box after the spinner's value has changed.
******************************************************************************/
void SpinnerWidget::updateTextBox()
{
	if(textBox()) {
		if(unit())
			_originalText = unit()->formatValue(unit()->nativeToUser(floatValue()));
		else
			_originalText = QString::number(floatValue());
		textBox()->setText(_originalText);
	}
}

/******************************************************************************
* Sets the current value of the spinner.
******************************************************************************/
void SpinnerWidget::setFloatValue(FloatType newVal, bool emitChangeSignal)
{
	// Clamp value.
	if(newVal == _value) return;
	newVal = std::max(minValue(), newVal);
	newVal = std::min(maxValue(), newVal);
	if(_value != newVal) {
		_value = newVal;
		if(emitChangeSignal)
			Q_EMIT spinnerValueChanged();
	}
	updateTextBox();
}

/******************************************************************************
* Sets the current integer value of the spinner.
******************************************************************************/
void SpinnerWidget::setIntValue(int newValInt, bool emitChangeSignal)
{
	FloatType newVal = (FloatType)newValInt;

	if(newVal == _value) return;
	// Clamp value.
	newVal = std::max((FloatType)ceil(minValue()), newVal);
	newVal = std::min((FloatType)floor(maxValue()), newVal);
	if(_value != newVal) {
		_value = newVal;
		if(emitChangeSignal)
			Q_EMIT spinnerValueChanged();
	}
	updateTextBox();
}

/******************************************************************************
* Sets the minimum allowed value of the spinner.
* If the current value of the spinner is less than the new minimum value,
* it will be set to the new minimum value.
******************************************************************************/
void SpinnerWidget::setMinValue(FloatType minValue)
{
	_minValue = minValue;
}

/******************************************************************************
* Sets the maximum allowed value of the spinner.
* If the current value of the spinner is greater than the new maximum value,
* it will be set to the new maximum value.
******************************************************************************/
void SpinnerWidget::setMaxValue(FloatType maxValue)
{
	_maxValue = maxValue;
}

/******************************************************************************
* Sets the units of this spinner's value.
******************************************************************************/
void SpinnerWidget::setUnit(ParameterUnit* unit) 
{ 
	if(unit == _unit)
		return;

	if(_unit)
		disconnect(_unit, &ParameterUnit::formatChanged, this, &SpinnerWidget::updateTextBox);
	
	_unit = unit; 
	
	if(_unit)
		connect(_unit, &ParameterUnit::formatChanged, this, &SpinnerWidget::updateTextBox);

	updateTextBox(); 
}

/******************************************************************************
* Handles the change events for the spinner.
******************************************************************************/
void SpinnerWidget::changeEvent(QEvent* event)
{
	QWidget::changeEvent(event);
	if(event->type() == QEvent::EnabledChange) {
		if(textBox())
			textBox()->setEnabled(isEnabled());
	}
}

/******************************************************************************
* Handles the mouse down event.
******************************************************************************/
void SpinnerWidget::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton && !_upperBtnPressed && !_lowerBtnPressed) {
		// Backup current value.
		_oldValue = floatValue();

		OVITO_ASSERT(_lowerBtnPressed == false && _upperBtnPressed == false);

		if(event->y() <= height()/2)
			_upperBtnPressed = true;
		else
			_lowerBtnPressed = true;

		_currentStepSize = unit() ? unit()->stepSize(floatValue(), _upperBtnPressed) : 1;
		if(textBox()) textBox()->setFocus(Qt::OtherFocusReason);
		
		grabMouse();
		repaint();
	}
	else if(event->button() == Qt::RightButton) {
		
		// restore old value
		setFloatValue(_oldValue, true);

		if(_upperBtnPressed == _lowerBtnPressed) {
			Q_EMIT spinnerDragAbort();
		}

		_upperBtnPressed = false;
		_lowerBtnPressed = false;

		releaseMouse();
		update();
	}	
}

/******************************************************************************
* Handles the mouse up event.
******************************************************************************/
void SpinnerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if(_upperBtnPressed || _lowerBtnPressed) {
		if(_upperBtnPressed == _lowerBtnPressed) {
			Q_EMIT spinnerDragStop();
		}
		else {
			FloatType newValue;
			if(_upperBtnPressed) {
				if(unit())
					newValue = unit()->roundValue(floatValue() + unit()->stepSize(floatValue(), true));
				else
					newValue = floatValue() + 1.0f;
			}
			else {
				if(unit())
					newValue = unit()->roundValue(floatValue() - unit()->stepSize(floatValue(), false));
				else
					newValue = floatValue() - 1.0f;
			}
			setFloatValue(newValue, true);
		}

		_upperBtnPressed = false;
		_lowerBtnPressed = false;

		// Repaint spinner.
		update();
	}
	releaseMouse();
}

/******************************************************************************
* Handles the mouse move event.
******************************************************************************/
void SpinnerWidget::mouseMoveEvent(QMouseEvent* event)
{
	if(_upperBtnPressed || _lowerBtnPressed) {
		if(_upperBtnPressed && !_lowerBtnPressed) {
			if(event->y() > height()/2 || event->y() < 0) {
				_lowerBtnPressed = true;
				_lastMouseY = _startMouseY = mapToGlobal(event->pos()).y();
				update();
				Q_EMIT spinnerDragStart();
			}
		}
		else if(!_upperBtnPressed && _lowerBtnPressed) {
			if(event->y() <= height()/2 || event->y() > height()) {
				_upperBtnPressed = true;
				_lastMouseY = _startMouseY = mapToGlobal(event->pos()).y();
				update();
				Q_EMIT spinnerDragStart();
			}
		}
		else { 
			QPoint cursorPos = QCursor::pos();
			int screenY = cursorPos.y();
			if(screenY != _lastMouseY) {
				int screenHeight = QApplication::desktop()->screenGeometry().height();
				if(screenY <= 5 && _lastMouseY == screenHeight-1) return;
				if(screenY >= screenHeight - 5 && _lastMouseY == 0) return;
				
				FloatType newVal = _oldValue + _currentStepSize * (FloatType)(_startMouseY - screenY) * 0.1f;
				if(unit())
					newVal = unit()->roundValue(newVal);
	
				if(screenY < _lastMouseY && screenY <= 5) {
					_lastMouseY = screenHeight-1;
					_startMouseY += _lastMouseY - screenY;
					QCursor::setPos(cursorPos.x(), _lastMouseY);
				}
				else if(screenY > _lastMouseY && screenY >= screenHeight - 5) {
					_lastMouseY = 0;
					_startMouseY += _lastMouseY - screenY;
					QCursor::setPos(cursorPos.x(), _lastMouseY);
				}
				else _lastMouseY = screenY;

				if(newVal != floatValue()) {
					setFloatValue(newVal, true);

					// Repaint viewports for immediate visual feedback when changing a parameter.
					if(MainWindow* mainWindow = qobject_cast<MainWindow*>(window()))
						mainWindow->processViewportUpdates();

					// Also repaint text box for immediate visual updates.
					if(textBox())
						textBox()->repaint();
				}
			}
		}
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
