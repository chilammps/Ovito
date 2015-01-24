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
#include "DefaultTextPrimitive.h"
#include "NonInteractiveSceneRenderer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/******************************************************************************
* Returns true if the buffer is filled and can be rendered with the given renderer.
******************************************************************************/
bool DefaultTextPrimitive::isValid(SceneRenderer* renderer)
{
	// This buffer type works only in conjunction with a non-interactive renderer.
	return (qobject_cast<NonInteractiveSceneRenderer*>(renderer) != nullptr);
}

/******************************************************************************
* Renders the text string at the given location given in normalized
* viewport coordinates ([-1,+1] range).
******************************************************************************/
void DefaultTextPrimitive::renderViewport(SceneRenderer* renderer, const Point2& pos, int alignment)
{
	QSize imageSize = renderer->outputSize();
	Point2 windowPos((pos.x() + 1.0f) * imageSize.width() / 2, (-pos.y() + 1.0f) * imageSize.height() / 2);
	renderWindow(renderer, windowPos, alignment);
}

/******************************************************************************
* Renders the text string at the given 2D window (pixel) coordinates.
******************************************************************************/
void DefaultTextPrimitive::renderWindow(SceneRenderer* renderer, const Point2& pos, int alignment)
{
	NonInteractiveSceneRenderer* niRenderer = dynamic_object_cast<NonInteractiveSceneRenderer>(renderer);
	if(text().isEmpty() || !niRenderer || renderer->isPicking())
		return;

	niRenderer->renderText(*this, pos, alignment);
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
