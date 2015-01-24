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

#include <plugins/particles/Particles.h>
#include <core/dataset/importexport/FileSource.h>
#include <core/animation/AnimationSettings.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/SubObjectParameterUI.h>
#include <plugins/particles/util/NearestNeighborFinder.h>
#include "WignerSeitzAnalysisModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, WignerSeitzAnalysisModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(WignerSeitzAnalysisModifier, WignerSeitzAnalysisModifierEditor);
DEFINE_REFERENCE_FIELD(WignerSeitzAnalysisModifier, _referenceObject, "Reference Configuration", DataObject);
DEFINE_FLAGS_PROPERTY_FIELD(WignerSeitzAnalysisModifier, _eliminateCellDeformation, "EliminateCellDeformation", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(WignerSeitzAnalysisModifier, _useReferenceFrameOffset, "UseReferenceFrameOffet");
DEFINE_PROPERTY_FIELD(WignerSeitzAnalysisModifier, _referenceFrameNumber, "ReferenceFrameNumber");
DEFINE_FLAGS_PROPERTY_FIELD(WignerSeitzAnalysisModifier, _referenceFrameOffset, "ReferenceFrameOffset", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(WignerSeitzAnalysisModifier, _referenceObject, "Reference Configuration");
SET_PROPERTY_FIELD_LABEL(WignerSeitzAnalysisModifier, _eliminateCellDeformation, "Eliminate homogeneous cell deformation");
SET_PROPERTY_FIELD_LABEL(WignerSeitzAnalysisModifier, _useReferenceFrameOffset, "Use reference frame offset");
SET_PROPERTY_FIELD_LABEL(WignerSeitzAnalysisModifier, _referenceFrameNumber, "Reference frame number");
SET_PROPERTY_FIELD_LABEL(WignerSeitzAnalysisModifier, _referenceFrameOffset, "Reference frame offset");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, WignerSeitzAnalysisModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
WignerSeitzAnalysisModifier::WignerSeitzAnalysisModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_eliminateCellDeformation(false),
	_useReferenceFrameOffset(false), _referenceFrameNumber(0), _referenceFrameOffset(-1),
	_vacancyCount(0), _interstitialCount(0)
{
	INIT_PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceObject);
	INIT_PROPERTY_FIELD(WignerSeitzAnalysisModifier::_eliminateCellDeformation);
	INIT_PROPERTY_FIELD(WignerSeitzAnalysisModifier::_useReferenceFrameOffset);
	INIT_PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceFrameNumber);
	INIT_PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceFrameOffset);

	// Create the file source object that will be responsible for loading
	// and storing the reference configuration.
	OORef<FileSource> linkedFileObj(new FileSource(dataset));

	// Disable the automatic adjustment of the animation length.
	// We don't want the scene's animation interval to be affected by an animation
	// loaded into the reference configuration object.
	linkedFileObj->setAdjustAnimationIntervalEnabled(false);
	setReferenceConfiguration(linkedFileObj);
}

/******************************************************************************
* Returns the source URL of the reference configuration.
******************************************************************************/
QUrl WignerSeitzAnalysisModifier::referenceSource() const
{
	if(FileSource* linkedFileObj = dynamic_object_cast<FileSource>(referenceConfiguration()))
		return linkedFileObj->sourceUrl();
	else
		return QUrl();
}

