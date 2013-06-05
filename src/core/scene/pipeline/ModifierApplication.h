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

/**
 * \file ModifierApplication.h
 * \brief Contains the definition of the Ovito::ModifierApplication class.
 */

#ifndef __OVITO_MODIFIER_APPLICATION_H
#define __OVITO_MODIFIER_APPLICATION_H

#include <core/Core.h>
#include "Modifier.h"

namespace Ovito {

class PipelineObject;	// defined in PipelineObject.h

/**
 * \brief Stores information about a particular application of a Modifier
 *        instance in a geometry pipeline.
 */
class ModifierApplication : public RefTarget
{
public:

	/// \brief Constructs an application object for a given Modifier instance.
	/// \param modifier The modifier that is going to be applied in a geometry pipeline.
	Q_INVOKABLE ModifierApplication(Modifier* modifier = NULL);

	/// \brief Returns the modifier instance that is applied in a particular geometry pipeline.
	/// \return The modifier instance.
	Modifier* modifier() const { return _modifier; }

	/// \brief Returns the geometry pipeline in which the modifier is used.
	/// \return The PipelineObject this application is part of.
	PipelineObject* pipelineObject() const;

	/// \brief Returns whether this modifier application is currently enabled.
	/// \return \c true if it is currently enabled, i.e. applied.
	///         \c false if it is disabled and skipped in the geometry pipeline.
	bool isEnabled() const { return _isEnabled; }

	/// \brief Enables or disables this modifier application.
	/// \param enabled Controls the state of the modifier.
	///
	/// A disabled modifier application is skipped in the geometry pipeline
	/// and its modifier is not applied to the input object.
	///
	/// \undoable
	void setEnabled(bool enabled) { _isEnabled = enabled; }

	/// \brief Return the status returned by the modifier during its last evaluation.
	const EvaluationStatus& status() const { return _evalStatus; }

	/// \brief Stores the status of this modifier application.
	/// \note This is an internal function.
	void setStatus(const EvaluationStatus& status);

public:

	Q_PROPERTY(bool isEnabled READ isEnabled WRITE setEnabled)

private:

	/// The modifier that is being applied.
	ReferenceField<Modifier> _modifier;

	/// The status returned by the modifier during its last evaluation.
	EvaluationStatus _evalStatus;

	/// Flag that indicates whether the modifier application is enabled.
	PropertyField<bool, bool, ReferenceEvent::TargetEnabledOrDisabled> _isEnabled;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_modifier);
	DECLARE_PROPERTY_FIELD(_isEnabled);
};


};

#endif // __OVITO_MODIFIER_APPLICATION_H
