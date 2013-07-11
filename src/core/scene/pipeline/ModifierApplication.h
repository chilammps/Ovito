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
	Q_INVOKABLE ModifierApplication(Modifier* modifier = nullptr);

	/// \brief Returns the modifier instance that is applied in a particular geometry pipeline.
	/// \return The modifier instance.
	Modifier* modifier() const { return _modifier; }

	/// \brief Returns the geometry pipeline in which the modifier is used.
	/// \return The PipelineObject this application is part of.
	PipelineObject* pipelineObject() const;

private:

	/// The modifier that is being applied.
	ReferenceField<Modifier> _modifier;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_modifier);
};


};

#endif // __OVITO_MODIFIER_APPLICATION_H
