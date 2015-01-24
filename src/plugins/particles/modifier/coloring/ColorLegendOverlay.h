///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_COLOR_LEGEND_OVERLAY_H
#define __OVITO_COLOR_LEGEND_OVERLAY_H

#include <plugins/particles/Particles.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/viewport/overlay/ViewportOverlay.h>
#include "ColorCodingModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

/**
 * \brief A viewport overlay that displays the color legend of a ColorCodingModifier.
 */
class OVITO_PARTICLES_EXPORT ColorLegendOverlay : public ViewportOverlay
{
public:

	/// \brief Constructor.
	Q_INVOKABLE ColorLegendOverlay(DataSet* dataset);

	/// \brief This method asks the overlay to paint its contents over the given viewport.
	virtual void render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings) override;

	/// Returns the ColorCodingModifier for which to display the legend.
	ColorCodingModifier* modifier() const { return _modifier; }

	/// Sets the ColorCodingModifier for which to display the legend.
	void setModifier(ColorCodingModifier* mod) { _modifier = mod; }

	/// Returns the formatting of the value labels in the color legend.
	const QString& valueFormatString() const { return _valueFormatString; }

	/// Sets the formatting of the value labels in the color legend.
	void setValueFormatString(const QString& format) { _valueFormatString = format; }

	/// Returns the title text of the color legend.
	const QString& title() const { return _title; }

	/// Sets the title text of the color legend.
	void setTitle(const QString& text) { _title = text; }

	/// Returns the user-defined text to be used for the first numeric label.
	const QString& label1() const { return _label1; }

	/// Returns the user-defined text to be used for the second numeric label.
	const QString& label2() const { return _label2; }

public:

	Q_PROPERTY(Ovito::Particles::ColorCodingModifier* modifier READ modifier WRITE setModifier);

private:

	/// The corner of the viewport where the color legend is displayed.
	PropertyField<int> _alignment;

	/// The orientation (horizontal/vertical) of the color legend.
	PropertyField<int> _orientation;

	/// Controls the overall size of the color legend.
	PropertyField<FloatType> _legendSize;

	/// Controls the aspect ration of the color bar.
	PropertyField<FloatType> _aspectRatio;

	/// Controls the horizontal offset of legend position.
	PropertyField<FloatType> _offsetX;

	/// Controls the vertical offset of legend position.
	PropertyField<FloatType> _offsetY;

	/// Controls the label font.
	PropertyField<QFont> _font;

	/// Controls the label font size.
	PropertyField<FloatType> _fontSize;

	/// The title label.
	PropertyField<QString> _title;

	/// User-defined text for the first numeric label.
	PropertyField<QString> _label1;

	/// User-defined text for the second numeric label.
	PropertyField<QString> _label2;

	/// The ColorCodingModifier for which to display the legend.
	ReferenceField<ColorCodingModifier> _modifier;

	/// Controls the formatting of the value labels in the color legend.
	PropertyField<QString> _valueFormatString;

	/// Controls the text color.
	PropertyField<Color, QColor> _textColor;

	DECLARE_PROPERTY_FIELD(_alignment);
	DECLARE_PROPERTY_FIELD(_orientation);
	DECLARE_PROPERTY_FIELD(_aspectRatio);
	DECLARE_PROPERTY_FIELD(_font);
	DECLARE_PROPERTY_FIELD(_fontSize);
	DECLARE_PROPERTY_FIELD(_legendSize);
	DECLARE_PROPERTY_FIELD(_offsetX);
	DECLARE_PROPERTY_FIELD(_offsetY);
	DECLARE_PROPERTY_FIELD(_title);
	DECLARE_PROPERTY_FIELD(_label1);
	DECLARE_PROPERTY_FIELD(_label2);
	DECLARE_PROPERTY_FIELD(_valueFormatString);
	DECLARE_PROPERTY_FIELD(_textColor);
	DECLARE_REFERENCE_FIELD(_modifier);

	Q_CLASSINFO("DisplayName", "Color legend");

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the ColorLegendOverlay class.
 */
class ColorLegendOverlayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE ColorLegendOverlayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_COLOR_LEGEND_OVERLAY_H
