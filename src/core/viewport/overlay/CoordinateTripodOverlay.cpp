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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include "CoordinateTripodOverlay.h"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CoordinateTripodOverlay, ViewportOverlay);
IMPLEMENT_OVITO_OBJECT(Core, CoordinateTripodOverlayEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(CoordinateTripodOverlay, CoordinateTripodOverlayEditor);
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _alignment, "Alignment");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis1Enabled, "Axis1Enabled");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis2Enabled, "Axis2Enabled");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis3Enabled, "Axis3Enabled");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis4Enabled, "Axis4Enabled");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis1Label, "Axis1Label");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis2Label, "Axis2Label");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis3Label, "Axis3Label");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis4Label, "Axis4Label");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis1Dir, "Axis1Dir");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis2Dir, "Axis2Dir");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis3Dir, "Axis3Dir");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis4Dir, "Axis4Dir");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis1Color, "Axis1Color");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis2Color, "Axis2Color");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis3Color, "Axis3Color");
DEFINE_PROPERTY_FIELD(CoordinateTripodOverlay, _axis4Color, "Axis4Color");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _alignment, "Position");

/******************************************************************************
* Constructor.
******************************************************************************/
CoordinateTripodOverlay::CoordinateTripodOverlay(DataSet* dataset) : ViewportOverlay(dataset),
		_alignment(Qt::AlignLeft | Qt::AlignBottom),
		_axis1Enabled(true), _axis2Enabled(true), _axis3Enabled(true), _axis4Enabled(false),
		_axis1Label("x"), _axis2Label("y"), _axis3Label("z"), _axis4Label("w"),
		_axis1Dir(1,0,0), _axis2Dir(0,1,0), _axis3Dir(0,0,1), _axis4Dir(sqrt(0.5),sqrt(0.5),0),
		_axis1Color(1,0,0), _axis2Color(0,1,0), _axis3Color(0.2,0.2,1), _axis4Color(0,1,1)
{
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_alignment);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Enabled);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Enabled);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Enabled);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Enabled);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Label);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Label);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Label);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Label);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Dir);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Dir);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Dir);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Dir);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Color);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Color);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Color);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Color);
}

/******************************************************************************
* This method asks the overlay to paint its contents over the given viewport.
******************************************************************************/
void CoordinateTripodOverlay::render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings)
{
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	qreal lineWidth = 4;
	FloatType tripodSize = 40.0f;
	FloatType arrowSize = 0.17f;
	QPointF origin(tripodSize, tripodSize);

	// Project axes to screen.
	Vector3 axisDirs[4] = {
			projParams.viewMatrix * _axis1Dir.value(),
			projParams.viewMatrix * _axis2Dir.value(),
			projParams.viewMatrix * _axis3Dir.value(),
			projParams.viewMatrix * _axis4Dir.value()
	};

	// Get axis colors.
	QColor axisColors[4] = {
			_axis1Color.value(),
			_axis2Color.value(),
			_axis3Color.value(),
			_axis4Color.value()
	};

	// Order axes back to front.
	std::vector<int> orderedAxes;
	if(_axis1Enabled) orderedAxes.push_back(0);
	if(_axis2Enabled) orderedAxes.push_back(1);
	if(_axis3Enabled) orderedAxes.push_back(2);
	if(_axis4Enabled) orderedAxes.push_back(3);
	std::sort(orderedAxes.begin(), orderedAxes.end(), [&axisDirs](int a, int b) {
		return axisDirs[a].z() < axisDirs[b].z();
	});

	for(int axis : orderedAxes) {
		QPen pen(axisColors[axis]);
		pen.setWidthF(lineWidth);
		pen.setJoinStyle(Qt::MiterJoin);
		pen.setCapStyle(Qt::RoundCap);
		painter.setPen(pen);
		Vector3 dir = tripodSize * axisDirs[axis];
		if(dir.squaredLength() > FLOATTYPE_EPSILON) {
			painter.drawLine(origin, origin + QPointF(dir.x(), -dir.y()));
			Vector3 ndir = dir.resized(tripodSize);
			QPointF head[3];
			head[1] = origin + QPointF(dir.x(), -dir.y());
			head[0] = head[1] + QPointF(arrowSize * (ndir.y() - ndir.x()), -arrowSize * (-ndir.x() - ndir.y()));
			head[2] = head[1] + QPointF(arrowSize * (-ndir.y() - ndir.x()), -arrowSize * (ndir.x() - ndir.y()));
			painter.drawPolyline(head, 3);
		}

		//painter.drawText(10, 10, QString("Hello World!"));
	}
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CoordinateTripodOverlayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Coordinate tripod"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	int row = 0;
	QGridLayout* sublayout;

	// Axis 1.
	BooleanGroupBoxParameterUI* axis1PUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Enabled));
	axis1PUI->groupBox()->setTitle("Axis 1");
	layout->addWidget(axis1PUI->groupBox(), row++, 0, 1, 2);
	sublayout = new QGridLayout(axis1PUI->groupBox());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(2);

	// Axis label.
	StringParameterUI* axis1LabelPUI = new StringParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Label));
	sublayout->addWidget(new QLabel(tr("Label:")), 0, 0);
	sublayout->addWidget(axis1LabelPUI->textBox(), 0, 1, 1, 2);

	// Axis color.
	ColorParameterUI* axis1ColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Color));
	sublayout->addWidget(new QLabel(tr("Color:")), 1, 0);
	sublayout->addWidget(axis1ColorPUI->colorPicker(), 1, 1, 1, 2);

	// Axis direction.
	sublayout->addWidget(new QLabel(tr("Direction:")), 2, 0, 1, 3);
	for(int dim = 0; dim < 3; dim++) {
		Vector3ParameterUI* axisDirPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Dir), dim);
		sublayout->addLayout(axisDirPUI->createFieldLayout(), 3, dim, 1, 1);
	}
}

};
