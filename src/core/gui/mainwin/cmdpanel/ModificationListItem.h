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

#ifndef __OVITO_MODIFICATION_LIST_ITEM_H
#define __OVITO_MODIFICATION_LIST_ITEM_H

#include <core/Core.h>
#include <core/reference/RefMaker.h>
#include <core/reference/RefTarget.h>
#include <core/scene/pipeline/ModifierApplication.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * An item of the ModificationListModel.
 *
 * Holds a reference to an object/modifier.
 */
class ModificationListItem : public RefMaker
{
public:

	enum Status {
		None,
		Info,
		Warning,
		Error,
		Pending
	};

public:

	/// Constructor.
	ModificationListItem(RefTarget* object, bool isSubObject = false, const QString& title = QString());

	/// Returns the object represented by this list item.
	RefTarget* object() const { return _object; }

	/// Returns the list of modifier applications if this is a modifier item.
	const QVector<ModifierApplication*>& modifierApplications() const { return _modApps; }

	/// Sets the list of modifier applications if this is a modifier item.
	void setModifierApplications(const QVector<ModifierApplication*>& modApps) { _modApps = modApps; }

	/// Returns true if this is a sub-object entry.
	bool isSubObject() const { return _isSubObject; }

	/// Sets whether this is a sub-object entry.
	void setSubObject(bool isSub) { _isSubObject = isSub; }

	/// Returns the status of the object represented by the list item.
	Status status() const;

	/// Returns the title text if this is a section header item.
	const QString title() const { return _title; }

Q_SIGNALS:

	/// This signal is emitted when this item has changed.
	void itemChanged(ModificationListItem* item);

	/// This signal is emitted when the list of sub-items of this item has changed.
	void subitemsChanged(ModificationListItem* parent);

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// The object represented by this item in the list box.
	ReferenceField<RefTarget> _object;

	/// The list of modifier application if this is a modifier item.
	VectorReferenceField<ModifierApplication> _modApps;

	/// Indicates that this is a sub-object entry.
	bool _isSubObject;

	/// Title text if this is a section header item.
	QString _title;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_object);
	DECLARE_VECTOR_REFERENCE_FIELD(_modApps);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_MODIFICATION_LIST_ITEM_H
