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
#include <core/reference/RefMaker.h>
#include <core/reference/RefTarget.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/SceneNode.h>

namespace Ovito {

class SceneObject;			// defined in SceneObject.h
class Modifier;				// defined in Modifier.h
class ModifyCommandPage;	// defined in ModifyCommandPage.h
class Rollout;				// defined in RolloutContainer.h
class ModifierStack;		// defined below

/******************************************************************************
* Holds the reference to an object/modifier in the current modifier stack.
******************************************************************************/
class ModifierStackEntry : public RefTarget
{
public:

	/// Constructor.
	ModifierStackEntry(ModifierStack* stack, RefTarget* commonObject, bool isSubObject = false);

	/// Records a mod application if this is a entry for a modifier.
	void addModifierApplication(ModifierApplication* modApp) {
		OVITO_CHECK_OBJECT_POINTER(modApp);
		OVITO_ASSERT(dynamic_object_cast<Modifier>(commonObject()));
		OVITO_ASSERT(_modApps.contains(modApp) == false);
		_modApps.push_back(modApp);
	}

	/// Returns the modification object of this entry.
	/// This can be either a SceneObject or a Modifier.
	RefTarget* commonObject() const { return _object; }

	/// Returns the list of modifier applications if this is a modifier entry.
	const QVector<ModifierApplication*>& modifierApplications() const { return _modApps; }

	/// Returns true if this is a sub-object entry.
	bool isSubObject() const { return _isSubObject; }

	/// Sets whether this is a sub-object entry.
	void setSubObject(bool isSub) { _isSubObject = isSub; }

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// The page that shows the modification stack.
	ModifierStack* _stack;

	/// The object displayed in the list box (either a SceneObject or a Modifier).
	ReferenceField<RefTarget> _object;

	/// The list of application if this is a modifier entry.
	VectorReferenceField<ModifierApplication> _modApps;

	/// Indicates that this is a sub-object entry.
	bool _isSubObject;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_object);
	DECLARE_VECTOR_REFERENCE_FIELD(_modApps);
};

/******************************************************************************
* This model class is used to populate the QListView widget.
******************************************************************************/
class ModifierStackModel : public QAbstractListModel
{
	Q_OBJECT

public:

	enum EntryStatus {
		None,
		Enabled,
		Disabled,
		Info,
		Warning,
		Error,
		Pending
	};

public:

	/// Constructor.
	ModifierStackModel(QObject* parent) : QAbstractListModel(parent),
		_statusInfoIcon(":/core/mainwin/status/status_info.png"),
		_statusWarningIcon(":/core/mainwin/status/status_warning.png"),
		_statusErrorIcon(":/core/mainwin/status/status_error.png"),
		_statusPendingIcon(":/core/mainwin/status/status_pending.gif"),
		_modifierEnabledIcon(":/core/command_panel/modifier_enabled.png"),
		_modifierDisabledIcon(":/core/command_panel/modifier_disabled.png") {
		connect(&_statusPendingIcon, SIGNAL(frameChanged(int)), this, SLOT(iconFrameChanged()));
	}

	/// Returns the number of list rows.
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override { return _entries.size(); }

	/// Returns the data associated with a list entry.
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	/// Discards all modifier stack entries and resets the list.
	void clear() {
		if(_entries.empty()) return;
		beginRemoveRows(QModelIndex(), 0, _entries.size()-1);
		_entries.clear();
		endRemoveRows();
	}

	/// Populates the list model with the given modifier stack entries.
	void setEntries(const QVector<ModifierStackEntry*>& newEntries) {
		clear();
		if(newEntries.empty()) return;
		beginInsertRows(QModelIndex(), 0, newEntries.size()-1);
		_entries = newEntries;
		endInsertRows();
	}

	/// Updates the display of a single modifier stack entry.
	void refreshStackEntry(ModifierStackEntry* entry);

	/// Returns the modifier stack that owns this list model.
	ModifierStack* stack() { return reinterpret_cast<ModifierStack*>(QObject::parent()); }

private Q_SLOTS:

	/// Is called by the system when the animated status icon changed.
	void iconFrameChanged();

private:

	/// Returns the status for a given entry of the modifier stack.
	EntryStatus getEntryStatus(ModifierStackEntry* entry) const;

