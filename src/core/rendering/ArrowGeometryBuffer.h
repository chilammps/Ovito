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
 * \brief Abstract base class for buffer objects that store vector glyphs.
 */
class OVITO_CORE_EXPORT ArrowGeometryBuffer : public OvitoObject
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

	enum Shape {
		CylinderShape,
		ArrowShape,
	};
	Q_ENUMS(Shape);

public:

	/// Constructor.
	ArrowGeometryBuffer(Shape shape, ShadingMode shadingMode, RenderingQuality renderingQuality) :
		_shape(shape), _shadingMode(shadingMode), _renderingQuality(renderingQuality) {}

	/// \brief Allocates a geometry buffer with the given number of elements.
	virtual void startSetElements(int elementCount) = 0;

	/// \brief Returns the number of elements stored in the buffer.
	virtual int elementCount() const = 0;

	/// \brief Sets the properties of a single element.
	virtual void setElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width) = 0;

	/// \brief Finalizes the geometry buffer after all elements have been set.
	virtual void endSetElements() = 0;

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer, quint32 pickingBaseID = 0) = 0;

	/// \brief Returns the shading mode for elements.
	ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Changes the shading mode for elements.
	/// \return false if the shading mode cannot be changed after the buffer has been created; true otherwise.
	virtual bool setShadingMode(ShadingMode mode) { _shadingMode = mode; return true; }

	/// \brief Returns the rendering quality of elements.
	RenderingQuality renderingQuality() const { return _renderingQuality; }

	/// \brief Changes the rendering quality of elements.
	/// \return false if the quality level cannot be changed after the buffer has been created; true otherwise.
	virtual bool setRenderingQuality(RenderingQuality level) { _renderingQuality = level; return true; }

	/// \brief Returns the selected element shape.
	Shape shape() const { return _shape; }

private:

	/// Controls the shading.
	ShadingMode _shadingMode;

	/// Controls the rendering quality.
	RenderingQuality _renderingQuality;

	/// The shape of the elements.
	Shape _shape;

	Q_OBJECT
	OVITO_OBJECT
};

};

Q_DECLARE_METATYPE(Ovito::ArrowGeometryBuffer::ShadingMode);
Q_DECLARE_METATYPE(Ovito::ArrowGeometryBuffer::RenderingQuality);
Q_DECLARE_METATYPE(Ovito::ArrowGeometryBuffer::Shape);
Q_DECLARE_TYPEINFO(Ovito::ArrowGeometryBuffer::ShadingMode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ArrowGeometryBuffer::RenderingQuality, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ArrowGeometryBuffer::Shape, Q_PRIMITIVE_TYPE);

#endif // __OVITO_ARROW_GEOMETRY_BUFFER_H
