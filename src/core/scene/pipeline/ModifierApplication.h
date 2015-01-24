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

#ifndef __OVITO_MODIFIER_APPLICATION_H
#define __OVITO_MODIFIER_APPLICATION_H

#include <core/Core.h>
#include "Modifier.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief Stores information about an application of a Modifier
 *        in a modification pipeline.
 *
 * Modifiers can be shared by multiple modification pipelines.
 * For every use of a Modifier instance in a pipeline, a ModifierApplication
 * is created, which links the Modifier to the PipelineObject.
 *
 * Modifiers can store custom data in the ModifierApplication, which are
 * specific to a particular pipeline.
 *
 * \sa Modifier
 * \sa PipelineObject
 */
class OVITO_CORE_EXPORT ModifierApplication : public RefTarget
{
public:

	/// \brief Constructs a modifierChanged for a Modifier instance.
	/// \param modifier The modifier that is going to be inserted into a modification pipeline.
	Q_INVOKABLE ModifierApplication(DataSet* dataset, Modifier* modifier = nullptr);

	/// \brief Returns the Modifier, which is connected by this ModifierApplication to a specific
	///        modification pipeline.
	/// \return The modifier instance.
	Modifier* modifier() const { return _modifier; }

	/// \brief Returns the modification pipeline the Modifier managed by this ModifierApplication is part of.
	/// \return The PipelineObject this ModifierApplication is referenced by.
	PipelineObject* pipelineObject() const;

	/// \brief Returns a list of ObjectNode instances that depend on this ModifierApplication.
	/// \return A list of scene nodes, all sharing the same PipelineObject, which references this
	///         ModifierApplication.
	QSet<ObjectNode*> objectNodes() const;

	/// \brief Returns the data managed by the modifier, which has been set via setModifierData().
	/// \return The data object previously set by the modifier.
	RefTarget* modifierData() const { return _modifierData; }

	/// \brief Sets or replaces the data object used by the modifier.
	/// \param data A subclass of RefTarget, created and managed by the Modifier.
	///
	/// Modifiers can use this method to store pipeline-specific data.
	void setModifierData(RefTarget* data) { _modifierData = data; }

protected:

	/// \brief Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override {
		// Forward enabled/disabled notification events from the Modifier.
		if(event->type() == ReferenceEvent::TargetEnabledOrDisabled)
			return true;
		return RefTarget::referenceEvent(source, event);
	}

private:

	/// The modifier that is inserted into the pipeline.
	ReferenceField<Modifier> _modifier;

	/// Optional data managed by the modifier.
	ReferenceField<RefTarget> _modifierData;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_modifier);
	DECLARE_REFERENCE_FIELD(_modifierData);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_MODIFIER_APPLICATION_H
