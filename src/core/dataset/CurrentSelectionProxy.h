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
 * \file CurrentSelectionProxy.h 
 * \brief Contains the definition of the Ovito::CurrentSelectionProxy class.
 */

#ifndef __OVITO_CURRENT_SELECTION_PROXY_H
#define __OVITO_CURRENT_SELECTION_PROXY_H

#include <core/Core.h>
#include <core/scene/SelectionSet.h>

namespace Ovito {


/**
 * \brief Proxy for the current SelectionSet in the current DataSet.
 * 
 * The nodes contained in the current selection set of the current dataset will be
 * mirrored in this selection set proxy.
 * 
 * \note This is an internal class that is not for public use.
 */
class OVITO_CORE_EXPORT CurrentSelectionProxy : public SelectionSet
{
public:

	/// Default constructor.
	CurrentSelectionProxy();

	/// Returns the number of scene nodes in the selection set.
	virtual int count() const override;

	/// Returns a scene node from the selection set.
    virtual SceneNode* node(int index) const override;

	/// Returns true if the given scene node is part of the selection set.
	virtual bool contains(SceneNode* node) const override;

	/// Adds a scene node to this selection set. 
    virtual void add(SceneNode* node) override;

	/// Adds multiple scene nodes to this selection set. 
    virtual void addAll(const QVector<SceneNode*>& nodes) override;

	/// Removes a scene node from this selection set. 
    virtual void remove(SceneNode* node) override;

	/// Clears the selection.
	virtual void clear() override;

	/// Returns the bounding box that includes all selected nodes.
	virtual Box3 boundingBox(TimePoint time) override;

	/// Returns all nodes that are selected.
	virtual const QVector<SceneNode*>& nodes() const override;

	/// Sets the contents of the selection set.
	virtual void setNodes(const QVector<SceneNode*>& nodes) override;
	
	/// Resets the selection set to contain only a single node. 
	virtual void setNode(SceneNode* node) override;
	
	/// Gets the selection set which this proxy encapsulates.
	SelectionSet* currentSelectionSet() const { return _selectionSet; }

	/// Sets the selection set which this proxy encapsulates.
	void setCurrentSelectionSet(SelectionSet* set) { _selectionSet = set; onChanged(); }

protected:

	/// From RefMaker.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the selection set has changed.
	/// Raises the relectionChanged signal of the DataSetManager.
	void onChanged();
	
Q_SIGNALS:

	/// This is only an internal signal.
	void internalSelectionChanged();
	
protected Q_SLOTS:

	/// Is called after the selection set has changed multiple times.
	void onInternalSelectionChanged();
	
private:

	/// Holds the references to the current selection set in the current data set.
	ReferenceField<SelectionSet> _selectionSet;
	
	/// Indicates that there is a pending change event in the event queue.
	bool _changeEventInQueue;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_selectionSet)
};

};

#endif // __OVITO_CURRENT_SELECTION_PROXY_H
