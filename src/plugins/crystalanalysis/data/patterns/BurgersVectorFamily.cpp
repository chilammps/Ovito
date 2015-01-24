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
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include "BurgersVectorFamily.h"

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, BurgersVectorFamily, RefTarget);
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, BurgersVectorFamilyEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(BurgersVectorFamily, BurgersVectorFamilyEditor);
DEFINE_PROPERTY_FIELD(BurgersVectorFamily, _name, "Name");
DEFINE_PROPERTY_FIELD(BurgersVectorFamily, _color, "Color");
DEFINE_PROPERTY_FIELD(BurgersVectorFamily, _isVisible, "Visible");
DEFINE_PROPERTY_FIELD(BurgersVectorFamily, _burgersVector, "BurgersVector");
SET_PROPERTY_FIELD_LABEL(BurgersVectorFamily, _name, "Name");
SET_PROPERTY_FIELD_LABEL(BurgersVectorFamily, _color, "Color");
SET_PROPERTY_FIELD_LABEL(BurgersVectorFamily, _isVisible, "Visible");
SET_PROPERTY_FIELD_LABEL(BurgersVectorFamily, _burgersVector, "Burgers vector");

/******************************************************************************
* Constructs a new BurgersVectorFamily.
******************************************************************************/
BurgersVectorFamily::BurgersVectorFamily(DataSet* dataset) : RefTarget(dataset), _isVisible(true)
{
	INIT_PROPERTY_FIELD(BurgersVectorFamily::_name);
	INIT_PROPERTY_FIELD(BurgersVectorFamily::_color);
	INIT_PROPERTY_FIELD(BurgersVectorFamily::_isVisible);
	INIT_PROPERTY_FIELD(BurgersVectorFamily::_burgersVector);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void BurgersVectorFamilyEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Burgers vector family"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout1 = new QGridLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(4);
	layout1->setColumnStretch(1, 1);
	
	// Text box for the name of atom type.
	StringParameterUI* namePUI = new StringParameterUI(this, PROPERTY_FIELD(BurgersVectorFamily::_name));
	layout1->addWidget(new QLabel(tr("Name:")), 0, 0);
	layout1->addWidget(namePUI->textBox(), 0, 1);
	
	// Color parameter.
	ColorParameterUI* colorPUI = new ColorParameterUI(this, PROPERTY_FIELD(BurgersVectorFamily::_color));
	layout1->addWidget(colorPUI->label(), 1, 0);
	layout1->addWidget(colorPUI->colorPicker(), 1, 1);
}

}	// End of namespace
}	// End of namespace
}	// End of namespace
