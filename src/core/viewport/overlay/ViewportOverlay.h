///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_VIEWPORT_OVERLAY_H
#define __OVITO_VIEWPORT_OVERLAY_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/scene/pipeline/PipelineStatus.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View)

/**
 * \brief Abstract base class for all viewport overlays.
 */
class OVITO_CORE_EXPORT ViewportOverlay : public RefTarget
{
protected:

	/// \brief Constructor.
	ViewportOverlay(DataSet* dataset);

public:

	/// \brief This method asks the overlay to paint its contents over the given viewport.
	virtual void render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings) = 0;

	/// \brief Returns the status of the object, which may indicate an error condition.
	///
	/// The default implementation of this method returns an empty status object.
	/// The object should generate a ReferenceEvent::ObjectStatusChanged event when its status changes.
	virtual PipelineStatus status() const { return PipelineStatus(); }

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_OVERLAY_H
