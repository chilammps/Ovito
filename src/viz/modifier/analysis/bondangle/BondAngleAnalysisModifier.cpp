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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>
#include <core/gui/properties/BooleanParameterUI.h>

#include "BondAngleAnalysisModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, BondAngleAnalysisModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, BondAngleAnalysisModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(BondAngleAnalysisModifier, BondAngleAnalysisModifierEditor)
DEFINE_VECTOR_REFERENCE_FIELD(BondAngleAnalysisModifier, _structureTypes, "StructureTypes", ParticleType)
DEFINE_REFERENCE_FIELD(BondAngleAnalysisModifier, _outputProperty, "OutputProperty", ParticleTypeProperty)
DEFINE_PROPERTY_FIELD(BondAngleAnalysisModifier, _autoUpdate, "AutoUpdate")
SET_PROPERTY_FIELD_LABEL(BondAngleAnalysisModifier, _structureTypes, "Structure types")
SET_PROPERTY_FIELD_LABEL(BondAngleAnalysisModifier, _autoUpdate, "Automatic update")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
BondAngleAnalysisModifier::BondAngleAnalysisModifier() : _autoUpdate(true)
{
	INIT_PROPERTY_FIELD(BondAngleAnalysisModifier::_structureTypes);
	INIT_PROPERTY_FIELD(BondAngleAnalysisModifier::_outputProperty);
	INIT_PROPERTY_FIELD(BondAngleAnalysisModifier::_autoUpdate);

	// Create the internal structure types.

	OORef<ParticleType> fccType(new ParticleType());
	fccType->setName(tr("FCC"));
	fccType->setColor(Color(0.4f, 1.0f, 0.4f));
	_structureTypes.push_back(fccType);

	OORef<ParticleType> hcpType(new ParticleType());
	hcpType->setName(tr("HCP"));
	hcpType->setColor(Color(1.0f, 0.4f, 0.4f));
	_structureTypes.push_back(hcpType);

	OORef<ParticleType> bccType(new ParticleType());
	bccType->setName(tr("BCC"));
	bccType->setColor(Color(0.4f, 0.4f, 1.0f));
	_structureTypes.push_back(bccType);

	OORef<ParticleType> icosahedralType(new ParticleType());
	icosahedralType->setName(tr("Icosahedral"));
	icosahedralType->setColor(Color(0.2f, 1.0f, 1.0f));
	_structureTypes.push_back(icosahedralType);

	OORef<ParticleType> noneType(new ParticleType());
	noneType->setName(tr("Other"));
	noneType->setColor(Color(0.95f, 0.95f, 0.95f));
	_structureTypes.push_back(noneType);

	// Create an (initially empty) property storage for the analysis results.
	_outputProperty = static_object_cast<ParticleTypeProperty>(ParticlePropertyObject::create(0, ParticleProperty::StructureTypeProperty));
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval BondAngleAnalysisModifier::modifierValidity(TimePoint time)
{
	// Return an empty validity interval if the modifier is currently being edited
	// to let the system create a pipeline cache point just before the modifier.
	// This will speed up re-evaluation of the pipeline if the user adjusts this modifier's parameters interactively.
	if(isBeingEdited())
		return TimeInterval::empty();

	return TimeInterval::forever();
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus BondAngleAnalysisModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	return ObjectStatus();
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void BondAngleAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Bond angle analysis"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
#ifndef Q_WS_MAC
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(0);
#endif

	BooleanParameterUI* autoUpdateUI = new BooleanParameterUI(this, PROPERTY_FIELD(BondAngleAnalysisModifier::_autoUpdate));
	layout1->addWidget(autoUpdateUI->checkBox());

	BooleanParameterUI* saveResultsUI = new BooleanParameterUI(this, "storeResultsWithScene", tr("Save results in scene file"));
	layout1->addWidget(saveResultsUI->checkBox());

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	// Derive a custom class from the list parameter UI to
	// give the items a color.
	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:
		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField)
			: RefTargetListParameterUI(parentEditor, refField, RolloutInsertionParameters(), NULL) {}
	protected:
		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			if(role == Qt::DecorationRole && target) {
				return (QColor)static_object_cast<ParticleType>(target)->color();
			}
			else return RefTargetListParameterUI::getItemData(target, index, role);
		}
		/// Do not open sub-editor for selected atom type.
		virtual void openSubEditor() override {}
	};

	_structureTypesPUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(BondAngleAnalysisModifier::_structureTypes));
	layout1->addSpacing(10);
	layout1->addWidget(new QLabel(tr("Structure types:")));
	layout1->addWidget(_structureTypesPUI->listWidget(150));
	connect(_structureTypesPUI->listWidget(), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onDoubleClickStructureType(const QModelIndex&)));
}

/******************************************************************************
* Is called when the user has double-clicked on one of the structure
* types in the list widget.
******************************************************************************/
void BondAngleAnalysisModifierEditor::onDoubleClickStructureType(const QModelIndex& index)
{
	// Let the user select a color for the structure type.
	ParticleType* stype = static_object_cast<ParticleType>(_structureTypesPUI->selectedObject());
	if(!stype) return;

	QColor oldColor = (QColor)stype->color();
	QColor newColor = QColorDialog::getColor(oldColor, container());
	if(!newColor.isValid() || newColor == oldColor) return;

	UndoManager::instance().beginCompoundOperation(tr("Change color"));
	stype->setColor(Color(newColor));
	UndoManager::instance().endCompoundOperation();
}

};	// End of namespace
