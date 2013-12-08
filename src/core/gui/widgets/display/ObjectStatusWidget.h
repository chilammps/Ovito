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
 * \file ObjectStatusWidget.h
 * \brief Contains the definition of the Ovito::ObjectStatusWidget class.
 */

#ifndef __OVITO_OBJECT_STATUS_WIDGET_H
#define __OVITO_OBJECT_STATUS_WIDGET_H

#include <core/Core.h>
#include <core/utilities/ObjectStatus.h>

namespace Ovito {

/**
 * \brief A widget that displays the information contained in the ObjectStatus class.
 */
class OVITO_CORE_EXPORT ObjectStatusWidget : public QScrollArea
{
	Q_OBJECT
	
public:
	
	/// \brief Constructs the widget.
	/// \param parent The parent widget for the new widget.
	ObjectStatusWidget(QWidget* parent = nullptr);

	/// Returns the current status displayed by the widget.
	const ObjectStatus& status() const { return _status; }

	/// Sets the status displayed by the widget.
	void setStatus(const ObjectStatus& status);

	/// Resets the widget to not display any status.
	void clearStatus() {
		setStatus(ObjectStatus());
	}

	/// Returns the minimum size of the widget.
	virtual QSize minimumSizeHint() const override;

	/// Returns the preferred size of the widget.
	virtual QSize sizeHint() const override;

private:
	
	/// The current status displayed by the widget.
	ObjectStatus _status;

	/// The internal text label.
	QLabel* _textLabel;

	/// The internal icon label.
	QLabel* _iconLabel;

	/// Status icons:
	QPixmap _statusWarningIcon;
	QPixmap _statusErrorIcon;
};

};

#endif // __OVITO_OBJECT_STATUS_WIDGET_H
