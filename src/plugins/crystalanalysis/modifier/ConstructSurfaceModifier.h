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

#ifndef __OVITO_CONSTRUCT_SURFACE_MODIFIER_H
#define __OVITO_CONSTRUCT_SURFACE_MODIFIER_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <plugins/particles/modifier/AsynchronousParticleModifier.h>
#include <plugins/particles/objects/SurfaceMesh.h>
#include <plugins/particles/objects/SurfaceMeshDisplay.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/*
 * Constructs a surface mesh from a particle system.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ConstructSurfaceModifier : public AsynchronousParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE ConstructSurfaceModifier(DataSet* dataset);

	/// \brief Returns the data object that stores the generated surface mesh.
	SurfaceMesh* surfaceMesh() const { return _surfaceMeshObj; }

	/// \brief Returns the display object that is responsible for rendering the surface mesh.
	SurfaceMeshDisplay* surfaceMeshDisplay() const { return _surfaceMeshDisplay; }

	/// \brief Returns the radius parameter used during construction of the surface.
	FloatType radius() const { return _radius; }

	/// \brief Sets the radius parameter used during construction of the surface.
	void setRadius(FloatType radius) { _radius = radius; }

	/// \brief Returns the level of smoothing applied to the constructed surface mesh.
	int smoothingLevel() const { return _smoothingLevel; }

	/// \brief Sets the level of smoothing applied to the constructed surface mesh.
	void setSmoothingLevel(int level) { _smoothingLevel = level; }

	/// Returns whether only selected particles are taken into account.
	bool onlySelectedParticles() const { return _onlySelectedParticles; }

	/// Sets whether only selected particles should be taken into account.
	void setOnlySelectedParticles(bool onlySelected) { _onlySelectedParticles = onlySelected; }

	/// Returns the solid volume computed during the last evaluation of the modifier.
	FloatType solidVolume() const { return _solidVolume; }

	/// Returns the total volume computed during the last evaluation of the modifier.
	FloatType totalVolume() const { return _totalVolume; }

	/// Returns the surface area computed during the last evaluation of the modifier.
	FloatType surfaceArea() const { return _surfaceArea; }

protected:

	/// Handles reference events sent by reference targets of this object.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

	/// Creates a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<ComputeEngine> createEngine(TimePoint time, TimeInterval validityInterval) override;

	/// Unpacks the results of the computation engine and stores them in the modifier.
	virtual void transferComputationResults(ComputeEngine* engine) override;

	/// Lets the modifier insert the cached computation results into the modification pipeline.
	virtual PipelineStatus applyComputationResults(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// Computation engine that builds the surface mesh.
	class ConstructSurfaceEngine : public ComputeEngine
	{
	public:

		/// Constructor.
		ConstructSurfaceEngine(const TimeInterval& validityInterval, ParticleProperty* positions, ParticleProperty* selection, const SimulationCell& simCell, FloatType radius, int smoothingLevel) :
			ComputeEngine(validityInterval), _positions(positions), _selection(selection), _simCell(simCell), _radius(radius), _smoothingLevel(smoothingLevel), _isCompletelySolid(false) {}

		/// Computes the modifier's results and stores them in this object for later retrieval.
		virtual void perform() override;

		/// Returns the generated mesh.
		HalfEdgeMesh& mesh() { return _mesh; }

		/// Returns the input particle positions.
		ParticleProperty* positions() const { return _positions.data(); }

		/// Returns the input particle selection.
		ParticleProperty* selection() const { return _selection.data(); }

		/// Returns the computed solid volume.
		FloatType solidVolume() const { return (FloatType)_solidVolume; }

		/// Returns the computed total volume.
		FloatType totalVolume() const { return std::abs(_simCell.matrix().determinant()); }

		/// Returns the computed surface area.
		FloatType surfaceArea() const { return (FloatType)_surfaceArea; }

		/// Indicates whether the entire simulation cell is part of the solid region.
		bool isCompletelySolid() const { return _isCompletelySolid; }

	private:

		FloatType _radius;
		int _smoothingLevel;
		QExplicitlySharedDataPointer<ParticleProperty> _positions;
		QExplicitlySharedDataPointer<ParticleProperty> _selection;
		HalfEdgeMesh _mesh;
		SimulationCell _simCell;
		double _solidVolume;
		double _surfaceArea;
		bool _isCompletelySolid;
	};

	/// Controls the radius of the probe sphere.
	PropertyField<FloatType> _radius;

	/// Controls the amount of smoothing.
	PropertyField<int> _smoothingLevel;

	/// Controls whether only selected particles should be taken into account.
	PropertyField<bool> _onlySelectedParticles;

	/// This stores the cached surface mesh produced by the modifier.
	ReferenceField<SurfaceMesh> _surfaceMeshObj;

	/// The display object for rendering the surface mesh.
	ReferenceField<SurfaceMeshDisplay> _surfaceMeshDisplay;

	/// The solid volume computed during the last evaluation of the modifier.
	FloatType _solidVolume;

	/// The total volume computed during the last evaluation of the modifier.
	FloatType _totalVolume;

	/// The surface area computed during the last evaluation of the modifier.
	FloatType _surfaceArea;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Construct surface mesh");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_radius);
	DECLARE_PROPERTY_FIELD(_smoothingLevel);
	DECLARE_PROPERTY_FIELD(_onlySelectedParticles);
	DECLARE_REFERENCE_FIELD(_surfaceMeshObj);
	DECLARE_REFERENCE_FIELD(_surfaceMeshDisplay);
};

/**
 * Properties editor for the ConstructSurfaceModifier class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ConstructSurfaceModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ConstructSurfaceModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CONSTRUCT_SURFACE_MODIFIER_H
