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

#ifndef __OVITO_DEFAULT_LINE_GEOMETRY_BUFFER_H
#define __OVITO_DEFAULT_LINE_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/rendering/LinePrimitive.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Buffer object that stores line geometry to be rendered by a non-interactive renderer.
 */
class OVITO_CORE_EXPORT DefaultLinePrimitive : public LinePrimitive
{
public:

	/// Constructor.
	DefaultLinePrimitive() {}

	/// \brief Allocates a geometry buffer with the given number of vertices.
	virtual void setVertexCount(int vertexCount, FloatType lineWidth) override {
		OVITO_ASSERT(vertexCount >= 0);
		_positionsBuffer.resize(vertexCount);
		_colorsBuffer.resize(vertexCount);
	}

	/// \brief Returns the number of vertices stored in the buffer.
	virtual int vertexCount() const override { return _positionsBuffer.size(); }

	/// \brief Sets the coordinates of the vertices.
	virtual void setVertexPositions(const Point3* coordinates) override {
		std::copy(coordinates, coordinates + _positionsBuffer.size(), _positionsBuffer.begin());
	}

	/// \brief Sets the colors of the vertices.
	virtual void setVertexColors(const ColorA* colors) override {
		std::copy(colors, colors + _colorsBuffer.size(), _colorsBuffer.begin());
	}

	/// \brief Sets the color of all vertices to the given value.
	virtual void setLineColor(const ColorA color) override {
		std::fill(_colorsBuffer.begin(), _colorsBuffer.end(), color);
	}

	/// \brief Returns true if the geometry buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) override;

	/// \brief Renders the geometry.
	virtual void render(SceneRenderer* renderer) override;

	/// Returns a reference to the internal buffer that stores the vertex positions.
	const std::vector<Point3>& positions() const { return _positionsBuffer; }

		/// Returns a reference to the internal buffer that stores the vertex colors.
	const std::vector<ColorA>& colors() const { return _colorsBuffer; }

private:

	/// The buffer that stores the vertex positions.
	std::vector<Point3> _positionsBuffer;

	/// The buffer that stores the vertex colors.
	std::vector<ColorA> _colorsBuffer;

};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_DEFAULT_LINE_GEOMETRY_BUFFER_H
