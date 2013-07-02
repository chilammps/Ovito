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

#ifndef __OVITO_PIPELINE_EDITOR_H
#define __OVITO_PIPELINE_EDITOR_H

#include <core/Core.h>
#include <core/reference/RefMaker.h>
#include <core/reference/RefTarget.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/SceneNode.h>

namespace Ovito {

class SceneObject;			// defined in SceneObject.h
class Modifier;				// defined in Modifier.h
class ModifyCommandPage;	// defined in ModifyCommandPage.h

/*
 * Displays the modification pipeline of the selected object(s).
 */
class PipelineEditor : public RefMaker
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

	/// Rebuilds the modifier stack as soon as possible.
	void updateLater() {
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

#endif // __OVITO_PIPELINE_EDITOR_H
