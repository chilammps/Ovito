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
	if(newFrameBuffer == frameBuffer()) {
		onFrameBufferContentReset();
		return;
	}

	if(frameBuffer()) {
		disconnect(_frameBuffer.data(), SIGNAL(contentChanged(QRect)), this, SLOT(onFrameBufferContentChanged(QRect)));
		disconnect(_frameBuffer.data(), SIGNAL(contentReset()), this, SLOT(update()));
	}
	
	_frameBuffer = newFrameBuffer;

	onFrameBufferContentReset();

	connect(_frameBuffer.data(), SIGNAL(contentChanged(QRect)), this, SLOT(onFrameBufferContentChanged(QRect)));
	connect(_frameBuffer.data(), SIGNAL(contentReset()), this, SLOT(onFrameBufferContentReset()));
}

/******************************************************************************
* Computes the preferred size of the widget.
******************************************************************************/
QSize FrameBufferWidget::sizeHint() const 
{
	if(_frameBuffer) {
		return _frameBuffer->image().size();
	}
	return QWidget::sizeHint();
} 

/******************************************************************************
* This is called by the system to paint the widgets area.
******************************************************************************/
void FrameBufferWidget::paintEvent(QPaintEvent* event)
{
	if(frameBuffer()) {
		QPainter painter(this);
		painter.drawImage(0, 0, frameBuffer()->image());
	}
}


};
