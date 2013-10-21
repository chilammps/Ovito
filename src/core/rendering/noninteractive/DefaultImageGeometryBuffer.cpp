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
#include "DefaultImageGeometryBuffer.h"
#include "NonInteractiveSceneRenderer.h"

namespace Ovito {

IMPLEMENT_OVITO_OBJECT(Core, DefaultImageGeometryBuffer, ImageGeometryBuffer);

/******************************************************************************
* Returns true if the buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool DefaultImageGeometryBuffer::isValid(SceneRenderer* renderer)
{
	// This buffer type works only in conjunction with a non-interactive renderer.
	return (qobject_cast<NonInteractiveSceneRenderer*>(renderer) != nullptr);
}

/******************************************************************************
* Renders the image in a rectangle given in viewport coordinates.
******************************************************************************/
void DefaultImageGeometryBuffer::renderViewport(SceneRenderer* renderer, const Point2& pos, const Vector2& size)
{
	NonInteractiveSceneRenderer* niRenderer = dynamic_object_cast<NonInteractiveSceneRenderer>(renderer);
	if(image().isNull() || !niRenderer || renderer->isPicking())
		return;
}

/******************************************************************************
* Renders the image in a rectangle given in window coordinates.
******************************************************************************/
void DefaultImageGeometryBuffer::renderWindow(SceneRenderer* renderer, const Point2& pos, const Vector2& size)
{
	NonInteractiveSceneRenderer* niRenderer = dynamic_object_cast<NonInteractiveSceneRenderer>(renderer);
	if(image().isNull() || !niRenderer || renderer->isPicking())
		return;
}

};
