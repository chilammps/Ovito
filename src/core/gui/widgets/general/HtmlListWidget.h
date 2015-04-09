///////////////////////////////////////////////////////////////////////////////
// 
//  Copyright (2015) Alexander Stukowski
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

#ifndef __OVITO_HTML_LIST_WIDGET_H
#define __OVITO_HTML_LIST_WIDGET_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Widgets)

/**
 * \brief A QListWidget that supports HTML text items.
 */
class OVITO_CORE_EXPORT HtmlListWidget : public QListWidget
{
public:
	
	/// \brief Constructs a list widget.
	/// \param parent The parent widget for the new widget.
	HtmlListWidget(QWidget* parent = nullptr);

	/// Returns the recommended size for the widget.
	virtual QSize sizeHint() const override {
		return QSize(320, 200);
	}
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_ELIDED_TEXT_LABEL_H