	/// Returns the tooltip text for a given entry of the modifier stack.
	QVariant getEntryToolTip(ModifierStackEntry* entry) const;

	QVector<ModifierStackEntry*> _entries;
	QIcon _modifierEnabledIcon;
	QIcon _modifierDisabledIcon;
	QIcon _statusInfoIcon;
	QIcon _statusWarningIcon;
	QIcon _statusErrorIcon;
	QMovie _statusPendingIcon;
};

/******************************************************************************
* Manages the modifier stack of the selected object(s).
******************************************************************************/
class ModifierStack : public RefMaker
{
public:

	/// Initializes the object.
	ModifierStack(ModifyCommandPage* owner);

	/// Clears the modification stack.
	void clearStack();

	/// Completely rebuilds the modifier stack list.
	void refreshModifierStack();

	/// Updates a single entry in the modifier stack list box.
	void refreshStackEntry(ModifierStackEntry* entry);

	/// Shows the properties of the selected item in the modifier stack box in the
	/// properties panel of the page.
	void updatePropertiesPanel();

	/// Updates the list box of modifier classes that can be applied to the current selected
	/// item in the modifier stack.
	void updateAvailableModifiers(ModifierStackEntry* currentEntry);

	/// Updates the state of the actions that can be invoked on the currently selected
	/// item in the modifier stack.
	void updateAvailableActions(ModifierStackEntry* currentEntry);

    /// Inserts the given modifier into the modification stack of the selected scene nodes.
	void applyModifier(Modifier* modifier);

	/// Returns the internal list model that can be used to populate a QListView widget.
	ModifierStackModel* listModel() const { return _listModel; }

	/// Returns the currently selected modifier stack entry (or NULL).
	ModifierStackEntry* selectedEntry() const;

	/// This invalidates the current modifier stack and rebuilts it as soon as possible.
	void invalidate() {
		if(needStackUpdate) return;
		needStackUpdate = true;
		internalStackUpdate();
	}

	/// Resets the internal invalidation flag.
	void validate() { needStackUpdate = false; }

	/// Returns true if the modifier stack ist currently in a valid state.
	bool isValid() const { return !needStackUpdate; }

	/// A category of modifiers.
	/// This struct must be public because it is accessed by the std::sort() function.
	struct ModifierCategory {
		QString id;
		QString label;
		QVector<const OvitoObjectType*> modifierClasses;
	};

Q_SIGNALS:

	/// This internal signal is emmited for deferred stack update.
	void internalStackUpdate();

protected Q_SLOTS:

	/// Handles the internalStackUpdate signal.
	void onInternalStackUpdate() {
		if(needStackUpdate) {
			needStackUpdate = false;
			refreshModifierStack();
		}
	}

private:

	/// Filters the given input list for ObjectNodes.
	void collectObjectNodes(const QVector<SceneNode*>& in);

	/// If all selected object nodes reference the same SceneObject
	/// then it is returned by this method; otherwise NULL is returned.
	SceneObject* commonObject();

	/// Gathers all defined modifier categories from the plugin manifests.
	void loadModifierCategories();

	/// Defines a comparison operator for the modifier list.
	static bool modifierOrdering(const OvitoObjectType* a, const OvitoObjectType* b);

	/// Defines a comparison operator for the modifier category list.
	static bool modifierCategoryOrdering(const ModifierCategory& a, const ModifierCategory& b);

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	/// The page that shows the modification stack.
	ModifyCommandPage* page;

	/// The list of modifications common to the selected scene nodes.
	VectorReferenceField<ModifierStackEntry> stackEntries;

	/// The ObjectNodes from the current selection set.
	VectorReferenceField<ObjectNode> selectedNodes;

	/// The entry in the modification stack that should be selected on the next update.
	RefTarget* nextObjectToSelect;

	/// The model for the list view widget.
	ModifierStackModel* _listModel;

	/// Indicates that the displayed modification stack needs to be updated.
	bool needStackUpdate;

	/// List of modifier categories.
	QVector<ModifierCategory> _modifierCategories;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_VECTOR_REFERENCE_FIELD(stackEntries);
	DECLARE_VECTOR_REFERENCE_FIELD(selectedNodes);
};

};
