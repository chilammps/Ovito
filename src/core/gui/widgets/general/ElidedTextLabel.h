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
 * \file ElidedTextLabel.h
 * \brief Contains the definition of the Ovito::ElidedTextLabel class.
 */

#ifndef __OVITO_ELIDED_TEXT_LABEL_H
#define __OVITO_ELIDED_TEXT_LABEL_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/**
 * \brief A QLabel-like widget that display a line of text, which is shortened if necessary to fit the available space.
 */
class OVITO_CORE_EXPORT ElidedTextLabel : public QLabel
{
	Q_OBJECT
	
public:
	
	/// \brief Constructs an empty label.
	/// \param parent The parent widget for the new widget.
	/// \param f Passed to the QFrame constructor.
	ElidedTextLabel(QWidget* parent = nullptr, Qt::WindowFlags f = 0) : QLabel(parent, f) {}

	/// \brief Constructs a label with text.
	/// \param text The text string to display.
	/// \param parent The parent widget for the new widget.
	/// \param f Passed to the QFrame constructor.
	ElidedTextLabel(const QString& string, QWidget* parent = nullptr, Qt::WindowFlags f = 0) : QLabel(string, parent, f) {}
	
protected:

	/// Returns the area that is available for us to draw the document
	QRect documentRect() const;

	/// Paints the widget.
	void paintEvent(QPaintEvent *) override;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ELIDED_TEXT_LABEL_H
