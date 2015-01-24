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

#ifndef __OVITO_CA_SHIFT_MODIFIER_H
#define __OVITO_CA_SHIFT_MODIFIER_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/objects/DataObject.h>
#include <core/animation/controller/Controller.h>
#include <core/gui/properties/PropertiesEditor.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/*
 * Displaces the dislocations and defect surface by a translation vector.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ShiftModifier : public Modifier
{
public:

	/// Constructor.
	Q_INVOKABLE ShiftModifier(DataSet* dataset);

	/// Asks the modifier whether it can be applied to the given input data.
	virtual bool isApplicableTo(const PipelineFlowState& input) override;

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override;

	/// This modifies the input object.
	virtual PipelineStatus modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state) override;

private:

	/// This property fields stores the translation vector.
	ReferenceField<Controller> _translation;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Shift");
	Q_CLASSINFO("ModifierCategory", "Crystal analysis");

	DECLARE_REFERENCE_FIELD(_translation);
};

/**
 * Properties editor for the ShiftModifier class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT ShiftModifierEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ShiftModifierEditor() {}

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

#endif // __OVITO_CA_SHIFT_MODIFIER_H
