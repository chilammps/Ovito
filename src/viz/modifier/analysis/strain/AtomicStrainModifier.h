///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
 * \file AtomicStrainModifier.h
 * \brief Contains the definition of the AtomViz::AtomicStrainModifier class.
 */

#ifndef __ATOMIC_STRAIN_MODIFIER_H
#define __ATOMIC_STRAIN_MODIFIER_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/gui/properties/SubObjectParameterUI.h>

#include <atomviz/AtomViz.h>
#include <atomviz/utils/OnTheFlyNeighborList.h>
#include <atomviz/atoms/datachannels/DeformationGradientDataChannel.h>
#include "../../AtomsObjectAnalyzerBase.h"

namespace AtomViz {

/*
 * Calculates atomic-level strain tensors from the relative displacements
 * of neighbor atoms.
 */
class ATOMVIZ_DLLEXPORT AtomicStrainModifier : public AtomsObjectAnalyzerBase
{
public:

	/// Default constructor.
	AtomicStrainModifier(bool isLoading = false);

	/// Returns the data channel that stores the computed deformation gradient tensors.
	DeformationGradientDataChannel* deformationGradientChannel() const { return _deformationGradientChannel; }

	/// Returns the data channel that stores the strain tensors.
	DataChannel* strainTensorChannel() const { return _strainTensorChannel; }

	/// Returns the data channel that stores the shear component of computed strain tensors.
	DataChannel* shearStrainChannel() const { return _shearStrainChannel; }

	/// Returns the data channel that stores the volumetric component of computed strain tensors.
	DataChannel* volumetricStrainChannel() const { return _volumetricStrainChannel; }

	/// Returns the channel that stores the atoms for which the strain tensor could not be computed.
	DataChannel* invalidAtomsChannel() const { return _invalidAtomsChannel; }

	/// \brief Returns the cutoff radius used to build the neighbor lists for the analysis.
	/// \return The cutoff radius in world units.
	/// \sa setNeighborCutoff()
	FloatType neighborCutoff() const { return _neighborCutoff; }

	/// \brief Sets the cutoff radius used to build the neighbor lists for the analysis.
	/// \param newCutoff The new cutoff radius in world units.
	/// \undoable
	/// \sa neighborCutoff()
	void setNeighborCutoff(FloatType newCutoff) { _neighborCutoff = newCutoff; }

	/// Returns the object that contains the reference configuration of the atoms
	/// used for calculating the displacements.
	SceneObject* referenceConfiguration() const { return _referenceObject; }
	/// Sets the object that contains the reference configuration of the atoms
	/// used for calculating the displacements.
	void setReferenceConfiguration(const SceneObject::SmartPtr& refAtoms) { _referenceObject = refAtoms; }

	/// Returns whether the reference configuration is shown instead of the current configuration.
	bool referenceShown() const { return _referenceShown; }
	/// Sets whether the reference configuration is shown instead of the current configuration.
	void setReferenceShown(bool show) { _referenceShown = show; }

	/// Returns whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacements.
	bool eliminateCellDeformation() const { return _eliminateCellDeformation; }

	/// Returns whether we assume the atomic coordinates are unwrapped when calculating the displacements.
	bool assumeUnwrappedCoordinates() const { return _assumeUnwrappedCoordinates; }

	/// Returns whether atoms, for which the strain tensor could not be computed, are being selected.
	bool selectInvalidAtoms() const { return _selectInvalidAtoms; }

	/// \brief Returns the path to the reference configuration file.
	/// \return The path to the input file or an empty string if no input file has been selected.
	/// \sa AtomsImportObject::inputFile()
	/// \sa showSelectionDialog()
	QString inputFile() const;

public Q_SLOTS:

	/// \brief Displays the file selection dialog and lets the user select the file with the reference configuration.
	/// \param parent The optional parent widget for the dialog.
	void showSelectionDialog(QWidget* parent = NULL);

public:

	Q_PROPERTY(bool referenceShown READ referenceShown WRITE setReferenceShown)
	Q_PROPERTY(QString inputFile READ inputFile)

protected:

	// RefTarget virtual functions:
	virtual void saveToStream(ObjectSaveStream& stream);
	virtual void loadFromStream(ObjectLoadStream& stream);
	virtual RefTarget::SmartPtr clone(bool deepCopy, CloneHelper& cloneHelper);

	/// This is the actual analysis method.
	/// It is responsible for storing the analysis results in the results container object that
	/// will be stored along with the modifier application object.
	virtual EvaluationStatus doAnalysis(TimeTicks time, bool suppressDialogs);

	/// Applies the previously calculated analysis results to the atoms object.
	virtual EvaluationStatus applyResult(TimeTicks time, TimeInterval& validityInterval);

	/// Performs the analysis.
	/// Throws an exception on error.
	/// Returns false if the operation has been canceled by the user.
	bool calculate(AtomsObject* atomsObject, bool suppressDialogs = false);

	/// Used internally to store diagnostic information.
	struct AnalysisInfo {

		/// Constructor
		AnalysisInfo(int _numInvalidAtoms = 0, int _minNeighbors = numeric_limits<int>::max(), int _maxNeighbors = 0) :
			numInvalidAtoms(_numInvalidAtoms), maxNeighbors(_maxNeighbors), minNeighbors(_minNeighbors) {}

