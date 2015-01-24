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

#ifndef __OVITO_ARROW_PRIMITIVE_H
#define __OVITO_ARROW_PRIMITIVE_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include "PrimitiveBase.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Abstract base class for rendering arrow glyphs and cylinders.
 */
class OVITO_CORE_EXPORT ArrowPrimitive : public PrimitiveBase
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
	ArrowPrimitive(Shape shape, ShadingMode shadingMode, RenderingQuality renderingQuality) :
		_shape(shape), _shadingMode(shadingMode), _renderingQuality(renderingQuality) {}

	/// \brief Allocates a geometry buffer with the given number of elements.
	virtual void startSetElements(int elementCount) = 0;

	/// \brief Returns the number of elements stored in the buffer.
	virtual int elementCount() const = 0;

	/// \brief Sets the properties of a single element.
	virtual void setElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width) = 0;

	/// \brief Finalizes the geometry buffer after all elements have been set.
	virtual void endSetElements() = 0;

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

};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::ArrowPrimitive::ShadingMode);
Q_DECLARE_METATYPE(Ovito::ArrowPrimitive::RenderingQuality);
Q_DECLARE_METATYPE(Ovito::ArrowPrimitive::Shape);
Q_DECLARE_TYPEINFO(Ovito::ArrowPrimitive::ShadingMode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ArrowPrimitive::RenderingQuality, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(Ovito::ArrowPrimitive::Shape, Q_PRIMITIVE_TYPE);

#endif // __OVITO_ARROW_PRIMITIVE_H
