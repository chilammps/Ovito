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
 * \file CentroSymmetryModifier.h
 * \brief Contains the definition of the Particles::CentroSymmetryModifier class.
 */

#ifndef __OVITO_CENTRO_SYMMETRY_MODIFIER_H
#define __OVITO_CENTRO_SYMMETRY_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/RefTargetListParameterUI.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>

namespace Particles {

using namespace Ovito;

class TreeNeighborListBuilder;

/**
 * \brief Calculates the centro-symmetry parameter (CSP) for particles.
 */
class OVITO_PARTICLES_EXPORT CentroSymmetryModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE CentroSymmetryModifier(DataSet* dataset);

	/// Returns the computed per-particle CSP values.
	const ParticleProperty& cspValues() const { OVITO_CHECK_POINTER(_cspValues.constData()); return *_cspValues; }

	/// Returns the number of nearest neighbors to take into account when computing the CSP.
	int numNeighbors() const { return _numNeighbors; }

	/// Sets the number of nearest neighbors to take into account when computing the CSP.
	void setNumNeighbors(int count) { _numNeighbors = count; }

public:

	Q_PROPERTY(int numNeighbors READ numNeighbors WRITE setNumNeighbors)

private:

	/// Computes the modifier's results.
	class CentroSymmetryEngine : public AsynchronousParticleModifier::Engine
	{
	public:

		/// Constructor.
		CentroSymmetryEngine(ParticleProperty* positions, const SimulationCellData& simCell, int nneighbors) :
			_nneighbors(nneighbors),
			_positions(positions),
			_simCell(simCell),
			_csp(new ParticleProperty(positions->size(), ParticleProperty::CentroSymmetryProperty)) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void compute(FutureInterfaceBase& futureInterface) override;

		/// Returns the property storage that contains the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the property storage that contains the computed per-particle CSP values.
		ParticleProperty* csp() const { return _csp.data(); }

		/// Returns the simulation cell data.
		const SimulationCellData& cell() const { return _simCell; }

	private:

		int _nneighbors;
		SimulationCellData _simCell;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _csp;
	};

protected:

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates and initializes a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<Engine> createEngine(TimePoint time, TimeInterval& validityInterval) override;

	/// Unpacks the computation results stored in the given engine object.
	virtual void retrieveModifierResults(Engine* engine) override;

	/// This lets the modifier insert the previously computed results into the pipeline.
	virtual ObjectStatus applyModifierResults(TimePoint time, TimeInterval& validityInterval) override;

	/// Computes the centrosymmetry parameter of a single particle.
	static FloatType computeCSP(TreeNeighborListBuilder& neighList, size_t particleIndex);

private:

	/// This stores the cached results of the modifier, i.e. the CSP values computed for the particles.
	QExplicitlySharedDataPointer<ParticleProperty> _cspValues;

	/// Specifies the number of nearest neighbors to take into account when computing the CSP.
	PropertyField<int> _numNeighbors;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Centrosymmetry parameter");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_numNeighbors);
};

/**
 * \brief A properties editor for the CentroSymmetryModifier class.
 */
class CentroSymmetryModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE CentroSymmetryModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};


};	// End of namespace

#endif // __OVITO_CENTRO_SYMMETRY_MODIFIER_H
