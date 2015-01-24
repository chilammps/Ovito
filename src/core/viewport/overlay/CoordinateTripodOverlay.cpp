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
#include <core/rendering/RenderSettings.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/FontParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include "CoordinateTripodOverlay.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CoordinateTripodOverlay, ViewportOverlay);
IMPLEMENT_OVITO_OBJECT(Core, CoordinateTripodOverlayEditor, PropertiesEditor);
SET_OVITO_OBJECT_EDITOR(CoordinateTripodOverlay, CoordinateTripodOverlayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _alignment, "Alignment", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _tripodSize, "Size", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _lineWidth, "LineWidth", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _font, "Font", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _fontSize, "FontSize", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _offsetX, "OffsetX", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _offsetY, "OffsetY", PROPERTY_FIELD_MEMORIZE);
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
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _axis1Color, "Axis1Color", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _axis2Color, "Axis2Color", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _axis3Color, "Axis3Color", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(CoordinateTripodOverlay, _axis4Color, "Axis4Color", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _alignment, "Position");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _tripodSize, "Size factor");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _lineWidth, "Line width");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _font, "Font");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _fontSize, "Label size");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _offsetX, "Offset X");
SET_PROPERTY_FIELD_LABEL(CoordinateTripodOverlay, _offsetY, "Offset Y");
SET_PROPERTY_FIELD_UNITS(CoordinateTripodOverlay, _offsetX, PercentParameterUnit);
SET_PROPERTY_FIELD_UNITS(CoordinateTripodOverlay, _offsetY, PercentParameterUnit);

