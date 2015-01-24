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

#ifndef __OVITO_FRAME_BUFFER_WIDGET_H
#define __OVITO_FRAME_BUFFER_WIDGET_H

#include <core/Core.h>
#include <core/rendering/FrameBuffer.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/**
 * This widget displays the contents of a FrameBuffer.
 */
class OVITO_CORE_EXPORT FrameBufferWidget : public QWidget
{
public:

	/// Constructor.
	FrameBufferWidget(QWidget* parent = nullptr) : QWidget(parent) {}

	/// Return the FrameBuffer that is currently shown in the widget (can be NULL).
	const QSharedPointer<FrameBuffer>& frameBuffer() const { return _frameBuffer; }
	
	/// Sets the FrameBuffer that is currently shown in the widget.
	void setFrameBuffer(const QSharedPointer<FrameBuffer>& frameBuffer);
	
	/// Returns the preferred size of the widget.
	virtual QSize sizeHint() const override;

protected:

	/// This is called by the system to paint the viewport area.
	virtual void paintEvent(QPaintEvent* event) override;
	
private Q_SLOTS:

	/// This handles contentChanged() signals from the frame buffer.
	void onFrameBufferContentChanged(QRect changedRegion) {
		// Repaint only portion of the widget.
		update(changedRegion);
	}

	/// This handles contentReset() signals from the frame buffer.
	void onFrameBufferContentReset();

private:

	/// The FrameBuffer that is shown in the widget. 
	QSharedPointer<FrameBuffer> _frameBuffer;

private:

	Q_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FRAME_BUFFER_WIDGET_H
