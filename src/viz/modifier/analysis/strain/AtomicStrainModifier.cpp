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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/animation/AnimManager.h>
#include <core/gui/properties/BooleanPropertyUI.h>
#include <core/gui/properties/FilenamePropertyUI.h>
#include <boost/iterator/counting_iterator.hpp>

#include "AtomicStrainModifier.h"

#include <atomviz/parser/AtomsImportObject.h>
#include <atomviz/utils/OnTheFlyNeighborList.h>
#include <atomviz/utils/ChemicalElements.h>

namespace AtomViz {

IMPLEMENT_SERIALIZABLE_PLUGIN_CLASS(AtomicStrainModifier, AtomsObjectAnalyzerBase)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, SceneObject, "Reference Configuration", _referenceObject)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "NeighborCutoff", _neighborCutoff)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "ShowReferenceConfiguration", _referenceShown)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "EliminateCellDeformation", _eliminateCellDeformation)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "AssumeUnwrappedCoordinates", _assumeUnwrappedCoordinates)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, DataChannel, "DeformationGradientChannel", _deformationGradientChannel)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, DataChannel, "StrainTensorChannel", _strainTensorChannel)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, DataChannel, "ShearStrainChannel", _shearStrainChannel)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, DataChannel, "VolumetricStrainChannel", _volumetricStrainChannel)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "CalculateDeformationGradients", _calculateDeformationGradients)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "CalculateStrainTensors", _calculateStrainTensors)
DEFINE_PROPERTY_FIELD(AtomicStrainModifier, "SelectInvalidAtoms", _selectInvalidAtoms)
DEFINE_REFERENCE_FIELD(AtomicStrainModifier, DataChannel, "InvalidAtomsChannel", _invalidAtomsChannel)
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _neighborCutoff, "Cutoff radius")
SET_PROPERTY_FIELD_UNITS(AtomicStrainModifier, _neighborCutoff, WorldParameterUnit)
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceObject, "Reference Configuration")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _deformationGradientChannel, "Deformation Gradient Channel")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _strainTensorChannel, "Strain Tensor Channel")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _shearStrainChannel, "Shear Strain Channel")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _volumetricStrainChannel, "Volumetric Strain Channel")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _referenceShown, "Show reference configuration")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _eliminateCellDeformation, "Eliminate homogeneous cell deformation")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _assumeUnwrappedCoordinates, "Assume unwrapped coordinates")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateDeformationGradients, "Output deformation gradient tensors")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _calculateStrainTensors, "Output strain tensors")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _selectInvalidAtoms, "Select invalid atoms")
SET_PROPERTY_FIELD_LABEL(AtomicStrainModifier, _invalidAtomsChannel, "Invalid atoms")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
AtomicStrainModifier::AtomicStrainModifier(bool isLoading)
	: AtomsObjectAnalyzerBase(isLoading), _neighborCutoff(0),
	  _referenceShown(false), _eliminateCellDeformation(false), _assumeUnwrappedCoordinates(false),
	  _calculateDeformationGradients(false), _calculateStrainTensors(false), _selectInvalidAtoms(true)
{
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _neighborCutoff);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _referenceObject);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _referenceShown);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _eliminateCellDeformation);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _assumeUnwrappedCoordinates);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _deformationGradientChannel);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _strainTensorChannel);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _shearStrainChannel);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _volumetricStrainChannel);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _calculateDeformationGradients);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _calculateStrainTensors);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _selectInvalidAtoms);
	INIT_PROPERTY_FIELD(AtomicStrainModifier, _invalidAtomsChannel);
	if(!isLoading) {

		// Create data channels for the computation results.
		_deformationGradientChannel = new DeformationGradientDataChannel(DataChannel::DeformationGradientChannel);
		_deformationGradientChannel->setVisible(false);
		_strainTensorChannel = new DataChannel(DataChannel::StrainTensorChannel);
		_strainTensorChannel->setVisible(false);
		_shearStrainChannel = new DataChannel(qMetaTypeId<FloatType>(), sizeof(FloatType), 1);
		_shearStrainChannel->setName(tr("Shear Strain"));
		_volumetricStrainChannel = new DataChannel(qMetaTypeId<FloatType>(), sizeof(FloatType), 1);
		_volumetricStrainChannel->setName(tr("Volumetric Strain"));

		_invalidAtomsChannel = new DataChannel(DataChannel::SelectionChannel);
		_invalidAtomsChannel->setVisible(true);

		// Create data source object.
		AtomsImportObject::SmartPtr importObj = new AtomsImportObject();
		importObj->setAdjustAnimationInterval(false);
		_referenceObject = importObj;

		// Use the default cutoff radius stored in the application settings.
		QSettings settings;
		settings.beginGroup("atomviz/neigborlist");
		setNeighborCutoff(settings.value("DefaultCutoff", 0.0).value<FloatType>());
		settings.endGroup();
	}
}

