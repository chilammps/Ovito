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

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

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
		disconnect(_frameBuffer.data(), &FrameBuffer::contentChanged, this, &FrameBufferWidget::onFrameBufferContentChanged);
		disconnect(_frameBuffer.data(), &FrameBuffer::contentReset, this, &FrameBufferWidget::onFrameBufferContentReset);
	}
	
	_frameBuffer = newFrameBuffer;

	onFrameBufferContentReset();

	connect(_frameBuffer.data(), &FrameBuffer::contentChanged, this, &FrameBufferWidget::onFrameBufferContentChanged);
	connect(_frameBuffer.data(), &FrameBuffer::contentReset, this, &FrameBufferWidget::onFrameBufferContentReset);
}

/******************************************************************************
* Computes the preferred size of the widget.
******************************************************************************/
QSize FrameBufferWidget::sizeHint() const 
{
	if(_frameBuffer) {
		return _frameBuffer->size();
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

/******************************************************************************
* This handles contentReset() signals from the frame buffer.
******************************************************************************/
void FrameBufferWidget::onFrameBufferContentReset()
{
	// Resize widget.
	if(_frameBuffer) {
		resize(_frameBuffer->size());
		for(QWidget* w = parentWidget(); w != nullptr; w = w->parentWidget()) {
			// Size hint of scroll area has changed. Update its geometry.
			if(qobject_cast<QScrollArea*>(w)) {
				w->updateGeometry();
				break;
			}
		}
	}

	// Repaint the widget.
	update();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
