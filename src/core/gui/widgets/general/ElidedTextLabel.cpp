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
#include "ElidedTextLabel.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/******************************************************************************
* Returns the rect that is available for us to draw the document
******************************************************************************/
QRect ElidedTextLabel::documentRect() const
{
	QRect cr = contentsRect();
	int m = margin();
	cr.adjust(m, m, -m, -m);
	m = indent();
    if(m < 0 && frameWidth()) // no indent, but we do have a frame
    	m = fontMetrics().width(QLatin1Char('x')) / 2 - margin();
    int align = QStyle::visualAlignment(layoutDirection(), alignment());
    if(m > 0) {
		if (align & Qt::AlignLeft)
            cr.setLeft(cr.left() + m);
        if (align & Qt::AlignRight)
            cr.setRight(cr.right() - m);
        if (align & Qt::AlignTop)
            cr.setTop(cr.top() + m);
        if (align & Qt::AlignBottom)
            cr.setBottom(cr.bottom() - m);
    }
    return cr;
}

/******************************************************************************
* Paints the widget.
******************************************************************************/
void ElidedTextLabel::paintEvent(QPaintEvent *)
{
    QStyle *style = QWidget::style();
    QPainter painter(this);
    QRect cr = documentRect();
    int flags = QStyle::visualAlignment(layoutDirection(), alignment());
    QString elidedText = painter.fontMetrics().elidedText(text(), Qt::ElideLeft, cr.width(), flags);
    style->drawItemText(&painter, cr, flags, palette(), isEnabled(), elidedText, foregroundRole());

    // Use the label's full text as tool tip.
    if(toolTip() != text())
    	setToolTip(text());
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