		/// This is the number of atoms for which we couldn't compute the atomic level strain tensor.
		int numInvalidAtoms;

		/// Minimum number of neighbors per atom.
		int minNeighbors;

		/// Maximum number of neighbors per atom.
		int maxNeighbors;

		/// This function is passed to QtConcurrent::mappedReduced().
		static void reduce(AnalysisInfo& s, AnalysisInfo i);
	};


private:

	/// This helper class is used to split up the computation into small
	/// operations that can be performed on multiple processors in parallel.
	class Kernel {
	public:
		// Constructor that takes references to the input and output arrays.
		Kernel(const OnTheFlyNeighborList& _nnlist, AtomicStrainModifier* _modifier, AtomsObject* currentObj, AtomsObject* refObj);

		// The actual kernel function that is called by the Qt concurrent framework for each atom.
		AnalysisInfo operator()(int atomIndex);

		// Required by QtConcurrent::mappedReduced().
		typedef AnalysisInfo result_type;

	private:
		/// Input neighbor list generator.
		const OnTheFlyNeighborList& nnlist;

		AffineTransformation simCell;
		AffineTransformation currentSimCellInv;
		const Point3* currentPositions;
		array<bool, 3> pbc;
		bool assumeUnwrappedCoordinates;

		Tensor2* deformationGradients;
		SymmetricTensor2* strains;
		FloatType* shearStrains;
		FloatType* volumetricStrains;
		int* selection;
	};

public:

	Q_PROPERTY(FloatType neighborCutoff READ neighborCutoff WRITE setNeighborCutoff)

private:

	/// Stores diagnostic information about the analysis.
	AnalysisInfo _analysisInfo;

	/// The reference configuration of the atoms.
	ReferenceField<SceneObject> _referenceObject;

	/// Controls the whether the reference configuration is shown instead of the current configuration.
	PropertyField<bool> _referenceShown;

	/// Controls the cutoff radius for the neighbor list building.
	PropertyField<FloatType> _neighborCutoff;

	/// Controls the whether the homogeneous deformation of the simulation cell is eliminated from the calculated displacements.
	PropertyField<bool> _eliminateCellDeformation;

	/// Controls the whether we assume the atomic coordinates are unwrapped when calculating the displacements.
	PropertyField<bool> _assumeUnwrappedCoordinates;

	/// This data channel stores the computed deformation gradient tensors.
	ReferenceField<DeformationGradientDataChannel> _deformationGradientChannel;

	/// This data channel stores the computed strain tensors.
	ReferenceField<DataChannel> _strainTensorChannel;

	/// This data channel stores the shear component of computed strain tensors.
	ReferenceField<DataChannel> _shearStrainChannel;

	/// This data channel stores the volumetric component of computed strain tensors.
	ReferenceField<DataChannel> _volumetricStrainChannel;

	/// Controls the whether atomic deformation gradient tensors should be computed and stored.
	PropertyField<bool> _calculateDeformationGradients;

	/// Controls the whether atomic strain tensors should be computed and stored.
	PropertyField<bool> _calculateStrainTensors;

	/// This data channel stores the selection of invalid atoms.
	ReferenceField<DataChannel> _invalidAtomsChannel;

	/// Controls the whether atoms, for which the strain tensor could not be computed, are being selected.
	PropertyField<bool> _selectInvalidAtoms;

private:

	Q_OBJECT
	DECLARE_SERIALIZABLE_PLUGIN_CLASS(AtomicStrainModifier)
	DECLARE_REFERENCE_FIELD(_referenceObject)
	DECLARE_PROPERTY_FIELD(_neighborCutoff)
	DECLARE_PROPERTY_FIELD(_referenceShown)
	DECLARE_PROPERTY_FIELD(_eliminateCellDeformation)
	DECLARE_PROPERTY_FIELD(_assumeUnwrappedCoordinates)
	DECLARE_REFERENCE_FIELD(_deformationGradientChannel)
	DECLARE_REFERENCE_FIELD(_strainTensorChannel)
	DECLARE_REFERENCE_FIELD(_shearStrainChannel)
	DECLARE_REFERENCE_FIELD(_volumetricStrainChannel)
	DECLARE_PROPERTY_FIELD(_calculateDeformationGradients)
	DECLARE_PROPERTY_FIELD(_calculateStrainTensors)
	DECLARE_PROPERTY_FIELD(_selectInvalidAtoms)
	DECLARE_REFERENCE_FIELD(_invalidAtomsChannel)
};

/******************************************************************************
* A properties editor for the AtomicStrainModifier class.
******************************************************************************/
class ATOMVIZ_DLLEXPORT AtomicStrainModifierEditor : public AtomsObjectModifierEditorBase
{
protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams);

protected Q_SLOTS:

	/// Is called when the user presses the Calculate button.
	void onCalculate();

	/// Stores the current cutoff radius in the application settings
	/// so it can be used as default value for new modifiers.
	void memorizeCutoff();

private:

	SubObjectParameterUI* subObjectUI;

	Q_OBJECT
	DECLARE_PLUGIN_CLASS(AtomicStrainModifierEditor)
};

};	// End of namespace AtomViz

#endif // __ATOMIC_STRAIN_MODIFIER_H