/******************************************************************************
* Sets the source URL of the reference configuration.
******************************************************************************/
void WignerSeitzAnalysisModifier::setReferenceSource(const QUrl& sourceUrl, const OvitoObjectType* importerType)
{
	if(FileSource* linkedFileObj = dynamic_object_cast<FileSource>(referenceConfiguration())) {
		linkedFileObj->setSource(sourceUrl, importerType);
	}
	else {
		OORef<FileSource> newObj(new FileSource(dataset()));
		newObj->setSource(sourceUrl, importerType);
		setReferenceConfiguration(newObj);
	}
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> WignerSeitzAnalysisModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get the reference configuration.
	PipelineFlowState refState = getReferenceState(time);
	if(refState.isEmpty())
		throw Exception(tr("Reference configuration has not been specified yet or is empty. Please pick a reference simulation file."));

	// Get the reference position property.
	ParticlePropertyObject* refPosProperty = ParticlePropertyObject::findInState(refState, ParticleProperty::PositionProperty);
	if(!refPosProperty)
		throw Exception(tr("The reference configuration does not contain particle positions."));

	// Get simulation cells.
	SimulationCellObject* inputCell = expectSimulationCell();
	SimulationCellObject* refCell = refState.findObject<SimulationCellObject>();
	if(!refCell)
		throw Exception(tr("Reference configuration does not contain simulation cell info."));

	// Check simulation cell(s).
	if(inputCell->volume() < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in the deformed configuration."));
	if(refCell->volume() < FLOATTYPE_EPSILON)
		throw Exception(tr("Simulation cell is degenerate in the reference configuration."));

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<WignerSeitzAnalysisEngine>(validityInterval, posProperty->storage(), inputCell->data(),
			refPosProperty->storage(), refCell->data(), eliminateCellDeformation());
}

/******************************************************************************
* Returns the reference state to be used to perform the analysis at the given time.
******************************************************************************/
PipelineFlowState WignerSeitzAnalysisModifier::getReferenceState(TimePoint time)
{
	// Get the reference positions of particles.
	if(!referenceConfiguration())
		throw Exception(tr("Cannot perform analysis without a reference configuration."));

	// What is the reference frame number to use?
	int referenceFrame;
	if(useReferenceFrameOffset()) {
		// Determine the current frame, preferably from the "Frame" attribute stored with the pipeline flow state.
		// If the "Frame" attribute is not present, infer it from the current animation time.
		int currentFrame = input().attributes().value(QStringLiteral("Frame"),
				dataset()->animationSettings()->timeToFrame(time)).toInt();

		// Use frame offset relative to current configuration.
		referenceFrame = currentFrame + referenceFrameOffset();
	}
	else {
		// Always use the same, user-specified frame as reference configuration.
		referenceFrame = referenceFrameNumber();
	}

	// Get the reference configuration.
	PipelineFlowState refState;
	if(FileSource* linkedFileObj = dynamic_object_cast<FileSource>(referenceConfiguration())) {
		if(linkedFileObj->numberOfFrames() > 0) {
			if(referenceFrame < 0 || referenceFrame >= linkedFileObj->numberOfFrames())
				throw Exception(tr("Requested reference frame %1 is out of range.").arg(referenceFrame));
			refState = linkedFileObj->requestFrame(referenceFrame);
		}
	}
	else refState = referenceConfiguration()->evaluate(dataset()->animationSettings()->frameToTime(referenceFrame));

	// Make sure the obtained reference configuration is valid and ready to use.
	if(refState.status().type() == PipelineStatus::Error)
		throw refState.status();
	if(refState.status().type() == PipelineStatus::Pending)
		throw PipelineStatus(PipelineStatus::Pending, tr("Waiting for input data to become ready..."));
	// Make sure we really received the requested reference frame.
	if(refState.attributes().value(QStringLiteral("Frame"), referenceFrame).toInt() != referenceFrame)
		throw Exception(tr("Requested reference frame %1 is out of range.").arg(referenceFrame));

	return refState;
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void WignerSeitzAnalysisModifier::WignerSeitzAnalysisEngine::perform()
{
	setProgressText(tr("Performing Wigner-Seitz cell analysis"));

	size_t particleCount = positions()->size();
	if(refPositions()->size() == 0)
		return;

	// Prepare the closest-point query structure.
	NearestNeighborFinder neighborTree(0);
	if(!neighborTree.prepare(refPositions(), refCell(), this))
		return;

	// Create output storage.
	ParticleProperty* output = occupancyNumbers();
	setProgressRange(particleCount);

	AffineTransformation tm;
	if(_eliminateCellDeformation)
		tm = refCell().matrix() * cell().inverseMatrix();

	FloatType closestDistanceSq;
	int particleIndex = 0;
	for(const Point3& p : positions()->constPoint3Range()) {

		Point3 p2 = _eliminateCellDeformation ? (tm * p) : p;
		int closestIndex = neighborTree.findClosestParticle(p2, closestDistanceSq);
		OVITO_ASSERT(closestIndex >= 0 && closestIndex < output->size());
		output->dataInt()[closestIndex]++;

		particleIndex++;
		if((particleIndex % 1024) == 0) {
			if(isCanceled())
				return;
			setProgressValue(particleIndex);
		}
	}

	// Count defects.
	_vacancyCount = 0;
	_interstitialCount = 0;
	for(int oc : output->constIntRange()) {
		if(oc == 0) _vacancyCount++;
		else if(oc > 1) _interstitialCount += oc - 1;
	}
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void WignerSeitzAnalysisModifier::transferComputationResults(ComputeEngine* engine)
{
	WignerSeitzAnalysisEngine* eng = static_cast<WignerSeitzAnalysisEngine*>(engine);
	_occupancyNumbers = eng->occupancyNumbers();
	_vacancyCount = eng->vacancyCount();
	_interstitialCount = eng->interstitialCount();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus WignerSeitzAnalysisModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_occupancyNumbers)
		throw Exception(tr("No computation results available."));

	PipelineFlowState refState = getReferenceState(time);

	QVariantMap oldAttributes = output().attributes();
	TimeInterval oldValidity = output().stateValidity();

	// Replace pipeline contents with reference configuration.
	output() = refState;
	output().setStateValidity(oldValidity);
	output().attributes() = oldAttributes;

	ParticlePropertyObject* posProperty = ParticlePropertyObject::findInState(output(), ParticleProperty::PositionProperty);
	if(!posProperty)
		throw Exception(tr("This modifier cannot be evaluated, because the reference configuration does not contain any particles."));
	_outputParticleCount = posProperty->size();

	if(posProperty->size() != _occupancyNumbers->size())
		throw Exception(tr("The number of particles in the reference configuration has changed. The stored results have become invalid."));

	outputCustomProperty(_occupancyNumbers.data());

	return PipelineStatus(PipelineStatus::Success, tr("Found %1 vacancies and %2 interstitials").arg(vacancyCount()).arg(interstitialCount()));
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void WignerSeitzAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute modifier results when the parameters have changed.
	if(field == PROPERTY_FIELD(WignerSeitzAnalysisModifier::_eliminateCellDeformation)
			|| field == PROPERTY_FIELD(WignerSeitzAnalysisModifier::_useReferenceFrameOffset)
			|| field == PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceFrameNumber)
			|| field == PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceFrameOffset))
		invalidateCachedResults();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void WignerSeitzAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Wigner-Seitz defect analysis"), rolloutParams, "particles.modifiers.wigner_seitz_analysis.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	BooleanParameterUI* eliminateCellDeformationUI = new BooleanParameterUI(this, PROPERTY_FIELD(WignerSeitzAnalysisModifier::_eliminateCellDeformation));
	layout->addWidget(eliminateCellDeformationUI->checkBox());

	QGroupBox* referenceFrameGroupBox = new QGroupBox(tr("Reference frame"));
	layout->addWidget(referenceFrameGroupBox);

	QGridLayout* sublayout = new QGridLayout(referenceFrameGroupBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(0, 5);
	sublayout->setColumnStretch(2, 95);

	// Add box for selection between absolute and relative reference frames.
	BooleanRadioButtonParameterUI* useFrameOffsetUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(WignerSeitzAnalysisModifier::_useReferenceFrameOffset));
	useFrameOffsetUI->buttonTrue()->setText(tr("Relative to current frame"));
	useFrameOffsetUI->buttonFalse()->setText(tr("Fixed reference configuration"));
	sublayout->addWidget(useFrameOffsetUI->buttonFalse(), 0, 0, 1, 3);

	IntegerParameterUI* frameNumberUI = new IntegerParameterUI(this, PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceFrameNumber));
	frameNumberUI->label()->setText(tr("Frame number:"));
	sublayout->addWidget(frameNumberUI->label(), 1, 1, 1, 1);
	sublayout->addLayout(frameNumberUI->createFieldLayout(), 1, 2, 1, 1);
	frameNumberUI->setMinValue(0);
	frameNumberUI->setEnabled(false);
	connect(useFrameOffsetUI->buttonFalse(), &QRadioButton::toggled, frameNumberUI, &IntegerParameterUI::setEnabled);

	sublayout->addWidget(useFrameOffsetUI->buttonTrue(), 2, 0, 1, 3);
	IntegerParameterUI* frameOffsetUI = new IntegerParameterUI(this, PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceFrameOffset));
	frameOffsetUI->label()->setText(tr("Frame offset:"));
	sublayout->addWidget(frameOffsetUI->label(), 3, 1, 1, 1);
	sublayout->addLayout(frameOffsetUI->createFieldLayout(), 3, 2, 1, 1);
	frameOffsetUI->setEnabled(false);
	connect(useFrameOffsetUI->buttonTrue(), &QRadioButton::toggled, frameOffsetUI, &IntegerParameterUI::setEnabled);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());

	// Open a sub-editor for the reference object.
	new SubObjectParameterUI(this, PROPERTY_FIELD(WignerSeitzAnalysisModifier::_referenceObject), RolloutInsertionParameters().setTitle(tr("Reference")));
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
