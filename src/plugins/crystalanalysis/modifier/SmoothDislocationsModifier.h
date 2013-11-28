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

#ifndef __OVITO_CA_SMOOTH_DISLOCATIONS_MODIFIER_H
#define __OVITO_CA_SMOOTH_DISLOCATIONS_MODIFIER_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/gui/properties/PropertiesEditor.h>

namespace CrystalAnalysis {

using namespace Ovito;
using namespace Particles;

/*
 * Smoothes the dislocation lines.
 */
class OVITO_CRYSTALANALYSIS_EXPORT SmoothDislocationsModifier : public Modifier
{
public:

	/// Constructor.
	Q_INVOKABLE SmoothDislocationsModifier();

	/// Asks the modifier whether it can be applied to the given input data.
	virtual bool isApplicableTo(const PipelineFlowState& input) override;

	/// This modifies the input object.
	virtual ObjectStatus modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state) override;

protected:

	/// Removes some of the sampling points from a dislocation line.
	void coarsenDislocationLine(FloatType linePointInterval, const QVector<Point3>& input, const QVector<int>& coreSize, QVector<Point3>& output, QVector<int>& outputCoreSize, bool isLoop);

	/// Smoothes the sampling points of a dislocation line.
	void smoothDislocationLine(int smoothingLevel, QVector<Point3>& line, bool isLoop);

private:

	/// Stores whether smoothing is enabled.
	PropertyField<bool> _smoothingEnabled;

	/// Controls the amount of smoothing.
	PropertyField<int> _smoothingLevel;

	/// Stores whether coarsening is enabled.
	PropertyField<bool> _coarseningEnabled;

	/// Controls the coarsening of dislocation lines.
	PropertyField<FloatType> _linePointInterval;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Smooth dislocations");
	Q_CLASSINFO("ModifierCategory", "Crystal analysis");

	DECLARE_PROPERTY_FIELD(_smoothingEnabled);
	DECLARE_PROPERTY_FIELD(_smoothingLevel);
	DECLARE_PROPERTY_FIELD(_coarseningEnabled);
	DECLARE_PROPERTY_FIELD(_linePointInterval);
};

/**
 * Properties editor for the SmoothDislocationsModifier class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT SmoothDislocationsModifierEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE SmoothDislocationsModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_CA_SMOOTH_DISLOCATIONS_MODIFIER_H
