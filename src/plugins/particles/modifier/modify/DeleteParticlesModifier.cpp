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

#include <plugins/particles/Particles.h>
#include "DeleteParticlesModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, DeleteParticlesModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(DeleteParticlesModifier, DeleteParticlesModifierEditor);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, DeleteParticlesModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
PipelineStatus DeleteParticlesModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	QString statusMessage = tr("%n input particles", 0, inputParticleCount());

	// Get the selection.
	ParticlePropertyObject* selProperty = expectStandardProperty(ParticleProperty::SelectionProperty);
	OVITO_ASSERT(selProperty->size() == inputParticleCount());

	// Compute filter mask.
	size_t numRejected = 0;
	boost::dynamic_bitset<> mask(inputParticleCount());
	const int* s = selProperty->constDataInt();
	const int* s_end = s + selProperty->size();
	boost::dynamic_bitset<>::size_type i = 0;
	for(; s != s_end; ++s, ++i) {
		if(*s) {
			mask.set(i);
			numRejected++;
		}
		else mask.reset(i);
	}

	// Remove selection property.
	removeOutputProperty(selProperty);

	// Delete the particles.
	deleteParticles(mask, numRejected);

	statusMessage += tr("\n%n particles deleted (%1%)", 0, numRejected).arg(numRejected * 100 / std::max((int)inputParticleCount(), 1));
	return PipelineStatus(PipelineStatus::Success, statusMessage);
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void DeleteParticlesModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Delete selected particles"), rolloutParams, "particles.modifiers.delete_selected_particles.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);

	// Status label.
	layout->addWidget(statusLabel());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
