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

#ifndef __OVITO_AFFINE_TRANSFORMATION_MODIFIER_H
#define __OVITO_AFFINE_TRANSFORMATION_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/gui/widgets/general/SpinnerWidget.h>
#include "../ParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

/**
 * \brief This modifier applies an arbitrary affine transformation to the
 *        particles and/or the simulation box.
 *
 * The affine transformation is given by a 3x4 matrix.
 */
class OVITO_PARTICLES_EXPORT AffineTransformationModifier : public ParticleModifier
{
public:

	/// \brief Constructor.
	Q_INVOKABLE AffineTransformationModifier(DataSet* dataset);

	// Property access functions:

	/// Returns the affine transformation matrix.
	const AffineTransformation& transformation() const { return _transformationTM; }

	/// Sets the affine transformation.
	void setTransformation(const AffineTransformation& tm) { _transformationTM = tm; }

	/// Returns the target cell matrix matrix for absolute transformation mode.
	const AffineTransformation& targetCell() const { return _targetCell; }

	/// Sets the target cell matrix for absolute transformation mode.
	void setTargetCell(const AffineTransformation& cell) { _targetCell = cell; }

	/// Returns true if relative transformation mode is selected; returns false if absolute transformation mode is active.
	bool relativeMode() const { return _relativeMode; }

	/// Switches between relative and absolute transformation mode.
	void setRelativeMode(bool relative) { _relativeMode = relative; }

	/// Returns whether the transformation is applied to the particles.
	bool applyToParticles() const { return _applyToParticles; }

	/// Sets whether the transformation is applied to the particles.
	void setApplyToParticles(bool apply) { _applyToParticles = apply; }

	/// Returns whether the transformation is applied only to the selected particles.
	bool selectionOnly() const { return _toSelectionOnly; }

	/// Sets whether the transformation is applied only to the selected particles.
	void setSelectionOnly(bool onlySelected) { _toSelectionOnly = onlySelected; }

	/// Returns whether the transformation is applied to the simulation box.
	bool applyToSimulationBox() const { return _applyToSimulationBox; }

	/// Sets whether the transformation is applied to the simulation box.
	void setApplyToSimulationBox(bool apply) { _applyToSimulationBox = apply; }

	/// Returns whether the transformation is applied to a surface mesh.
	bool applyToSurfaceMesh() const { return _applyToSurfaceMesh; }

	/// Sets whether the transformation is applied to a surface mesh.
	void setApplyToSurfaceMesh(bool apply) { _applyToSurfaceMesh = apply; }

protected:

	/// \brief This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp) override;

	/// Modifies the particle object.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This property fields stores the transformation matrix (used in 'relative' mode).
	PropertyField<AffineTransformation> _transformationTM;

	/// This property fields stores the simulation cell geometry (used in 'absolute' mode).
	PropertyField<AffineTransformation> _targetCell;

	/// This controls whether the transformation is applied to the particles.
	PropertyField<bool> _applyToParticles;

	/// This controls whether the transformation is applied only to the selected particles.
	PropertyField<bool> _toSelectionOnly;

	/// This controls whether the transformation is applied to the simulation box.
	PropertyField<bool> _applyToSimulationBox;

	/// This controls whether a relative transformation is applied to the simulation box or
	/// the absolute cell geometry has been specified.
	PropertyField<bool> _relativeMode;

	/// This controls whether the transformation is applied to surface meshes.
	PropertyField<bool> _applyToSurfaceMesh;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Affine transformation");
	Q_CLASSINFO("ModifierCategory", "Modification");

	DECLARE_PROPERTY_FIELD(_transformationTM);
	DECLARE_PROPERTY_FIELD(_applyToParticles);
	DECLARE_PROPERTY_FIELD(_toSelectionOnly);
	DECLARE_PROPERTY_FIELD(_applyToSimulationBox);
	DECLARE_PROPERTY_FIELD(_targetCell);
	DECLARE_PROPERTY_FIELD(_relativeMode);
	DECLARE_PROPERTY_FIELD(_applyToSurfaceMesh);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the AffineTransformationModifier class.
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

	/// Is called when the user presses the 'Enter rotation' button.
	void onEnterRotation();

private:

	/// Takes the value entered by the user and stores it in transformation controller.
	void updateParameterValue();

	SpinnerWidget* elementSpinners[3][4];

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __AFFINE_TRANSFORMATION_MODIFIER_H
