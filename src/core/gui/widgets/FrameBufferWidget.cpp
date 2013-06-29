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
#include "FrameBufferWidget.h"

namespace Ovito {

/******************************************************************************
* Sets the FrameBuffer that is currently shown in the widget.
******************************************************************************/
void FrameBufferWidget::setFrameBuffer(const QSharedPointer<FrameBuffer>& newFrameBuffer)
{
	if(newFrameBuffer == frameBuffer()) return;	// Nothing has changed.
	
	_frameBuffer = newFrameBuffer;

	updateScrollBars();
}

/******************************************************************************
* Computes the preferred size of the widget.
******************************************************************************/
QSize FrameBufferWidget::sizeHint() const 
{
	if(_frameBuffer) {
		return _frameBuffer->image().size() + QSize(frameWidth() + 2, frameWidth() + 2);
	}
	return QAbstractScrollArea::sizeHint(); 
} 

/******************************************************************************
* Receive resize events for the viewport widget.
******************************************************************************/
void FrameBufferWidget::resizeEvent(QResizeEvent* event)
{
	updateScrollBars();
}

/******************************************************************************
* Updates the ranges of the scroll bars after the size of the
* frame buffer or the widget have changed.
******************************************************************************/
void FrameBufferWidget::updateScrollBars()
{
	if(frameBuffer()) {
		QSize areaSize = viewport()->size();
		horizontalScrollBar()->setPageStep(40);
		verticalScrollBar()->setPageStep(40);
		horizontalScrollBar()->setRange(0, frameBuffer()->width() - areaSize.width());     
		verticalScrollBar()->setRange(0, frameBuffer()->height() - areaSize.height());
	}
	else {
		horizontalScrollBar()->setRange(0, 0);     
		verticalScrollBar()->setRange(0, 0);
	}
}

/******************************************************************************
* This is called by the system to paint the widgets area.
******************************************************************************/
void FrameBufferWidget::paintEvent(QPaintEvent* event)
{
	if(frameBuffer()) {
		QPainter painter(viewport());
		painter.drawImage(-horizontalScrollBar()->value(), -verticalScrollBar()->value(), frameBuffer()->image());
	}
}


};
