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

#ifndef __OVITO_DEFAULT_ARROW_GEOMETRY_BUFFER_H
#define __OVITO_DEFAULT_ARROW_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/ArrowPrimitive.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Buffer object that stores a set of arrows to be rendered by a non-interactive renderer.
 */
class OVITO_CORE_EXPORT DefaultArrowPrimitive : public ArrowPrimitive
{
public:

	/// Data structure that stores the data of a single arrow element.
	struct ArrowElement {
		Point3 pos;
		Vector3 dir;
		ColorA color;
		FloatType width;
	};

public:

	/// Constructor.
	DefaultArrowPrimitive(ArrowPrimitive::Shape shape, ShadingMode shadingMode, RenderingQuality renderingQuality) :
		ArrowPrimitive(shape, shadingMode, renderingQuality) {}

	/// \brief Allocates a geometry buffer with the given number of elements.
	virtual void startSetElements(int elementCount) override {
		OVITO_ASSERT(elementCount >= 0);
		_elements.resize(elementCount);
	}

	/// \brief Returns the number of elements stored in the buffer.
	virtual int elementCount() const override { return _elements.size(); }

	/// \brief Sets the properties of a single line element.
	virtual void setElement(int index, const Point3& pos, const Vector3& dir, const ColorA& color, FloatType width) override;

	/// \brief Finalizes the geometry buffer after all elements have been set.
	virtual void endSetElements() override {}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

	/// Returns a reference to the internal buffer that stores the arrow geometry.
	const std::vector<ArrowElement>& elements() const { return _elements; }

private:

	/// The internal memory buffer for arrow elements.
	std::vector<ArrowElement> _elements;

};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_VIEWPORT_ARROW_GEOMETRY_BUFFER_H
