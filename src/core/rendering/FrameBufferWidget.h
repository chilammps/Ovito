///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2008) Alexander Stukowski
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

#ifndef __FRAME_BUFFER_WIDGET_H
#define __FRAME_BUFFER_WIDGET_H

#include <core/Core.h>
#include "FrameBuffer.h"

namespace Core {

/******************************************************************************
* This widget displays the contents of a FrameBuffer.
******************************************************************************/
class CORE_DLLEXPORT FrameBufferWidget : public QAbstractScrollArea
{
public:
	/// Constructor.
	FrameBufferWidget(QWidget* parent = NULL) : QAbstractScrollArea(parent) {}

	/// Return the FrameBuffer that is currently shown in the widget (can be NULL).
	const boost::shared_ptr<FrameBuffer>& frameBuffer() const { return _frameBuffer; }
	
	/// Sets the FrameBuffer that is currently shown in the widget.
	void setFrameBuffer(const boost::shared_ptr<FrameBuffer>& frameBuffer);
	
	/// Returns the preferred size of the widget.
	virtual QSize sizeHint() const;

	/// Returns the maximum size of the widget.
	virtual QSize maximumSize() const { return sizeHint(); }
	
protected:

	/// This is called by the system to paint the viewport area.
	virtual void paintEvent(QPaintEvent* event);
	
	/// Receive resize events for the viewport widget.
	void resizeEvent(QResizeEvent* event);

private:

	/// The FrameBuffer that is shown in the widget. 
	boost::shared_ptr<FrameBuffer> _frameBuffer;
	
	/// Updates the ranges of the scrollbars after the size of the framebuffer or the widget have changed.
	void updateScrollBars();

private:
	Q_OBJECT
};

};

#endif // __FRAME_BUFFER_WIDGET_H
