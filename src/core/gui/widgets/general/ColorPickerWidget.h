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

/** 
 * \file ColorPickerWidget.h 
 * \brief Contains the definition of the Ovito::ColorPickerWidget class.
 */

#ifndef __OVITO_COLOR_PICKER_WIDGET_H
#define __OVITO_COLOR_PICKER_WIDGET_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/**
 * \brief A UI control lets the user choose a color.
 */
class OVITO_CORE_EXPORT ColorPickerWidget : public QAbstractButton
{
	Q_OBJECT
	
public:
	
	/// \brief Constructs the color picker control.
	/// \param parent The parent widget for the widget.
	ColorPickerWidget(QWidget* parent = 0);

	/// \brief Gets the current value of the color picker.
	/// \return The current color.
	/// \sa setColor()
	const Color& color() const { return _color; }

	/// \brief Sets the current value of the color picker.
	/// \param newVal The new color value.
	/// \param emitChangeSignal Controls whether the control should emit
	///                         a colorChanged() signal when \a newVal is
	///                         not equal to the old color.
	/// \sa color()
	void setColor(const Color& newVal, bool emitChangeSignal = false);
	
	/// Returns the preferred size of the widget.
	virtual QSize sizeHint() const override;

Q_SIGNALS:

	/// \brief This signal is emitted by the color picker after its value has been changed by the user.
	void colorChanged(); 
	
protected Q_SLOTS:

	/// \brief Is called when the user has clicked on the color picker control.
	///
	/// This will open the color selection dialog.
	void activateColorPicker();
	
protected:

	/// Paints the widget.
	virtual void paintEvent(QPaintEvent* event) override;

	/// The currently selected color.
	Color _color; 
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COLOR_PICKER_WIDGET_H
