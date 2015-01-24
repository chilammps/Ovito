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

#ifndef __OVITO_PARTICLE_SELECTION_SET_H
#define __OVITO_PARTICLE_SELECTION_SET_H

#include <plugins/particles/Particles.h>
#include <core/scene/pipeline/PipelineFlowState.h>
#include <core/reference/RefTarget.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Util)

/**
 * \brief Stores a particle selection set and provides modification functions.
 *
 * This class is used by some modifiers to store the selection state of particles.
 *
 * This selection state can either be stored in an index-based fashion using a bit array,
 * or as a list of particle identifiers. The second storage scheme is less efficient,
 * but supports situations where the order or the number of particles change.
 */
class OVITO_PARTICLES_EXPORT ParticleSelectionSet : public RefTarget
{
public:

	/// Controls the mode of operation of the setParticleSelection() method.
	enum SelectionMode {
		SelectionReplace,		//< Replace the selection with the new selection set.
		SelectionAdd,			//< Add the selection set to the existing selection.
		SelectionSubtract		//< Subtracts the selection set from the existing selection.
	};

public:

	/// Constructor.
	Q_INVOKABLE ParticleSelectionSet(DataSet* dataset) : RefTarget(dataset), _useIdentifiers(true) {
		INIT_PROPERTY_FIELD(ParticleSelectionSet::_useIdentifiers);
	}

	/// Returns the stored selection set as a bit array.
	const QBitArray& selection() const { return _selection; }

	/// Adopts the selection set from the given input state.
	void resetSelection(const PipelineFlowState& state);

	/// Clears the particle selection.
	void clearSelection(const PipelineFlowState& state);

	/// Selects all particles in the given particle data set.
	void selectAll(const PipelineFlowState& state);

	/// Toggles the selection state of a single particle.
	void toggleParticle(const PipelineFlowState& state, size_t particleIndex);

	/// Toggles the selection state of a single particle.
	void toggleParticleIdentifier(int particleId);

	/// Toggles the selection state of a single particle.
	void toggleParticleIndex(size_t particleIndex);

	/// Replaces the particle selection.
	void setParticleSelection(const PipelineFlowState& state, const QBitArray& selection, SelectionMode mode = SelectionReplace);

	/// Copies the stored selection set into the given output selection particle property.
	PipelineStatus applySelection(ParticlePropertyObject* outputSelectionProperty, ParticlePropertyObject* identifierProperty);

	/// Returns true if this object tries to store identifiers of selected particle when available.
	bool useIdentifiers() const { return _useIdentifiers; }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// This helper method determines the number of particles present in the given pipeline state.
	static size_t particleCount(const PipelineFlowState& state);

private:

	/// Stores the selection set as a bit array.
	QBitArray _selection;

	/// Stores the selection as a list of particle identifiers.
	QSet<int> _selectedIdentifiers;

	/// Controls whether the object should store the identifiers of selected particles (when available).
	PropertyField<bool> _useIdentifiers;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_useIdentifiers);

	friend class ReplaceSelectionOperation;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_SELECTION_SET_H
