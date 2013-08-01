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
 * \file ImageGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ImageGeometryBuffer class.
 */

#ifndef __OVITO_IMAGE_GEOMETRY_BUFFER_H
#define __OVITO_IMAGE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito {

class SceneRenderer;			// defined in SceneRenderer.h

/**
 * \brief Abstract base class for buffer objects that store images.
 */
class OVITO_CORE_EXPORT ImageGeometryBuffer : public OvitoObject
{
public:

	/// \brief Sets the text to be rendered.
	virtual void setImage(const QImage& image) { _image = image; }

	/// \brief Returns the image stored in the buffer.
	const QImage& image() const { return _image; }

	/// \brief Returns true if the buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the image in a rectangle given in pixel coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, const Vector2& size) = 0;

	/// \brief Renders the image in a rectangle given in viewport coordinates.
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, const Vector2& size) = 0;

private:

	/// The image to be rendered.
	QImage _image;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_IMAGE_GEOMETRY_BUFFER_H
