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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <plugins/crystalanalysis/data/dislocations/DislocationNetwork.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include "SmoothDislocationsModifier.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, SmoothDislocationsModifier, Modifier);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, SmoothDislocationsModifierEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(SmoothDislocationsModifier, SmoothDislocationsModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(SmoothDislocationsModifier, _smoothingEnabled, "SmoothingEnabled", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SmoothDislocationsModifier, _smoothingLevel, "SmoothingLevel", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SmoothDislocationsModifier, _coarseningEnabled, "CoarseningEnabled", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SmoothDislocationsModifier, _linePointInterval, "LinePointInterval", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(SmoothDislocationsModifier, _smoothingEnabled, "Enable smoothing");
SET_PROPERTY_FIELD_LABEL(SmoothDislocationsModifier, _smoothingLevel, "Smoothing level");
SET_PROPERTY_FIELD_LABEL(SmoothDislocationsModifier, _coarseningEnabled, "Enable coarsening");
SET_PROPERTY_FIELD_LABEL(SmoothDislocationsModifier, _linePointInterval, "Point separation");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
SmoothDislocationsModifier::SmoothDislocationsModifier(DataSet* dataset) : Modifier(dataset),
	_smoothingEnabled(true), _coarseningEnabled(true),
	_smoothingLevel(4), _linePointInterval(2)
{
	INIT_PROPERTY_FIELD(SmoothDislocationsModifier::_smoothingEnabled);
	INIT_PROPERTY_FIELD(SmoothDislocationsModifier::_smoothingLevel);
	INIT_PROPERTY_FIELD(SmoothDislocationsModifier::_coarseningEnabled);
	INIT_PROPERTY_FIELD(SmoothDislocationsModifier::_linePointInterval);
}

/******************************************************************************
* Asks the modifier whether it can be applied to the given input data.
******************************************************************************/
bool SmoothDislocationsModifier::isApplicableTo(const PipelineFlowState& input)
{
	return (input.findObject<DislocationNetwork>() != nullptr);
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus SmoothDislocationsModifier::modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state)
{
	DislocationNetwork* inputDislocations = state.findObject<DislocationNetwork>();
	if(!inputDislocations)
		return PipelineStatus::Success;	// Nothing to smooth in the modifier's input.

	CloneHelper cloneHelper;
	OORef<DislocationNetwork> outputDislocations = cloneHelper.cloneObject(inputDislocations, false);

	for(DislocationSegment* segment : outputDislocations->segments()) {
		QVector<Point3> line;
		QVector<int> coreSize;
		coarsenDislocationLine(_coarseningEnabled.value() ? _linePointInterval.value() : 0, segment->line(), segment->coreSize(), line, coreSize, segment->isClosedLoop() && !segment->isInfiniteLine());
		smoothDislocationLine(_smoothingEnabled.value() ? _smoothingLevel.value() : 0, line, segment->isClosedLoop());
		segment->setLine(line, coreSize);
	}

	outputDislocations->notifyDependents(ReferenceEvent::TargetChanged);
	state.replaceObject(inputDislocations, outputDislocations);
	return PipelineStatus::Success;
}

/******************************************************************************
* Removes some of the sampling points from a dislocation line.
******************************************************************************/
void SmoothDislocationsModifier::coarsenDislocationLine(FloatType linePointInterval, const QVector<Point3>& input, const QVector<int>& coreSize, QVector<Point3>& output, QVector<int>& outputCoreSize, bool isLoop)
{
	OVITO_ASSERT(input.size() >= 2);
	OVITO_ASSERT(input.size() == coreSize.size());

	if(linePointInterval <= 0) {
		output = input;
		outputCoreSize = coreSize;
		return;
	}

	// Always keep the end points.
	output.push_back(input.front());
	outputCoreSize.push_back(coreSize.front());

	QVector<Point3>::const_iterator inputPtr = input.constBegin();
	QVector<int>::const_iterator inputCoreSizePtr = coreSize.constBegin();

	int sum;
	int count;

	sum = count = 0;
	do {
		sum += *inputCoreSizePtr;
		count++;
		++inputPtr;
		++inputCoreSizePtr;
	}
	while(2*count*count < (int)(linePointInterval * sum) && count < input.size()/4);

	do {
		sum = count = 0;
		Vector3 com = Vector3::Zero();
		do {
			sum += *inputCoreSizePtr++;
			com.x() += inputPtr->x();
			com.y() += inputPtr->y();
			com.z() += inputPtr->z();
			count++;
			++inputPtr;
		}
		while(count*count < (int)(linePointInterval * sum) && count < input.size()/4 && inputPtr < input.constEnd()-1);
		output.push_back(Point3::Origin() + com / count);
		outputCoreSize.push_back(sum / count);
	}
	while(inputPtr < input.constEnd() - 1);

	// Always keep the end points.
	output.push_back(input.back());
	outputCoreSize.push_back(coreSize.back());

	OVITO_ASSERT(output.size() >= 2);
	OVITO_ASSERT(!isLoop || output.size() >= 3);
}

