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

#ifndef __OVITO_MODIFIER_LIST_BOX_H
#define __OVITO_MODIFIER_LIST_BOX_H

#include <core/Core.h>
#include <core/object/OvitoObjectType.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

class ModificationListModel;
class ModificationListItem;

/**
 * A combo-box widget that lets the user insert new modifiers into the modification pipeline.
 */
class ModifierListBox : public QComboBox
{
public:

	/// Initializes the widget.
	ModifierListBox(QWidget* parent, ModificationListModel* modificationList);

	/// Is called just before the drop-down box is activated.
	virtual void showPopup() override {
		updateAvailableModifiers();
		QComboBox::showPopup();
	}

private Q_SLOTS:

	/// Updates the list box of modifier classes that can be applied to the current selected
	/// item in the modification list.
	void updateAvailableModifiers();

private:

	/// A category of modifiers.
	struct ModifierCategory {
		QString name;
		QVector<const OvitoObjectType*> modifierClasses;
	};

	/// List of modifier categories.
	QVector<ModifierCategory> _modifierCategories;

	/// The modification list model.
	ModificationListModel* _modificationList;

	/// The font to be used for category headers.
	QFont _categoryFont;

	/// The background brush to be used for category headers.
	QBrush _categoryBackgroundBrush;

	/// The foreground brush to be used for category headers.
	QBrush _categoryForegroundBrush;

	Q_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_MODIFIER_LIST_BOX_H