/******************************************************************************
* Applies the previously calculated analysis results to the atoms object.
******************************************************************************/
EvaluationStatus AtomicStrainModifier::applyResult(TimeTicks time, TimeInterval& validityInterval)
{
	// Check if analysis results are still valid.
	if(input()->atomsCount() != shearStrainChannel()->size())
		throw Exception(tr("Number of atoms of input object has changed. Analysis results became invalid."));

	// Get the reference positions of the atoms.
	if(!_referenceObject)
		throw Exception(tr("No atomic reference configuration has been specified."));

	// Get the reference configuration.
	PipelineFlowState refState = _referenceObject->evalObject(time);
	AtomsObject* refObj = dynamic_object_cast<AtomsObject>(refState.result());
	if(!refObj)
		throw Exception(tr("Please choose an atomic reference configuration."));
	validityInterval.intersect(refState.stateValidity());

	// Deformed and reference configuration must contain the same number of atoms.
	if(refObj->atomsCount() != input()->atomsCount())
		throw Exception(tr("Mismatch between number of atoms in reference configuration and current configuration."));

	// Create copies of the output channels that store the calculated values.
	CloneHelper cloneHelper;
	if(_calculateDeformationGradients && input()->atomsCount() == deformationGradientChannel()->size())
		output()->insertDataChannel(cloneHelper.cloneObject(deformationGradientChannel(), true));
	if(_calculateStrainTensors && input()->atomsCount() == strainTensorChannel()->size())
		output()->insertDataChannel(cloneHelper.cloneObject(strainTensorChannel(), true));
	output()->insertDataChannel(cloneHelper.cloneObject(shearStrainChannel(), true));
	output()->insertDataChannel(cloneHelper.cloneObject(volumetricStrainChannel(), true));

	// Set selection channel.
	if(_selectInvalidAtoms && input()->atomsCount() == invalidAtomsChannel()->size())
		output()->insertDataChannel(cloneHelper.cloneObject(invalidAtomsChannel(), true));

	// Copy atomic positions from reference configuration into geometry pipeline if requested.
	if(referenceShown()) {
		DataChannel* refPosChannel = refObj->getStandardDataChannel(DataChannel::PositionChannel);
		if(!refPosChannel)
			throw Exception(tr("The reference dataset does not contain atomic positions."));

		output()->simulationCell()->setCellMatrix(refObj->simulationCell()->cellMatrix());
		DataChannel* outputPosChannel = outputStandardChannel(DataChannel::PositionChannel);
		OVITO_ASSERT(outputPosChannel->size() == refPosChannel->size());
		memcpy(outputPosChannel->dataPoint3(), refPosChannel->constDataPoint3(), sizeof(Point3) * outputPosChannel->size());
	}

	QString statusMessage = tr("Number of invalid atoms: %1").arg(_analysisInfo.numInvalidAtoms);
	statusMessage += tr("\nMinimum number of neighbors: %1").arg(_analysisInfo.minNeighbors);
	statusMessage += tr("\nMaximum number of neighbors: %1").arg(_analysisInfo.maxNeighbors);

	return EvaluationStatus(EvaluationStatus::EVALUATION_SUCCESS, QString(), statusMessage);
}

