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
#include "FreezeSelectionModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, FreezeSelectionModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, FreezeSelectionModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(FreezeSelectionModifier, FreezeSelectionModifierEditor)

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus FreezeSelectionModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	ParticlePropertyObject* identifierProperty = inputStandardProperty(ParticleProperty::IdentifierProperty);

	size_t nselected = 0;
	if(!identifierProperty) {

		// When not using particle identifiers, the number of particles may not change.
		if(inputParticleCount() != selectionSnapshot().size())
			throw Exception(tr("Cannot restore saved selection. The number of particles has changed since the selection snapshot was taken."));

		// Restore selection simply by placing the snapshot into the pipeline.
		ParticlePropertyObject* selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);
		selProperty->replaceStorage(_selectionProperty.data());

		nselected = inputParticleCount() - std::count(_selectionProperty->constDataInt(), _selectionProperty->constDataInt() + _selectionProperty->size(), 0);
	}
	else {

		ParticlePropertyObject* selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);
		OVITO_ASSERT(selProperty->size() == identifierProperty->size());

		const int* id = identifierProperty->constDataInt();
		const int* id_end = id + identifierProperty->size();
		int* s = selProperty->dataInt();
		for(; id != id_end; ++id, ++s) {
			if(*s = std::binary_search(_selectedParticles.cbegin(), _selectedParticles.cend(), *id))
				nselected++;
		}
		selProperty->changed();
	}

	return ObjectStatus(ObjectStatus::Success, QString(), tr("%1 selected particles in stored selection set").arg(nselected));
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void FreezeSelectionModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Make a snapshot of the current selection when the modifier is created.
	PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
	takeSelectionSnapshot(input);
}

/******************************************************************************
* Takes a snapshot of the selection state.
******************************************************************************/
void FreezeSelectionModifier::takeSelectionSnapshot(const PipelineFlowState& state)
{
	// An undo stack record that restores the old selection snapshot.
	class ReplaceSelectionOperation : public UndoableOperation {
	public:
		ReplaceSelectionOperation(FreezeSelectionModifier* modifier) :
			_modifier(modifier), _selectionProperty(modifier->_selectionProperty), _selectedParticles(modifier->_selectedParticles) {}
		virtual void undo() override {
			_selectionProperty.swap(_modifier->_selectionProperty);
			_selectedParticles.swap(_modifier->_selectedParticles);
			_modifier->notifyDependents(ReferenceEvent::TargetChanged);
		}
		virtual void redo() override { undo(); }
	private:
		OORef<FreezeSelectionModifier> _modifier;
		QExplicitlySharedDataPointer<ParticleProperty> _selectionProperty;
		QVector<int> _selectedParticles;
	};

	// Take a snapshot of the current selection.
	for(const auto& o : state.objects()) {
		ParticlePropertyObject* selProperty = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(selProperty && selProperty->type() == ParticleProperty::SelectionProperty) {

			// Make a backup of the old snapshot so it can be restored.
			if(UndoManager::instance().isRecording())
				UndoManager::instance().push(new ReplaceSelectionOperation(this));

			// Take new snapshot.
			_selectionProperty = selProperty->storage();
			_selectedParticles.clear();

			// Save identifiers of selected particles in case the ordering of particle changes.
			for(const auto& o2 : state.objects()) {
				ParticlePropertyObject* identifierProperty = dynamic_object_cast<ParticlePropertyObject>(o2.get());
				if(identifierProperty && identifierProperty->type() == ParticleProperty::IdentifierProperty) {
					OVITO_ASSERT(identifierProperty->size() == selProperty->size());
					const int* id = identifierProperty->constDataInt();
					const int* id_end = id + identifierProperty->size();
					const int* s = selProperty->constDataInt();
					for(; id != id_end; ++id, ++s) {
						if(*s) _selectedParticles.push_back(*id);
					}
					// Sort identifiers.
					std::sort(_selectedParticles.begin(), _selectedParticles.end());
					break;
				}
			}

			notifyDependents(ReferenceEvent::TargetChanged);
			return;
		}
	}

	// Reset selection snapshot if input doesn't contain a selection state.
	if(_selectionProperty->size() != 0) {
		if(UndoManager::instance().isRecording())
			UndoManager::instance().push(new ReplaceSelectionOperation(this));
		_selectionProperty = new ParticleProperty(0, ParticleProperty::SelectionProperty);
		_selectedParticles.clear();
		notifyDependents(ReferenceEvent::TargetChanged);
	}
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void FreezeSelectionModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);

	stream.beginChunk(0x01);
	_selectionProperty->saveToStream(stream);
	stream << _selectedParticles;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void FreezeSelectionModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);

	stream.expectChunk(0x01);
	_selectionProperty->loadFromStream(stream);
	stream >> _selectedParticles;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> FreezeSelectionModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<FreezeSelectionModifier> clone = static_object_cast<FreezeSelectionModifier>(ParticleModifier::clone(deepCopy, cloneHelper));

	clone->_selectionProperty = this->_selectionProperty;
	clone->_selectedParticles = this->_selectedParticles;

	return clone;
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

	PipelineFlowState input = mod->getModifierInput();
	UndoManager::instance().beginCompoundOperation(tr("Take selection snapshot"));
	mod->takeSelectionSnapshot(input);
	UndoManager::instance().endCompoundOperation();
}

};	// End of namespace
