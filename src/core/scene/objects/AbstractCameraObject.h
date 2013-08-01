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
 * \file AbstractCameraObject.h 
 * \brief Contains the definition of the Ovito::AbstractCameraObject class.
 */

#ifndef __OVITO_ABSTRACT_CAMERA_OBJECT_H
#define __OVITO_ABSTRACT_CAMERA_OBJECT_H

#include <core/Core.h>
#include "SceneObject.h"

namespace Ovito {
	
struct ViewProjectionParameters;	// defined in Viewport.h
	
/**
 * \brief Abstract base class for camera scene objects.
 */
class OVITO_CORE_EXPORT AbstractCameraObject : public SceneObject
{
public:
	
	/// \brief Default constructor.
	AbstractCameraObject() {}

	/// \brief Returns a structure that describes the camera.
	/// \param[in] time The animation time for which the camera's projection parameters should be returned.
	/// \param[in,out] projParams The structure that is to be filled with the projection parameters.
	///     The following fields of the ViewProjectionParameters structure are already filled in when
	///     the method is called:
	///   - ViewProjectionParameters::aspectRatio (The aspect ratio (height/width) of the viewport)
	///   - ViewProjectionParameters::viewMatrix (The world to view space transformation)
	///   - ViewProjectionParameters::znear (The distance to the bounding box of the scene in view space)
	///   - ViewProjectionParameters::zfar (The distance to the back side of the bounding box of the scene in view space)
	virtual void projectionParameters(TimePoint time, ViewProjectionParameters& projParams) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};


};

#endif // __OVITO_ABSTRACT_CAMERA_OBJECT_H
