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
 * \file AffineTransformationModifier.h
 * \brief Contains the definition of the Particles::AffineTransformationModifier class.
 */

#ifndef __OVITO_AFFINE_TRANSFORMATION_MODIFIER_H
#define __OVITO_AFFINE_TRANSFORMATION_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/widgets/general/SpinnerWidget.h>
#include "../ParticleModifier.h"

namespace Particles {

using namespace Ovito;

/**
 * \brief This modifier applies an arbitrary affine transformation to the
 *        particles and/or the simulation box.
 *
 * The affine transformation is given by a 3x4 matrix.
 */
class OVITO_PARTICLES_EXPORT AffineTransformationModifier : public ParticleModifier
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE AffineTransformationModifier();

	/// \brief This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp) override;

	// Property access functions:

	/// Returns the affine transformation matrix at the current animation time.
	const AffineTransformation& transformation() const { return _transformationTM; }

	/// Sets the affine transformation at the current animation time.
	void setTransformation(const AffineTransformation& tm) { _transformationTM = tm; }

	/// Returns whether the transformation is applied to the particles.
	bool applyToParticles() const { return _applyToParticles; }

	/// Sets whether the transformation is applied to the particles.
	void setApplyToParticles(bool apply) { _applyToParticles = apply; }

	/// Returns whether the transformation is applied only to the selected particles.
	bool toSelectionOnly() const { return _toSelectionOnly; }

	/// Sets whether the transformation is applied only to the selected particles.
	void setToSelectionOnly(bool onlySelected) { _toSelectionOnly = onlySelected; }

	/// Returns whether the transformation is applied to the simulation box.
	bool applyToSimulationBox() const { return _applyToSimulationBox; }

	/// Sets whether the transformation is applied to the simulation box.
	void setApplyToSimulationBox(bool apply) { _applyToSimulationBox = apply; }

public:

	Q_PROPERTY(bool toSelectionOnly READ toSelectionOnly WRITE setToSelectionOnly)
	Q_PROPERTY(bool applyToSimulationBox READ applyToSimulationBox WRITE setApplyToSimulationBox)
	Q_PROPERTY(bool applyToParticles READ applyToParticles WRITE setApplyToParticles)

protected:

	/// Modifies the particle object.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This property fields stores the transformation matrix (used in 'relative' mode).
	PropertyField<AffineTransformation> _transformationTM;

	/// This property fields stores the simulation cell geometry (used in 'absolute' mode).
	PropertyField<AffineTransformation> _destinationCell;

	/// This controls whether the transformation is applied to the particles.
	PropertyField<bool> _applyToParticles;

	/// This controls whether the transformation is applied only to the selected particles.
	PropertyField<bool> _toSelectionOnly;

	/// This controls whether the transformation is applied to the simulation box.
	PropertyField<bool> _applyToSimulationBox;

	/// This controls whether a relative transformation is applied to the simulation box or
	/// the absolute cell geometry has been specified.
	PropertyField<bool> _relativeMode;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Affine transformation");
	Q_CLASSINFO("ModifierCategory", "Modification");

	DECLARE_PROPERTY_FIELD(_transformationTM);
	DECLARE_PROPERTY_FIELD(_applyToParticles);
	DECLARE_PROPERTY_FIELD(_toSelectionOnly);
	DECLARE_PROPERTY_FIELD(_applyToSimulationBox);
	DECLARE_PROPERTY_FIELD(_destinationCell);
	DECLARE_PROPERTY_FIELD(_relativeMode);
};

/**
 * \brief A properties editor for the AffineTransformationModifier class.
 *
 * This editor class creates and manages the user interface through which the
 * user can alter the modifier's parameters.
 */
class AffineTransformationModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE AffineTransformationModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private Q_SLOTS:

	/// Is called when the spinner value has changed.
	void onSpinnerValueChanged();

	/// Is called when the user begins dragging the spinner interactively.
	void onSpinnerDragStart();

	/// Is called when the user stops dragging the spinner interactively.
	void onSpinnerDragStop();

	/// Is called when the user aborts dragging the spinner interactively.
	void onSpinnerDragAbort();

	/// This method updates the displayed matrix values.
	void updateUI();

private:

	/// Takes the value entered by the user and stores it in transformation controller.
	void updateParameterValue();

	SpinnerWidget* elementSpinners[3][4];

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __AFFINE_TRANSFORMATION_MODIFIER_H
