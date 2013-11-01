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
#include "DeleteParticlesModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, DeleteParticlesModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Particles, DeleteParticlesModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(DeleteParticlesModifier, DeleteParticlesModifierEditor)

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
ObjectStatus DeleteParticlesModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	QString statusMessage = tr("%n input particles", 0, inputParticleCount());

	// Get the selection.
	ParticlePropertyObject* selProperty = expectStandardProperty(ParticleProperty::SelectionProperty);
	OVITO_ASSERT(selProperty->size() == inputParticleCount());

	// Compute filter mask.
	size_t numRejected = 0;
	std::vector<bool> mask(inputParticleCount());
	const int* s = selProperty->constDataInt();
	const int* s_end = s + selProperty->size();
	auto m = mask.begin();
	for(; s != s_end; ++s, ++m) {
		if(*s) {
			*m = true;
			numRejected++;
		}
		else
			*m = false;
	}

	// Remove selection property.
	removeOutputProperty(selProperty);

	// Delete the particles.
	deleteParticles(mask, numRejected);

	statusMessage += tr("\n%n particles deleted (%1%)", 0, numRejected).arg(numRejected * 100 / std::max((int)inputParticleCount(), 1));
	return ObjectStatus(ObjectStatus::Success, statusMessage);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void DeleteParticlesModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Delete selected particles"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(0);

	// Status label.
	layout->addWidget(statusLabel());
}

};	// End of namespace
