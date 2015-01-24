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

#ifndef __OVITO_CA_SMOOTH_SURFACE_MODIFIER_H
#define __OVITO_CA_SMOOTH_SURFACE_MODIFIER_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/utilities/mesh/HalfEdgeMesh.h>
#include <core/gui/properties/PropertiesEditor.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/*
 * Smoothes and fairs the defect surface mesh..
 */
class OVITO_CRYSTALANALYSIS_EXPORT SmoothSurfaceModifier : public Modifier
{
public:

	/// Constructor.
	Q_INVOKABLE SmoothSurfaceModifier(DataSet* dataset);

	/// Asks the modifier whether it can be applied to the given input data.
	virtual bool isApplicableTo(const PipelineFlowState& input) override;

	/// This modifies the input object.
	virtual PipelineStatus modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state) override;

	/// Returns the smoothing strength.
	int smoothingLevel() const { return _smoothingLevel; }

	/// Sets the smoothing strength.
	void setSmoothingLevel(int level) { _smoothingLevel = level; }

private:

	/// Controls the amount of smoothing.
	PropertyField<int> _smoothingLevel;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Smooth surface");
	Q_CLASSINFO("ModifierCategory", "Crystal analysis");

	DECLARE_PROPERTY_FIELD(_smoothingLevel);
};

/**
 * Properties editor for the SmoothSurfaceModifier class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT SmoothSurfaceModifierEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE SmoothSurfaceModifierEditor() {}

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

#endif // __OVITO_CA_SMOOTH_SURFACE_MODIFIER_H
