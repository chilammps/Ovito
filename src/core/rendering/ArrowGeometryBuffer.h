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
 * \file ArrowGeometryBuffer.h
 * \brief Contains the definition of the Ovito::ArrowGeometryBuffer class.
 */

#ifndef __OVITO_ARROW_GEOMETRY_BUFFER_H
#define __OVITO_ARROW_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito {

class SceneRenderer;			// defined in SceneRenderer.h

/**
 * \brief Abstract base class for buffer objects that store arrows.
 */
class ArrowGeometryBuffer : public OvitoObject
{
public:

	enum ShadingMode {
		NormalShading,
		FlatShading,
	};
	Q_ENUMS(ShadingMode);

	enum RenderingQuality {
		LowQuality,
		MediumQuality,
		HighQuality
	};
	Q_ENUMS(RenderingQuality);

public:

	/// Constructor.
	ArrowGeometryBuffer(ShadingMode shadingMode, RenderingQuality renderingQuality) : _shadingMode(shadingMode), _renderingQuality(renderingQuality) {}

	/// \brief Allocates a geometry buffer with the given number of arrows.
	virtual void startSetArrows(int arrowCount) = 0;

	/// \brief Returns the number of arrows stored in the buffer.
	virtual int arrowCount() const = 0;

	/// \brief Sets the properties of a single arrow.
	virtual void setArrow(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width) = 0;

	/// \brief Finalizes the geometry buffer after all arrows have been set.
	virtual void endSetArrows() = 0;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer, quint32 pickingBaseID = 0) = 0;

	/// \brief Returns the shading mode for arrows.
	ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Changes the shading mode for arrows.
	/// \return false if the shading mode cannot be changed after the buffer has been created; true otherwise.
	virtual bool setShadingMode(ShadingMode mode) { _shadingMode = mode; return true; }

	/// \brief Returns the rendering quality of arrows.
	RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Changes the rendering quality of arrows.
	/// \return false if the quality level cannot be changed after the buffer has been created; true otherwise.
	virtual bool setRenderingQuality(RenderingQuality level) { _renderingQuality = level; return true; }

private:

	/// Controls the shading of arrows.
	ShadingMode _shadingMode;

	/// Controls the rendering quality of arrows.
	RenderingQuality _renderingQuality;

	Q_OBJECT
	OVITO_OBJECT
};

};

Q_DECLARE_METATYPE(Ovito::ArrowGeometryBuffer::ShadingMode);
Q_DECLARE_METATYPE(Ovito::ArrowGeometryBuffer::RenderingQuality);
Q_DECLARE_TYPEINFO(Ovito::ArrowGeometryBuffer::ShadingMode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ArrowGeometryBuffer::RenderingQuality, Q_PRIMITIVE_TYPE);

#endif // __OVITO_ARROW_GEOMETRY_BUFFER_H
