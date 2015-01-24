///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_STATUS_WIDGET_H
#define __OVITO_STATUS_WIDGET_H

#include <core/Core.h>
#include <core/scene/pipeline/PipelineStatus.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/**
 * \brief A widget that displays information from the PipelineStatus class.
 */
class OVITO_CORE_EXPORT StatusWidget : public QScrollArea
{
	Q_OBJECT
	
public:
	
	/// \brief Constructs the widget.
	/// \param parent The parent widget for the new widget.
	StatusWidget(QWidget* parent = nullptr);

	/// Returns the current status displayed by the widget.
	const PipelineStatus& status() const { return _status; }

	/// Sets the status to be displayed by the widget.
	void setStatus(const PipelineStatus& status);

	/// Resets the widget to not display any status.
	void clearStatus() {
		setStatus(PipelineStatus());
	}

	/// Returns the minimum size of the widget.
	virtual QSize minimumSizeHint() const override;

	/// Returns the preferred size of the widget.
	virtual QSize sizeHint() const override;

private:
	
	/// The current status displayed by the widget.
	PipelineStatus _status;

	/// The internal text label.
	QLabel* _textLabel;

	/// The internal icon label.
	QLabel* _iconLabel;

	/// Status icons:
	QPixmap _statusWarningIcon;
	QPixmap _statusErrorIcon;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_STATUS_WIDGET_H
