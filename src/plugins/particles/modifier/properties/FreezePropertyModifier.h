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

#ifndef __OVITO_FREEZE_PROPERTY_MODIFIER_H
#define __OVITO_FREEZE_PROPERTY_MODIFIER_H

#include <plugins/particles/Particles.h>
#include "../ParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Properties)

/**
 * \brief Saves the current state of a particle property and preserves it over time.
 */
class OVITO_PARTICLES_EXPORT FreezePropertyModifier : public ParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE FreezePropertyModifier(DataSet* dataset);

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override { return TimeInterval::infinite(); }

	/// Sets the source particle property which should be preserved.
	void setSourceProperty(const ParticlePropertyReference& prop) { _sourceProperty = prop; }

	/// Returns the source particle property which is preserved.
	const ParticlePropertyReference& sourceProperty() const { return _sourceProperty; }

	/// Sets the particle property to which the stored values should be written.
	void setDestinationProperty(const ParticlePropertyReference& prop) { _destinationProperty = prop; }

	/// Returns the particle property to which the stored values will be written
	const ParticlePropertyReference& destinationProperty() const { return _destinationProperty; }

	/// Takes a snapshot of the source property.
	void takePropertySnapshot(ModifierApplication* modApp, const PipelineFlowState& state);

protected:

	/// This virtual method is called by the modification system when the modifier is being inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipelineObject, ModifierApplication* modApp) override;

	/// Modifies the particle object.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// The particle property that is preserved by this modifier.
	PropertyField<ParticlePropertyReference> _sourceProperty;

	/// The particle property to which the stored values should be written
	PropertyField<ParticlePropertyReference> _destinationProperty;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Freeze property");
	Q_CLASSINFO("ModifierCategory", "Modification");

	DECLARE_PROPERTY_FIELD(_sourceProperty);
	DECLARE_PROPERTY_FIELD(_destinationProperty);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the FreezePropertyModifier class.
 */
class FreezePropertyModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor
	Q_INVOKABLE FreezePropertyModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Takes a new snapshot of the current particle property values.
	void takeSnapshot();

	/// Is called when the user has selected a different source property.
	void onSourcePropertyChanged();

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * Helper class used by the FreezePropertyModifier to store the values of
 * the selected particle property.
 */
class OVITO_PARTICLES_EXPORT SavedParticleProperty : public RefTarget
{
public:

	/// Constructor.
	Q_INVOKABLE SavedParticleProperty(DataSet* dataset) : RefTarget(dataset) {
		INIT_PROPERTY_FIELD(SavedParticleProperty::_property);
		INIT_PROPERTY_FIELD(SavedParticleProperty::_identifiers);
	}

	/// Makes a copy of the given source property and, optionally, of the provided
	/// particle identifier list, which will allow to restore the saved property
	/// values even if the order of particles changes.
	void reset(ParticlePropertyObject* property, ParticlePropertyObject* identifiers);

	/// Returns the stored copy of the particle property.
	ParticlePropertyObject* property() const { return _property; }

	/// Returns the particle identifiers, taken at the time when the property values were saved.
	ParticlePropertyObject* identifiers() const { return _identifiers; }

private:

	/// The stored copy of the particle property.
	ReferenceField<ParticlePropertyObject> _property;

	/// A copy of the particle identifiers, taken at the time when the property values were saved.
	ReferenceField<ParticlePropertyObject> _identifiers;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_property);
	DECLARE_REFERENCE_FIELD(_identifiers);
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_FREEZE_PROPERTY_MODIFIER_H
