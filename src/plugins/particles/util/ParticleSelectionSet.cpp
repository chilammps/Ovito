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

#include <plugins/particles/Particles.h>
#include <core/dataset/UndoStack.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include "ParticleSelectionSet.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ParticleSelectionSet, RefTarget);
DEFINE_PROPERTY_FIELD(ParticleSelectionSet, _useIdentifiers, "UseIdentifiers");

/* Undo record that can restore an old particle selection state. */
class ReplaceSelectionOperation : public UndoableOperation
{
public:
	ReplaceSelectionOperation(ParticleSelectionSet* owner) :
		_owner(owner), _selection(owner->_selection), _selectedIdentifiers(owner->_selectedIdentifiers) {}
	virtual void undo() override {
		_selection.swap(_owner->_selection);
		_selectedIdentifiers.swap(_owner->_selectedIdentifiers);
		_owner->notifyDependents(ReferenceEvent::TargetChanged);
	}
private:
	OORef<ParticleSelectionSet> _owner;
	QBitArray _selection;
	QSet<int> _selectedIdentifiers;
};

/* Undo record that can restore selection state of a single particle. */
class ToggleSelectionOperation : public UndoableOperation
{
public:
	ToggleSelectionOperation(ParticleSelectionSet* owner, int particleId, size_t particleIndex = std::numeric_limits<size_t>::max()) :
		_owner(owner), _particleIndex(particleIndex), _particleId(particleId) {}
	virtual void undo() override {
		if(_particleIndex != std::numeric_limits<size_t>::max())
			_owner->toggleParticleIndex(_particleIndex);
		else
			_owner->toggleParticleIdentifier(_particleId);
	}
private:
	OORef<ParticleSelectionSet> _owner;
	int _particleId;
	size_t _particleIndex;
};

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ParticleSelectionSet::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);
	stream.beginChunk(0x01);
	stream << _selection;
	stream << _selectedIdentifiers;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ParticleSelectionSet::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream >> _selection;
	stream >> _selectedIdentifiers;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> ParticleSelectionSet::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<ParticleSelectionSet> clone = static_object_cast<ParticleSelectionSet>(RefTarget::clone(deepCopy, cloneHelper));
	clone->_selection = this->_selection;
	clone->_selectedIdentifiers = this->_selectedIdentifiers;
	return clone;
}

/******************************************************************************
* Determines the number of particles present in the given pipeline state.
******************************************************************************/
size_t ParticleSelectionSet::particleCount(const PipelineFlowState& state)
{
	// Find the first particle property object to determine the number of particles.
	for(DataObject* o : state.objects()) {
		if(ParticlePropertyObject* particleProperty = dynamic_object_cast<ParticlePropertyObject>(o))
			return particleProperty->size();
	}
	return 0;
}

/******************************************************************************
* Adopts the selection state from the modifier's input.
******************************************************************************/
void ParticleSelectionSet::resetSelection(const PipelineFlowState& state)
{
	// Take a snapshot of the current selection.
	ParticlePropertyObject* selProperty = ParticlePropertyObject::findInState(state, ParticleProperty::SelectionProperty);
	if(selProperty) {

		// Make a backup of the old snapshot so it may be restored.
		if(dataset()->undoStack().isRecording())
			dataset()->undoStack().push(new ReplaceSelectionOperation(this));

		ParticlePropertyObject* identifierProperty = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);
		if(identifierProperty && useIdentifiers()) {
			OVITO_ASSERT(selProperty->size() == identifierProperty->size());
			_selectedIdentifiers.clear();
			_selection.clear();
			const int* s = selProperty->constDataInt();
			for(int id : identifierProperty->constIntRange()) {
				if(*s++)
					_selectedIdentifiers.insert(id);
			}
		}
		else {
			// Take a snapshot of the selection state.
			_selectedIdentifiers.clear();
			_selection.fill(false, selProperty->size());
			const int* s = selProperty->constDataInt();
			const int* s_end = s + selProperty->size();
			for(int index = 0; s != s_end; ++s, index++) {
				if(*s) _selection.setBit(index);
			}
		}

		notifyDependents(ReferenceEvent::TargetChanged);
	}
	else {
		// Reset selection snapshot if input doesn't contain a selection state.
		clearSelection(state);
	}
}

