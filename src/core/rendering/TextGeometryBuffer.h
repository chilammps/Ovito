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
 * \file TextGeometryBuffer.h
 * \brief Contains the definition of the Ovito::TextGeometryBuffer class.
 */

#ifndef __OVITO_TEXT_GEOMETRY_BUFFER_H
#define __OVITO_TEXT_GEOMETRY_BUFFER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>

namespace Ovito {

class SceneRenderer;			// defined in SceneRenderer.h

/**
 * \brief Abstract base class for buffer objects that store text strings.
 */
class TextGeometryBuffer : public OvitoObject
{
public:

	/// \brief Default constructor.
	TextGeometryBuffer() : _color(ColorA(1,1,1,1)) {}

	/// \brief Sets the text to be rendered.
	virtual void setText(const QString& text) { _text = text; }

	/// \brief Returns the number of vertices stored in the buffer.
	const QString& text() const { return _text; }

	/// \brief Sets the text color.
	virtual void setColor(const ColorA& color) { _color = color; }

	/// \brief Returns the text color.
	const ColorA& color() const { return _color; }

	/// Sets the text font.
	virtual void setFont(const QFont& font) { _font = font; }

	/// Returns the text font.
	const QFont& font() const { return _font; }

	/// \brief Returns true if the buffer is filled and can be rendered with the given renderer.
	virtual bool isValid(SceneRenderer* renderer) = 0;

	/// \brief Renders the text string at the given 2D location.
	virtual void render(SceneRenderer* renderer, const Point2& pos) = 0;

private:

	/// The text to be rendered.
	QString _text;

	/// The text color.
	ColorA _color;

	/// The text font.
	QFont _font;

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_TEXT_GEOMETRY_BUFFER_H
