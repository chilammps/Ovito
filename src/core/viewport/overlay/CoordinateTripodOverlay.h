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

#ifndef __OVITO_COORDINATE_TRIPOD_OVERLAY_H
#define __OVITO_COORDINATE_TRIPOD_OVERLAY_H

#include <core/Core.h>
#include "ViewportOverlay.h"

namespace Ovito {

/**
 * \brief A viewport overlay that displays the coordinate system orientation.
 */
class OVITO_CORE_EXPORT CoordinateTripodOverlay : public ViewportOverlay
{
public:

	/// \brief Constructor.
	Q_INVOKABLE CoordinateTripodOverlay(DataSet* dataset);

	/// \brief This method asks the overlay to paint its contents over the given viewport.
	virtual void render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings) override;

private:

	Q_CLASSINFO("DisplayName", "Coordinate tripod");

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_COORDINATE_TRIPOD_OVERLAY_H
