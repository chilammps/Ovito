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

#ifndef __OVITO_COORDINATE_TRIPOD_OVERLAY_H
#define __OVITO_COORDINATE_TRIPOD_OVERLAY_H

#include <core/Core.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "ViewportOverlay.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A viewport overlay that displays the coordinate system orientation.
 */
class OVITO_CORE_EXPORT CoordinateTripodOverlay : public ViewportOverlay
{
public:

	/// \brief Constructor.
	Q_INVOKABLE CoordinateTripodOverlay(DataSet* dataset);

	/// \brief This method asks the overlay to paint its contents over the given viewport.
	virtual void render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings) override;

private:

	/// The corner of the viewport where the tripod is shown in.
	PropertyField<int> _alignment;

	/// Controls the size of the tripod.
	PropertyField<FloatType> _tripodSize;

	/// Controls the line width.
	PropertyField<FloatType> _lineWidth;

	/// Controls the horizontal offset of tripod position.
	PropertyField<FloatType> _offsetX;

	/// Controls the vertical offset of tripod position.
	PropertyField<FloatType> _offsetY;

	/// Controls the label font.
	PropertyField<QFont> _font;

	/// Controls the label font size.
	PropertyField<FloatType> _fontSize;

	/// Controls the display of the first axis.
	PropertyField<bool> _axis1Enabled;

	/// Controls the display of the second axis.
	PropertyField<bool> _axis2Enabled;

	/// Controls the display of the third axis.
	PropertyField<bool> _axis3Enabled;

	/// Controls the display of the fourth axis.
	PropertyField<bool> _axis4Enabled;

	/// The label of the first axis.
	PropertyField<QString> _axis1Label;

	/// The label of the second axis.
	PropertyField<QString> _axis2Label;

	/// The label of the third axis.
	PropertyField<QString> _axis3Label;

	/// The label of the fourth axis.
	PropertyField<QString> _axis4Label;

	/// The direction of the first axis.
	PropertyField<Vector3> _axis1Dir;

	/// The direction of the second axis.
	PropertyField<Vector3> _axis2Dir;

	/// The direction of the third axis.
	PropertyField<Vector3> _axis3Dir;

	/// The direction of the fourth axis.
	PropertyField<Vector3> _axis4Dir;

	/// The display color of the first axis.
	PropertyField<Color, QColor> _axis1Color;

	/// The display color of the second axis.
	PropertyField<Color, QColor> _axis2Color;

	/// The display color of the third axis.
	PropertyField<Color, QColor> _axis3Color;

	/// The display color of the fourth axis.
	PropertyField<Color, QColor> _axis4Color;

	DECLARE_PROPERTY_FIELD(_alignment);
	DECLARE_PROPERTY_FIELD(_font);
	DECLARE_PROPERTY_FIELD(_fontSize);
	DECLARE_PROPERTY_FIELD(_tripodSize);
	DECLARE_PROPERTY_FIELD(_lineWidth);
	DECLARE_PROPERTY_FIELD(_offsetX);
	DECLARE_PROPERTY_FIELD(_offsetY);
	DECLARE_PROPERTY_FIELD(_axis1Enabled);
	DECLARE_PROPERTY_FIELD(_axis2Enabled);
	DECLARE_PROPERTY_FIELD(_axis3Enabled);
	DECLARE_PROPERTY_FIELD(_axis4Enabled);
	DECLARE_PROPERTY_FIELD(_axis1Label);
	DECLARE_PROPERTY_FIELD(_axis2Label);
	DECLARE_PROPERTY_FIELD(_axis3Label);
	DECLARE_PROPERTY_FIELD(_axis4Label);
	DECLARE_PROPERTY_FIELD(_axis1Dir);
	DECLARE_PROPERTY_FIELD(_axis2Dir);
	DECLARE_PROPERTY_FIELD(_axis3Dir);
	DECLARE_PROPERTY_FIELD(_axis4Dir);
	DECLARE_PROPERTY_FIELD(_axis1Color);
	DECLARE_PROPERTY_FIELD(_axis2Color);
	DECLARE_PROPERTY_FIELD(_axis3Color);
	DECLARE_PROPERTY_FIELD(_axis4Color);

	Q_CLASSINFO("DisplayName", "Coordinate tripod");

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A properties editor for the CoordinateTripodOverlay class.
 */
class CoordinateTripodOverlayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE CoordinateTripodOverlayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_COORDINATE_TRIPOD_OVERLAY_H
