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

#ifndef __OVITO_TEXT_PRIMITIVE_H
#define __OVITO_TEXT_PRIMITIVE_H

#include <core/Core.h>
#include "PrimitiveBase.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * \brief Abstract base class rendering of text primitives.
 */
class OVITO_CORE_EXPORT TextPrimitive : public PrimitiveBase
{
public:

	/// \brief Default constructor.
	TextPrimitive() : _color(1,1,1,1), _backgroundColor(0,0,0,0) {}

	/// \brief Sets the text to be rendered.
	virtual void setText(const QString& text) { _text = text; }

	/// \brief Returns the number of vertices stored in the buffer.
	const QString& text() const { return _text; }

	/// \brief Sets the text color.
	virtual void setColor(const ColorA& color) { _color = color; }

	/// \brief Returns the text color.
	const ColorA& color() const { return _color; }

	/// \brief Sets the text background color.
	virtual void setBackgroundColor(const ColorA& color) { _backgroundColor = color; }

	/// \brief Returns the text background color.
	const ColorA& backgroundColor() const { return _backgroundColor; }

	/// Sets the text font.
	virtual void setFont(const QFont& font) { _font = font; }

	/// Returns the text font.
	const QFont& font() const { return _font; }

	/// \brief Renders the text string at the given 2D window (pixel) coordinates.
	virtual void renderWindow(SceneRenderer* renderer, const Point2& pos, int alignment = Qt::AlignLeft | Qt::AlignTop) = 0;

	/// \brief Renders the text string at the given 2D normalized viewport coordinates ([-1,+1] range).
	virtual void renderViewport(SceneRenderer* renderer, const Point2& pos, int alignment = Qt::AlignLeft | Qt::AlignTop) = 0;

	/// \brief Renders the primitive using the given renderer.
	virtual void render(SceneRenderer* renderer) override {}

private:

	/// The text to be rendered.
	QString _text;

	/// The text color.
	ColorA _color;

	/// The text background color.
	ColorA _backgroundColor;

	/// The text font.
	QFont _font;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_TEXT_PRIMITIVE_H
