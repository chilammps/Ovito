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
#include <core/gui/undo/UndoManager.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <plugins/particles/data/ParticleSelectionSet.h>
#include "FreezeSelectionModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, FreezeSelectionModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Particles, FreezeSelectionModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(FreezeSelectionModifier, FreezeSelectionModifierEditor)

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus FreezeSelectionModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Retrieve the selection stored in the modifier application.
	ParticleSelectionSet* selectionSet = dynamic_object_cast<ParticleSelectionSet>(modifierApplication()->modifierData());
	if(!selectionSet)
		throw Exception(tr("No stored selection set available. Please take a new snapshot of the current selection state."));

	return selectionSet->applySelection(
			outputStandardProperty(ParticleProperty::SelectionProperty),
			inputStandardProperty(ParticleProperty::IdentifierProperty));
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void FreezeSelectionModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Take a snapshot of the existing selection state at the time the modifier is created.
	if(dynamic_object_cast<ParticleSelectionSet>(modApp->modifierData()) == nullptr) {
		PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
		takeSelectionSnapshot(modApp, input);
	}
}

/******************************************************************************
* Takes a snapshot of the selection state.
******************************************************************************/
void FreezeSelectionModifier::takeSelectionSnapshot(ModifierApplication* modApp, const PipelineFlowState& state)
{
	OORef<ParticleSelectionSet> selectionSet = dynamic_object_cast<ParticleSelectionSet>(modApp->modifierData());
	if(!selectionSet) {
		selectionSet = new ParticleSelectionSet();
		modApp->setModifierData(selectionSet.get());
	}
	selectionSet->resetSelection(state);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void FreezeSelectionModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	QWidget* rollout = createRollout(tr("Freeze selection"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QPushButton* takeSnapshotBtn = new QPushButton(tr("Take selection snapshot"), rollout);
	connect(takeSnapshotBtn, SIGNAL(clicked(bool)), this, SLOT(takeSelectionSnapshot()));
	layout->addWidget(takeSnapshotBtn);

	// Status label.
	layout->addSpacing(12);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* Takes a new snapshot of the current particle selection.
******************************************************************************/
void FreezeSelectionModifierEditor::takeSelectionSnapshot()
{
	FreezeSelectionModifier* mod = static_object_cast<FreezeSelectionModifier>(editObject());
	if(!mod) return;

	UndoableTransaction::handleExceptions(tr("Take selection snapshot"), [mod]() {
		for(const auto& modInput : mod->getModifierInputs())
			mod->takeSelectionSnapshot(modInput.first, modInput.second);
	});
}

};	// End of namespace