/******************************************************************************
* Constructor.
******************************************************************************/
CoordinateTripodOverlay::CoordinateTripodOverlay(DataSet* dataset) : ViewportOverlay(dataset),
		_alignment(Qt::AlignLeft | Qt::AlignBottom),
		_tripodSize(0.075), _lineWidth(0.06), _offsetX(0), _offsetY(0),
		_fontSize(0.4),
		_axis1Enabled(true), _axis2Enabled(true), _axis3Enabled(true), _axis4Enabled(false),
		_axis1Label("x"), _axis2Label("y"), _axis3Label("z"), _axis4Label("w"),
		_axis1Dir(1,0,0), _axis2Dir(0,1,0), _axis3Dir(0,0,1), _axis4Dir(sqrt(0.5),sqrt(0.5),0),
		_axis1Color(1,0,0), _axis2Color(0,0.8,0), _axis3Color(0.2,0.2,1), _axis4Color(1,0,1)
{
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_alignment);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_tripodSize);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_lineWidth);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_offsetX);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_offsetY);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_font);
	INIT_PROPERTY_FIELD(CoordinateTripodOverlay::_fontSize);
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
	FloatType tripodSize = _tripodSize.value() * renderSettings->outputImageHeight();
	if(tripodSize <= 0) return;

	FloatType lineWidth = _lineWidth.value() * tripodSize;
	if(lineWidth <= 0) return;

	FloatType arrowSize = 0.17f;

	QPointF origin(_offsetX.value() * renderSettings->outputImageWidth(), -_offsetY.value() * renderSettings->outputImageHeight());
	FloatType margin = tripodSize + lineWidth;

	if(_alignment.value() & Qt::AlignLeft) origin.rx() += margin;
	else if(_alignment.value() & Qt::AlignRight) origin.rx() += renderSettings->outputImageWidth() - margin;
	else if(_alignment.value() & Qt::AlignHCenter) origin.rx() += 0.5 * renderSettings->outputImageWidth();

	if(_alignment.value() & Qt::AlignTop) origin.ry() += margin;
	else if(_alignment.value() & Qt::AlignBottom) origin.ry() += renderSettings->outputImageHeight() - margin;
	else if(_alignment.value() & Qt::AlignVCenter) origin.ry() += 0.5 * renderSettings->outputImageHeight();

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

	QString labels[4] = {
			_axis1Label.value(),
			_axis2Label.value(),
			_axis3Label.value(),
			_axis4Label.value()
	};
	QFont font = _font.value();
	qreal fontSize = tripodSize * std::max(0.0, (double)_fontSize.value());
	if(fontSize != 0) {
		font.setPointSizeF(fontSize);
		painter.setFont(font);
	}

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
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

		if(fontSize != 0) {
			QRectF textRect = painter.boundingRect(QRectF(0,0,0,0), Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip, labels[axis]);
			textRect.translate(origin + QPointF(dir.x(), -dir.y()));
			if(std::abs(dir.x()) > FLOATTYPE_EPSILON || std::abs(dir.y()) > FLOATTYPE_EPSILON) {
				FloatType offset1 = dir.x() != 0 ? textRect.width() / std::abs(dir.x()) : FLOATTYPE_MAX;
				FloatType offset2 = dir.y() != 0 ? textRect.height() / std::abs(dir.y()) : FLOATTYPE_MAX;
				textRect.translate(0.5 * std::min(offset1, offset2) * QPointF(dir.x(), -dir.y()));
				Vector3 ndir(dir.x(), dir.y(), 0);
				ndir.resize(lineWidth);
				textRect.translate(ndir.x(), -ndir.y());
			}
			painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip, labels[axis]);
		}
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

	VariantComboBoxParameterUI* alignmentPUI = new VariantComboBoxParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_alignment));
	layout->addWidget(new QLabel(tr("Position:")), 0, 0);
	layout->addWidget(alignmentPUI->comboBox(), 0, 1);
	alignmentPUI->comboBox()->addItem(tr("Top left"), QVariant::fromValue((int)(Qt::AlignTop | Qt::AlignLeft)));
	alignmentPUI->comboBox()->addItem(tr("Top right"), QVariant::fromValue((int)(Qt::AlignTop | Qt::AlignRight)));
	alignmentPUI->comboBox()->addItem(tr("Bottom left"), QVariant::fromValue((int)(Qt::AlignBottom | Qt::AlignLeft)));
	alignmentPUI->comboBox()->addItem(tr("Bottom right"), QVariant::fromValue((int)(Qt::AlignBottom | Qt::AlignRight)));

	FloatParameterUI* offsetXPUI = new FloatParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_offsetX));
	layout->addWidget(offsetXPUI->label(), 1, 0);
	layout->addLayout(offsetXPUI->createFieldLayout(), 1, 1);

	FloatParameterUI* offsetYPUI = new FloatParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_offsetY));
	layout->addWidget(offsetYPUI->label(), 2, 0);
	layout->addLayout(offsetYPUI->createFieldLayout(), 2, 1);

	FloatParameterUI* sizePUI = new FloatParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_tripodSize));
	layout->addWidget(sizePUI->label(), 3, 0);
	layout->addLayout(sizePUI->createFieldLayout(), 3, 1);
	sizePUI->setMinValue(0);

	FloatParameterUI* lineWidthPUI = new FloatParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_lineWidth));
	layout->addWidget(lineWidthPUI->label(), 4, 0);
	layout->addLayout(lineWidthPUI->createFieldLayout(), 4, 1);
	lineWidthPUI->setMinValue(0);

	FloatParameterUI* fontSizePUI = new FloatParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_fontSize));
	layout->addWidget(fontSizePUI->label(), 5, 0);
	layout->addLayout(fontSizePUI->createFieldLayout(), 5, 1);
	fontSizePUI->setMinValue(0);

	FontParameterUI* labelFontPUI = new FontParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_font));
	layout->addWidget(labelFontPUI->label(), 6, 0);
	layout->addWidget(labelFontPUI->fontPicker(), 6, 1);

	// Create a second rollout.
	rollout = createRollout(tr("Coordinate axes"), rolloutParams);

    // Create the rollout contents.
	layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	int row = 0;
	QGridLayout* sublayout;
	StringParameterUI* axisLabelPUI;
	ColorParameterUI* axisColorPUI;
	BooleanGroupBoxParameterUI* axisPUI;

	// Axis 1.
	axisPUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Enabled));
	axisPUI->groupBox()->setTitle("Axis 1");
	layout->addWidget(axisPUI->groupBox(), row++, 0, 1, 2);
	sublayout = new QGridLayout(axisPUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(2);

	// Axis label.
	axisLabelPUI = new StringParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Label));
	sublayout->addWidget(new QLabel(tr("Label:")), 0, 0);
	sublayout->addWidget(axisLabelPUI->textBox(), 0, 1, 1, 2);

	// Axis color.
	axisColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Color));
	sublayout->addWidget(new QLabel(tr("Color:")), 1, 0);
	sublayout->addWidget(axisColorPUI->colorPicker(), 1, 1, 1, 2);

	// Axis direction.
	sublayout->addWidget(new QLabel(tr("Direction:")), 2, 0, 1, 3);
	for(int dim = 0; dim < 3; dim++) {
		Vector3ParameterUI* axisDirPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis1Dir), dim);
		sublayout->addLayout(axisDirPUI->createFieldLayout(), 3, dim, 1, 1);
	}

	// Axis 2
	axisPUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Enabled));
	axisPUI->groupBox()->setTitle("Axis 2");
	layout->addWidget(axisPUI->groupBox(), row++, 0, 1, 2);
	sublayout = new QGridLayout(axisPUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(2);

	// Axis label.
	axisLabelPUI = new StringParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Label));
	sublayout->addWidget(new QLabel(tr("Label:")), 0, 0);
	sublayout->addWidget(axisLabelPUI->textBox(), 0, 1, 1, 2);

	// Axis color.
	axisColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Color));
	sublayout->addWidget(new QLabel(tr("Color:")), 1, 0);
	sublayout->addWidget(axisColorPUI->colorPicker(), 1, 1, 1, 2);

	// Axis direction.
	sublayout->addWidget(new QLabel(tr("Direction:")), 2, 0, 1, 3);
	for(int dim = 0; dim < 3; dim++) {
		Vector3ParameterUI* axisDirPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis2Dir), dim);
		sublayout->addLayout(axisDirPUI->createFieldLayout(), 3, dim, 1, 1);
	}

	// Axis 3.
	axisPUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Enabled));
	axisPUI->groupBox()->setTitle("Axis 3");
	layout->addWidget(axisPUI->groupBox(), row++, 0, 1, 2);
	sublayout = new QGridLayout(axisPUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(2);

	// Axis label.
	axisLabelPUI = new StringParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Label));
	sublayout->addWidget(new QLabel(tr("Label:")), 0, 0);
	sublayout->addWidget(axisLabelPUI->textBox(), 0, 1, 1, 2);

	// Axis color.
	axisColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Color));
	sublayout->addWidget(new QLabel(tr("Color:")), 1, 0);
	sublayout->addWidget(axisColorPUI->colorPicker(), 1, 1, 1, 2);

	// Axis direction.
	sublayout->addWidget(new QLabel(tr("Direction:")), 2, 0, 1, 3);
	for(int dim = 0; dim < 3; dim++) {
		Vector3ParameterUI* axisDirPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis3Dir), dim);
		sublayout->addLayout(axisDirPUI->createFieldLayout(), 3, dim, 1, 1);
	}

	// Axis 4.
	axisPUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Enabled));
	axisPUI->groupBox()->setTitle("Axis 4");
	layout->addWidget(axisPUI->groupBox(), row++, 0, 1, 2);
	sublayout = new QGridLayout(axisPUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(2);

	// Axis label.
	axisLabelPUI = new StringParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Label));
	sublayout->addWidget(new QLabel(tr("Label:")), 0, 0);
	sublayout->addWidget(axisLabelPUI->textBox(), 0, 1, 1, 2);

	// Axis color.
	axisColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Color));
	sublayout->addWidget(new QLabel(tr("Color:")), 1, 0);
	sublayout->addWidget(axisColorPUI->colorPicker(), 1, 1, 1, 2);

	// Axis direction.
	sublayout->addWidget(new QLabel(tr("Direction:")), 2, 0, 1, 3);
	for(int dim = 0; dim < 3; dim++) {
		Vector3ParameterUI* axisDirPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(CoordinateTripodOverlay::_axis4Dir), dim);
		sublayout->addLayout(axisDirPUI->createFieldLayout(), 3, dim, 1, 1);
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