/******************************************************************************
* Clears the particle selection.
******************************************************************************/
void ParticleSelectionSet::clearSelection(const PipelineFlowState& state)
{
	// Make a backup of the old selection state so it may be restored.
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new ReplaceSelectionOperation(this));

	if(useIdentifiers() && ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty)) {
		_selection.clear();
		_selectedIdentifiers.clear();
	}
	else {
		_selection.fill(false, particleCount(state));
		_selectedIdentifiers.clear();
	}
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Replaces the particle selection.
******************************************************************************/
void ParticleSelectionSet::setParticleSelection(const PipelineFlowState& state, const QBitArray& selection, SelectionMode mode)
{
	// Make a backup of the old snapshot so it may be restored.
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new ReplaceSelectionOperation(this));

	ParticlePropertyObject* identifierProperty = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);
	if(identifierProperty && useIdentifiers()) {
		OVITO_ASSERT(selection.size() == identifierProperty->size());
		_selection.clear();
		int index = 0;
		if(mode == SelectionReplace) {
			_selectedIdentifiers.clear();
			for(int id : identifierProperty->constIntRange()) {
				if(selection.testBit(index++))
					_selectedIdentifiers.insert(id);
			}
		}
		else if(mode == SelectionAdd) {
			for(int id : identifierProperty->constIntRange()) {
				if(selection.testBit(index++))
					_selectedIdentifiers.insert(id);
			}
		}
		else if(mode == SelectionSubtract) {
			for(int id : identifierProperty->constIntRange()) {
				if(selection.testBit(index++))
					_selectedIdentifiers.remove(id);
			}
		}
	}
	else {
		_selectedIdentifiers.clear();
		if(mode == SelectionReplace)
			_selection = selection;
		else if(mode == SelectionAdd) {
			_selection.resize(selection.size());
			_selection |= selection;
		}
		else if(mode == SelectionSubtract) {
			_selection.resize(selection.size());
			_selection &= ~selection;
		}
	}

	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Toggles the selection state of a single particle.
******************************************************************************/
void ParticleSelectionSet::toggleParticle(const PipelineFlowState& state, size_t particleIndex)
{
	if(particleIndex >= particleCount(state))
		return;

	ParticlePropertyObject* identifiers = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);
	if(useIdentifiers() && identifiers) {
		_selection.clear();
		toggleParticleIdentifier(identifiers->getInt(particleIndex));
	}
	else if((int)particleIndex < _selection.size()) {
		_selectedIdentifiers.clear();
		toggleParticleIndex(particleIndex);
	}
}

/******************************************************************************
* Toggles the selection state of a single particle.
******************************************************************************/
void ParticleSelectionSet::toggleParticleIdentifier(int particleId)
{
	// Make a backup of the old selection state so it may be restored.
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new ToggleSelectionOperation(this, particleId));

	if(useIdentifiers()) {
		if(_selectedIdentifiers.contains(particleId))
			_selectedIdentifiers.remove(particleId);
		else
			_selectedIdentifiers.insert(particleId);
	}
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Toggles the selection state of a single particle.
******************************************************************************/
void ParticleSelectionSet::toggleParticleIndex(size_t particleIndex)
{
	// Make a backup of the old selection state so it may be restored.
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new ToggleSelectionOperation(this, -1, particleIndex));

	if((int)particleIndex < _selection.size())
		_selection.toggleBit(particleIndex);
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Selects all particles in the given particle data set.
******************************************************************************/
void ParticleSelectionSet::selectAll(const PipelineFlowState& state)
{
	// Make a backup of the old selection state so it may be restored.
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new ReplaceSelectionOperation(this));

	ParticlePropertyObject* identifiers = ParticlePropertyObject::findInState(state, ParticleProperty::IdentifierProperty);
	if(useIdentifiers() && identifiers != nullptr) {
		_selection.clear();
		_selectedIdentifiers.clear();
		for(int id : identifiers->constIntRange())
			_selectedIdentifiers.insert(id);
	}
	else {
		_selection.fill(true, particleCount(state));
		_selectedIdentifiers.clear();
	}
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Copies the stored selection set into the given output selection particle property.
******************************************************************************/
PipelineStatus ParticleSelectionSet::applySelection(ParticlePropertyObject* outputSelectionProperty, ParticlePropertyObject* identifierProperty)
{
	size_t nselected = 0;
	if(!identifierProperty || !useIdentifiers()) {

		// When not using particle identifiers, the number of particles may not change.
		if(outputSelectionProperty->size() != _selection.size())
			return PipelineStatus(PipelineStatus::Error, tr("Cannot apply stored selection state. The number of input particles has changed."));

		// Restore selection simply by placing the snapshot into the pipeline.
		int index = 0;
		for(int& s : outputSelectionProperty->intRange()) {
			if((s = _selection.testBit(index++)))
				nselected++;
		}
	}
	else {
		OVITO_ASSERT(outputSelectionProperty->size() == identifierProperty->size());

		const int* id = identifierProperty->constDataInt();
		for(int& s : outputSelectionProperty->intRange()) {
			if((s = _selectedIdentifiers.contains(*id++)))
				nselected++;
		}
	}
	outputSelectionProperty->changed();

	return PipelineStatus(PipelineStatus::Success, tr("%1 particles selected").arg(nselected));
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