/******************************************************************************
* Smoothes the sampling points of a dislocation line.
******************************************************************************/
void SmoothDislocationsModifier::smoothDislocationLine(int smoothingLevel, QVector<Point3>& line, bool isLoop)
{
	if(smoothingLevel <= 0)
		return;	// Nothing to do.

	if(line.size() <= 2)
		return;	// Nothing to do.

	// This is the 2d implementation of the mesh smoothing algorithm:
	//
	// Gabriel Taubin
	// A Signal Processing Approach To Fair Surface Design
	// In SIGGRAPH 95 Conference Proceedings, pages 351-358 (1995)

	FloatType k_PB = 0.1f;
	FloatType lambda = 0.5f;
	FloatType mu = 1.0f / (k_PB - 1.0f/lambda);
	const FloatType prefactors[2] = { lambda, mu };

	std::vector<Vector3> laplacians(line.size());
	for(int iteration = 0; iteration < smoothingLevel; iteration++) {

		for(int pass = 0; pass <= 1; pass++) {
			// Compute discrete Laplacian for each point.
			auto l = laplacians.begin();
			if(isLoop == false)
				(*l++).setZero();
			else
				(*l++) = ((*(line.end()-2) - *(line.end()-3)) + (*(line.begin()+1) - line.front())) * FloatType(0.5);

			QVector<Point3>::const_iterator p1 = line.begin();
			QVector<Point3>::const_iterator p2 = line.begin() + 1;
			for(;;) {
				QVector<Point3>::const_iterator p0 = p1;
				++p1;
				++p2;
				if(p2 == line.end())
					break;
				*l++ = ((*p0 - *p1) + (*p2 - *p1)) * FloatType(0.5);
			}

			*l++ = laplacians.front();
			OVITO_ASSERT(l == laplacians.end());

			auto lc = laplacians.cbegin();
			for(Point3& p : line) {
				p += prefactors[pass] * (*lc++);
			}
		}
	}
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SmoothDislocationsModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the first rollout.
	QWidget* rollout = createRollout(tr("Smooth dislocations"), rolloutParams);

    QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);

	BooleanGroupBoxParameterUI* smoothingEnabledUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(SmoothDislocationsModifier::_smoothingEnabled));
	smoothingEnabledUI->groupBox()->setTitle(tr("Smoothing"));
    QGridLayout* sublayout = new QGridLayout(smoothingEnabledUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setColumnStretch(1, 1);
	layout->addWidget(smoothingEnabledUI->groupBox());

	IntegerParameterUI* smoothingLevelUI = new IntegerParameterUI(this, PROPERTY_FIELD(SmoothDislocationsModifier::_smoothingLevel));
	sublayout->addWidget(smoothingLevelUI->label(), 0, 0);
	sublayout->addLayout(smoothingLevelUI->createFieldLayout(), 0, 1);
	smoothingLevelUI->setMinValue(0);

	BooleanGroupBoxParameterUI* coarseningEnabledUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(SmoothDislocationsModifier::_coarseningEnabled));
	coarseningEnabledUI->groupBox()->setTitle(tr("Coarsening"));
    sublayout = new QGridLayout(coarseningEnabledUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setColumnStretch(1, 1);
	layout->addWidget(coarseningEnabledUI->groupBox());

	FloatParameterUI* linePointIntervalUI = new FloatParameterUI(this, PROPERTY_FIELD(SmoothDislocationsModifier::_linePointInterval));
	sublayout->addWidget(linePointIntervalUI->label(), 0, 0);
	sublayout->addLayout(linePointIntervalUI->createFieldLayout(), 0, 1);
	linePointIntervalUI->setMinValue(0);
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
