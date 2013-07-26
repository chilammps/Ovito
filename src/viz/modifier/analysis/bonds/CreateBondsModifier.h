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
 * \file CreateBondsModifier.h
 * \brief Contains the definition of the Viz::CreateBondsModifier class.
 */

#ifndef __OVITO_CREATE_BONDS_MODIFIER_H
#define __OVITO_CREATE_BONDS_MODIFIER_H

#include <core/Core.h>
#include <viz/data/BondsDisplay.h>
#include "../../AsynchronousParticleModifier.h"

namespace Viz {

using namespace Ovito;

/**
 * \brief A modifier that creates bonds between pairs of particles based on their distance.
 */
class CreateBondsModifier : public AsynchronousParticleModifier
{
public:

	/// Engine that determines the bonds between particles.
	class BondGenerationEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		BondGenerationEngine(ParticleProperty* positions, const SimulationCellData& simCell, FloatType cutoff) :
			_positions(positions), _simCell(simCell), _cutoff(cutoff),
			_bonds(new BondsStorage()) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

		/// Returns the generated bonds.
		BondsStorage* bonds() { return _bonds.data(); }

		/// Returns the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

	private:

		FloatType _cutoff;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<BondsStorage> _bonds;
		SimulationCellData _simCell;
	};

public:

	/// Default constructor.
	Q_INVOKABLE CreateBondsModifier();

	/// \brief Returns the cutoff radius used to determine which particles are bonded.
	/// \return The cutoff radius in world units.
	/// \sa setCutoff()
	FloatType cutoff() const { return _cutoff; }

	/// \brief Sets the cutoff radius that is used for generating bonds.
	/// \param newCutoff The new cutoff radius in world units.
	/// \undoable
	/// \sa cutoff()
	void setCutoff(FloatType newCutoff) { _cutoff = newCutoff; }

	/// \brief Returns the display object that is responsible for rendering the bonds.
	BondsDisplay* bondsDisplay() const { return _bondsDisplay; }

	/// \brief Returns the scene object that stores the generated bonds.
	BondsObject* bondsObject() const { return _bondsObj; }

public:

	Q_PROPERTY(FloatType cutoff READ cutoff WRITE setCutoff)

protected:

	/// Handles reference events sent by reference targets of this object.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Resets the modifier's result cache.
	virtual void invalidateCachedResults() override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// This lets the modifier insert the previously computed results into the pipeline.
	virtual ObjectStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

	/// The cutoff radius for bond generation.
	PropertyField<FloatType> _cutoff;

	/// The display object for rendering the bonds.
	ReferenceField<BondsDisplay> _bondsDisplay;

	/// This stores the cached results of the modifier, i.e. the bonds information.
	ReferenceField<BondsObject> _bondsObj;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Create bonds");
	Q_CLASSINFO("ModifierCategory", "Modify");

	DECLARE_PROPERTY_FIELD(_cutoff);
	DECLARE_REFERENCE_FIELD(_bondsDisplay);
	DECLARE_REFERENCE_FIELD(_bondsObj);
};

/**
 * \brief A properties editor for the CreateBondsModifier class.
 */
class CreateBondsModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CreateBondsModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Stores the current cutoff radius in the application settings
	/// so it can be used as default value for new modifiers in the future.
	void memorizeCutoff();

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_CREATE_BONDS_MODIFIER_H