/******************************************************************************
* This is the actual analysis method.
******************************************************************************/
EvaluationStatus AtomicStrainModifier::doAnalysis(TimeTicks time, bool suppressDialogs)
{
	// Perform analysis.
	if(calculate(input(), suppressDialogs))
		return EvaluationStatus();
	else
		return EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, tr("Calculation has been canceled by the user."));
}

/******************************************************************************
* This function is passed to QtConcurrent::mappedReduced() to combine
* the analysis information from multiple worker threads.
******************************************************************************/
void AtomicStrainModifier::AnalysisInfo::reduce(AnalysisInfo& s, AnalysisInfo i)
{
	s.numInvalidAtoms += i.numInvalidAtoms;
	s.minNeighbors = std::min(i.minNeighbors, s.minNeighbors);
	s.maxNeighbors = std::max(i.maxNeighbors, s.maxNeighbors);
}

/******************************************************************************
* Performs the analysis.
* Throws an exception on error.
* Returns false when the operation has been canceled by the user.
******************************************************************************/
bool AtomicStrainModifier::calculate(AtomsObject* atomsObject, bool suppressDialogs)
{
	ProgressIndicator progress(tr("Calculating atomic strain tensors (on %n processor(s))", NULL, QThread::idealThreadCount()), atomsObject->atomsCount(), suppressDialogs);

	// Reset everything.
	deformationGradientChannel()->setSize(0);
	strainTensorChannel()->setSize(0);
	shearStrainChannel()->setSize(0);
	volumetricStrainChannel()->setSize(0);
	invalidAtomsChannel()->setSize(0);

	// Get the reference positions of the atoms.
	if(!_referenceObject)
		throw Exception(tr("Cannot calculate atomic displacements. Atomic reference configuration has not been specified."));

	// Get the reference configuration.
	PipelineFlowState refState = _referenceObject->evalObject(0);
	AtomsObject* refObj = dynamic_object_cast<AtomsObject>(refState.result());
	if(!refObj)
		throw Exception(tr("Please choose a reference configuration."));

	// Deformed and reference configuration must contain the same number of atoms.
	if(refObj->atomsCount() != atomsObject->atomsCount())
		throw Exception(tr("Cannot calculate atomic strain tensors. Mismatch between number of atoms in reference configuration and current configuration."));

	// Check simulation cell(s).
	if(fabs(atomsObject->simulationCell()->volume()) < FLOATTYPE_EPSILON || fabs(refObj->simulationCell()->volume()) < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in either the deformed or the reference configuration."));

	// Prepare the neighbor list.
	OnTheFlyNeighborList neighborList(neighborCutoff());
	if(!neighborList.prepare(refObj, suppressDialogs))
		return false;

	// Get the current position channels.
	expectStandardChannel(DataChannel::PositionChannel);

	// Prepare the output channels.
	if(_calculateDeformationGradients)
		deformationGradientChannel()->setSize(atomsObject->atomsCount());
	if(_calculateStrainTensors)
		strainTensorChannel()->setSize(atomsObject->atomsCount());
	shearStrainChannel()->setSize(atomsObject->atomsCount());
	volumetricStrainChannel()->setSize(atomsObject->atomsCount());
	if(_selectInvalidAtoms)
		invalidAtomsChannel()->setSize(atomsObject->atomsCount());

	// These calls are necessary to deep copy the memory array of the data channels before accessing them from multiple threads.
	if(_calculateDeformationGradients) deformationGradientChannel()->dataFloat();
	if(_calculateStrainTensors)strainTensorChannel()->dataFloat();
	shearStrainChannel()->dataFloat();
	volumetricStrainChannel()->dataFloat();
	if(_selectInvalidAtoms) invalidAtomsChannel()->dataInt();

	// Execute analysis code for each atom in a parallel fashion.
	Kernel kernel(neighborList, this, atomsObject, refObj);
	boost::counting_iterator<int> firstAtom(0);
	boost::counting_iterator<int> lastAtom(atomsObject->atomsCount());
	QFuture<AtomicStrainModifier::AnalysisInfo> future = QtConcurrent::mappedReduced(firstAtom, lastAtom, kernel, AtomicStrainModifier::AnalysisInfo::reduce);
	progress.waitForFuture(future);

	// Throw away results obtained so far if the user cancels the calculation.
	if(future.isCanceled()) {
		deformationGradientChannel()->setSize(0);
		strainTensorChannel()->setSize(0);
		shearStrainChannel()->setSize(0);
		volumetricStrainChannel()->setSize(0);
		invalidAtomsChannel()->setSize(0);
		return false;
	}
	_analysisInfo = future.result();

	return true;
}

/******************************************************************************
* Constructor that takes references to the input and output arrays.
******************************************************************************/
AtomicStrainModifier::Kernel::Kernel(const OnTheFlyNeighborList& _nnlist, AtomicStrainModifier* modifier, AtomsObject* currentObj, AtomsObject* refObj)
	: nnlist(_nnlist)
{
	pbc = currentObj->simulationCell()->periodicity();
	currentSimCellInv = currentObj->simulationCell()->cellMatrix().inverse();
	if(modifier->eliminateCellDeformation())
		simCell = refObj->simulationCell()->cellMatrix();
	else
		simCell = currentObj->simulationCell()->cellMatrix();
	currentPositions = modifier->expectStandardChannel(DataChannel::PositionChannel)->constDataPoint3();
	assumeUnwrappedCoordinates = modifier->assumeUnwrappedCoordinates();

	if(modifier->deformationGradientChannel()->size() == currentObj->atomsCount())
		deformationGradients = modifier->deformationGradientChannel()->dataTensor2();
	else
		deformationGradients = NULL;
	if(modifier->strainTensorChannel()->size() == currentObj->atomsCount())
		strains = modifier->strainTensorChannel()->dataSymmetricTensor2();
	else
		strains = NULL;
	shearStrains = modifier->shearStrainChannel()->dataFloat();
	volumetricStrains = modifier->volumetricStrainChannel()->dataFloat();

	if(modifier->invalidAtomsChannel()->size() == currentObj->atomsCount())
		selection = modifier->invalidAtomsChannel()->dataInt();
	else
		selection = NULL;
}


/******************************************************************************
* Calculates the strain tensor for a single atom.
******************************************************************************/
AtomicStrainModifier::AnalysisInfo AtomicStrainModifier::Kernel::operator()(int atomIndex)
{
	Matrix3 V(NULL_MATRIX);
	Matrix3 W(NULL_MATRIX);

	// Iterate over neighbor vectors of central atom.
	const Point3 x = currentPositions[atomIndex];
	int numNeighbors = 0;
	for(OnTheFlyNeighborList::iterator neighborIter(nnlist, atomIndex); !neighborIter.atEnd(); neighborIter.next()) {
		const Vector3& r0 = neighborIter.delta();
		Vector3 r = currentPositions[neighborIter.current()] - x;
		Vector3 sr = currentSimCellInv * r;
		if(!assumeUnwrappedCoordinates) {
			for(int k = 0; k < 3; k++) {
				if(!pbc[k]) continue;
				while(sr[k] > 0.5) sr[k] -= 1.0;
				while(sr[k] < -0.5) sr[k] += 1.0;
			}
		}
		r = simCell * sr;

		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				V(i,j) += r0[j] * r0[i];
				W(i,j) += r0[j] * r[i];
			}
		}

		numNeighbors++;
	}

	if(fabs(V.determinant()) < FLOATTYPE_EPSILON) {
		if(selection) selection[atomIndex] = 1;
		return AnalysisInfo(1, numNeighbors, numNeighbors);
	}

	// Calculate deformation gradient tensor.
	Tensor2 F = W * V.inverse();
	if(deformationGradients)
		deformationGradients[atomIndex] = F;

	// Calculate strain tensor.
	SymmetricTensor2 strain = (Product_AtA(F) - IDENTITY) * 0.5;
	if(strains)
		strains[atomIndex] = strain;

	// Calculate shear component.
	if(shearStrains) {
		shearStrains[atomIndex] = sqrt(square(strain(0,1)) + square(strain(1,2)) + square(strain(0,2)) + (square(strain(1,1) - strain(2,2)) + square(strain(0,0) - strain(2,2)) + square(strain(0,0) - strain(1,1))) / 6.0);
#ifndef Q_CC_MSVC
		OVITO_ASSERT(!isnan(shearStrains[atomIndex]) && !isinf(shearStrains[atomIndex]));
#endif
	}

	// Calculate volumetric component.
	if(volumetricStrains) {
		volumetricStrains[atomIndex] = (strain(0,0) + strain(1,1) + strain(2,2)) / 3;
#ifndef Q_CC_MSVC
		OVITO_ASSERT(!isnan(volumetricStrains[atomIndex]) && !isinf(volumetricStrains[atomIndex]));
#endif
	}

	if(selection) selection[atomIndex] = 0;
	return AnalysisInfo(0, numNeighbors, numNeighbors);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void AtomicStrainModifier::saveToStream(ObjectSaveStream& stream)
{
	AtomsObjectAnalyzerBase::saveToStream(stream);

	stream.beginChunk(0x1);
	stream << _analysisInfo.numInvalidAtoms;
	stream << _analysisInfo.minNeighbors;
	stream << _analysisInfo.maxNeighbors;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void AtomicStrainModifier::loadFromStream(ObjectLoadStream& stream)
{
	AtomsObjectAnalyzerBase::loadFromStream(stream);

	stream.expectChunk(0x1);
	stream >> _analysisInfo.numInvalidAtoms;
	stream >> _analysisInfo.minNeighbors;
	stream >> _analysisInfo.maxNeighbors;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
RefTarget::SmartPtr AtomicStrainModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	AtomicStrainModifier::SmartPtr clone = static_object_cast<AtomicStrainModifier>(AtomsObjectAnalyzerBase::clone(deepCopy, cloneHelper));
	clone->_analysisInfo = this->_analysisInfo;
	return clone;
}

/******************************************************************************
* Returns the path to the reference configuration file.
******************************************************************************/
QString AtomicStrainModifier::inputFile() const
{
	AtomsImportObject* importObj = dynamic_object_cast<AtomsImportObject>(referenceConfiguration());
	if(importObj)
		return importObj->inputFile();
	else
		return QString();
}

/******************************************************************************
* Displays the file selection dialog and lets the user select the file with the reference configuration.
******************************************************************************/
void AtomicStrainModifier::showSelectionDialog(QWidget* parent)
{
	AtomsImportObject* importObj = dynamic_object_cast<AtomsImportObject>(referenceConfiguration());
	if(importObj)
		importObj->showSelectionDialog(parent);
}

IMPLEMENT_PLUGIN_CLASS(AtomicStrainModifierEditor, AtomsObjectModifierEditorBase)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void AtomicStrainModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Calculate atomic strain"), rolloutParams, "atomviz.modifiers.calculate_atomic_strain", "atomviz.modifiers.calculate_atomic_strain.html");

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(0);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setColumnStretch(1, 1);

	// Cutoff parameter.
	FloatPropertyUI* cutoffRadiusPUI = new FloatPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _neighborCutoff));
	gridlayout->addWidget(cutoffRadiusPUI->label(), 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);
	connect(cutoffRadiusPUI->spinner(), SIGNAL(spinnerValueChanged()), this, SLOT(memorizeCutoff()));

	CutoffPresetsUI* cutoffPresetsPUI = new CutoffPresetsUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _neighborCutoff));
	gridlayout->addWidget(cutoffPresetsPUI->comboBox(), 1, 1);

	layout1->addLayout(gridlayout);

	BooleanPropertyUI* calculateDeformationGradientsUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _calculateDeformationGradients));
	layout1->addWidget(calculateDeformationGradientsUI->checkBox());

	BooleanPropertyUI* calculateStrainTensorsUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _calculateStrainTensors));
	layout1->addWidget(calculateStrainTensorsUI->checkBox());

	QCheckBox* calculateShearStrainsBox = new QCheckBox(tr("Output shear strains"));
	calculateShearStrainsBox->setEnabled(false);
	calculateShearStrainsBox->setChecked(true);
	layout1->addWidget(calculateShearStrainsBox);

	QCheckBox* calculateVolumetricStrainsBox = new QCheckBox(tr("Output volumetric strains"));
	calculateVolumetricStrainsBox->setEnabled(false);
	calculateVolumetricStrainsBox->setChecked(true);
	layout1->addWidget(calculateVolumetricStrainsBox);

	BooleanPropertyUI* eliminateCellDeformationUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _eliminateCellDeformation));
	layout1->addWidget(eliminateCellDeformationUI->checkBox());

	BooleanPropertyUI* assumeUnwrappedUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _assumeUnwrappedCoordinates));
	layout1->addWidget(assumeUnwrappedUI->checkBox());

	BooleanPropertyUI* showReferenceUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _referenceShown));
	layout1->addWidget(showReferenceUI->checkBox());

	BooleanPropertyUI* selectInvalidAtomsUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _selectInvalidAtoms));
	layout1->addWidget(selectInvalidAtomsUI->checkBox());

	BooleanPropertyUI* autoUpdateUI = new BooleanPropertyUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomsObjectAnalyzerBase, _autoUpdateOnTimeChange));
	layout1->addWidget(autoUpdateUI->checkBox());

	FilenamePropertyUI* inputFilePUI = new FilenamePropertyUI(this, "inputFile", SLOT(showSelectionDialog(QWidget*)));
	layout1->addWidget(inputFilePUI->selectorWidget());
	inputFilePUI->selectorWidget()->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));

	QPushButton* calcButton = new QPushButton(tr("Calculate"), rollout);
	layout1->addSpacing(6);
	layout1->addWidget(calcButton);
	connect(calcButton, SIGNAL(clicked(bool)), this, SLOT(onCalculate()));

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	// Open a sub-editor for the internal AtomsImportObject.
	subObjectUI = new SubObjectParameterUI(this, PROPERTY_FIELD_DESCRIPTOR(AtomicStrainModifier, _referenceObject));
}

/******************************************************************************
* Is called when the user presses the Calculate button.
******************************************************************************/
void AtomicStrainModifierEditor::onCalculate()
{
	if(!editObject()) return;
	AtomicStrainModifier* modifier = static_object_cast<AtomicStrainModifier>(editObject());
	try {
		modifier->performAnalysis(ANIM_MANAGER.time());
	}
	catch(Exception& ex) {
		ex.prependGeneralMessage(tr("Failed to compute atomic strain tensors."));
		ex.showError();
	}
}

/******************************************************************************
* Stores the current cutoff radius in the application settings
* so it can be used as default value for new modifiers.
******************************************************************************/
void AtomicStrainModifierEditor::memorizeCutoff()
{
	if(!editObject()) return;
	AtomicStrainModifier* modifier = static_object_cast<AtomicStrainModifier>(editObject());

	QSettings settings;
	settings.beginGroup("atomviz/neigborlist");
	settings.setValue("DefaultCutoff", modifier->neighborCutoff());
	settings.endGroup();
}

};	// End of namespace AtomViz
