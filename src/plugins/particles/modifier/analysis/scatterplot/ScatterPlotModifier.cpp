///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//  Copyright (2014) Lars Pastewka
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

#include <plugins/particles/Particles.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/AnimationSettings.h>
#include <plugins/particles/util/ParticlePropertyParameterUI.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>
#include "ScatterPlotModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ScatterPlotModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(ScatterPlotModifier, ScatterPlotModifierEditor);
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _selectXAxisInRange, "SelectXAxisInRange");
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _selectionXAxisRangeStart, "SelectionXAxisRangeStart", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _selectionXAxisRangeEnd, "SelectionXAxisRangeEnd", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _selectYAxisInRange, "SelectYAxisInRange");
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _selectionYAxisRangeStart, "SelectionYAxisRangeStart", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _selectionYAxisRangeEnd, "SelectionYAxisRangeEnd", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _fixXAxisRange, "FixXAxisRange");
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _xAxisRangeStart, "XAxisRangeStart", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _xAxisRangeEnd, "XAxisRangeEnd", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _fixYAxisRange, "FixYAxisRange");
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _yAxisRangeStart, "YAxisRangeStart", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _yAxisRangeEnd, "YAxisRangeEnd", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _xAxisProperty, "XAxisProperty");
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _yAxisProperty, "YAxisProperty");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectXAxisInRange, "Select particles in x-range");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectionXAxisRangeStart, "Selection x-range start");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectionXAxisRangeEnd, "Selection x-range end");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectYAxisInRange, "Select particles in y-range");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectionYAxisRangeStart, "Selection y-range start");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectionYAxisRangeEnd, "Selection y-range end");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _fixXAxisRange, "Fix x-axis range");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _xAxisRangeStart, "X-axis range start");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _xAxisRangeEnd, "X-axis range end");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _fixYAxisRange, "Fix y-axis range");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _yAxisRangeStart, "Y-axis range start");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _yAxisRangeEnd, "Y-axis range end");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _xAxisProperty, "X-axis property");
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _yAxisProperty, "Y-axis property");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ScatterPlotModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ScatterPlotModifier::ScatterPlotModifier(DataSet* dataset) : ParticleModifier(dataset),
	_selectXAxisInRange(false),	_selectionXAxisRangeStart(0), _selectionXAxisRangeEnd(1),
	_selectYAxisInRange(false),	_selectionYAxisRangeStart(0), _selectionYAxisRangeEnd(1),
	_fixXAxisRange(false), _xAxisRangeStart(0),	_xAxisRangeEnd(0), _fixYAxisRange(false),
	_yAxisRangeStart(0), _yAxisRangeEnd(0)
{
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectXAxisInRange);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectionXAxisRangeStart);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectionXAxisRangeEnd);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectYAxisInRange);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectionYAxisRangeStart);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectionYAxisRangeEnd);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_fixXAxisRange);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_xAxisRangeStart);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_xAxisRangeEnd);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_fixYAxisRange);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_yAxisRangeStart);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_yAxisRangeEnd);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_xAxisProperty);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_yAxisProperty);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void ScatterPlotModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);
	ParticlePropertyReference bestProperty;
	if(xAxisProperty().isNull() || yAxisProperty().isNull()) {
		// Select the first available particle property from the input state.
		PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
		for(DataObject* o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
			if(property && (property->dataType() == qMetaTypeId<int>() || property->dataType() == qMetaTypeId<FloatType>())) {
				bestProperty = ParticlePropertyReference(property, (property->componentCount() > 1) ? 0 : -1);
			}
		}
	}
	if(xAxisProperty().isNull() && !bestProperty.isNull()) {
		setXAxisProperty(bestProperty);
	}
	if(yAxisProperty().isNull() && !bestProperty.isNull()) {
		setYAxisProperty(bestProperty);
	}
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus ScatterPlotModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the source property.
	if(xAxisProperty().isNull())
		throw Exception(tr("Select a particle property first."));
	ParticlePropertyObject* xProperty = xAxisProperty().findInState(input());
	ParticlePropertyObject* yProperty = yAxisProperty().findInState(input());
	if(!xProperty)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(xAxisProperty().name()));
	if(!yProperty)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(yAxisProperty().name()));
	if(xAxisProperty().vectorComponent() >= (int)xProperty->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(xAxisProperty().name()).arg(xProperty->componentCount()));
	if(yAxisProperty().vectorComponent() >= (int)yProperty->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(yAxisProperty().name()).arg(yProperty->componentCount()));

	size_t xVecComponent = std::max(0, xAxisProperty().vectorComponent());
	size_t xVecComponentCount = xProperty->componentCount();
	size_t yVecComponent = std::max(0, yAxisProperty().vectorComponent());
	size_t yVecComponentCount = yProperty->componentCount();

	ParticleTypeProperty *typeProperty = static_object_cast<ParticleTypeProperty>(inputStandardProperty(ParticleProperty::ParticleTypeProperty));
	if (!typeProperty)
		throw Exception(tr("The standard ParticleTypeProperty does not exist."));
	_colorMap = typeProperty->colorMap();

	int numIds = 0;
	for(const ParticleType *type: typeProperty->particleTypes()) {
		numIds = std::max(numIds, type->id());
	}
	numIds++;

	ParticlePropertyObject* selProperty = nullptr;
	FloatType selectionXAxisRangeStart = _selectionXAxisRangeStart;
	FloatType selectionXAxisRangeEnd = _selectionXAxisRangeEnd;
	FloatType selectionYAxisRangeStart = _selectionYAxisRangeStart;
	FloatType selectionYAxisRangeEnd = _selectionYAxisRangeEnd;
	size_t numSelected = 0;
	if(_selectXAxisInRange || _selectYAxisInRange) {
		selProperty = outputStandardProperty(ParticleProperty::SelectionProperty, true);
		int* s_begin = selProperty->dataInt();
		int* s_end = s_begin + selProperty->size();
		for(auto s = s_begin; s != s_end; ++s) {
			*s = 1;
			numSelected++;
		}
	}
	if(_selectXAxisInRange) {
		if(selectionXAxisRangeStart > selectionXAxisRangeEnd)
			std::swap(selectionXAxisRangeStart, selectionXAxisRangeEnd);
	}
	if(_selectYAxisInRange) {
		if(selectionYAxisRangeStart > selectionYAxisRangeEnd)
			std::swap(selectionYAxisRangeStart, selectionYAxisRangeEnd);
	}

	double xIntervalStart = _xAxisRangeStart;
	double xIntervalEnd = _xAxisRangeEnd;
	double yIntervalStart = _yAxisRangeStart;
	double yIntervalEnd = _yAxisRangeEnd;

	_xData.clear();
	_yData.clear();
	_xData.resize(numIds);
	_yData.resize(numIds);

	if(xProperty->size() > 0) {
		if(xProperty->dataType() == qMetaTypeId<FloatType>()) {
			const FloatType* vx_begin = xProperty->constDataFloat() + xVecComponent;
			const FloatType* vx_end = vx_begin + (xProperty->size() * xVecComponentCount);
			if (!_fixXAxisRange) {
				xIntervalStart = xIntervalEnd = *vx_begin;
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount) {
					if(*vx < xIntervalStart) xIntervalStart = *vx;
					if(*vx > xIntervalEnd) xIntervalEnd = *vx;
				}
			}
			if(xIntervalEnd != xIntervalStart) {
				const int *particleTypeId = typeProperty->constDataInt();
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount, particleTypeId++) {
					_xData[*particleTypeId].append(*vx);
				}
			}

			if(selProperty && _selectXAxisInRange) {
				OVITO_ASSERT(selProperty->size() == xProperty->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount, ++s) {
					if(*vx < selectionXAxisRangeStart || *vx > selectionXAxisRangeEnd) {
						*s = 0;
						numSelected--;
					}
				}
			}
		}
		else if(xProperty->dataType() == qMetaTypeId<int>()) {
			const int* vx_begin = xProperty->constDataInt() + xVecComponent;
			const int* vx_end = vx_begin + (xProperty->size() * xVecComponentCount);
			if (!_fixXAxisRange) {
				xIntervalStart = xIntervalEnd = *vx_begin;
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount) {
					if(*vx < xIntervalStart) xIntervalStart = *vx;
					if(*vx > xIntervalEnd) xIntervalEnd = *vx;
				}
			}
			if(xIntervalEnd != xIntervalStart) {
				const int *particleTypeId = typeProperty->constDataInt();
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount, particleTypeId++) {
					_xData[*particleTypeId].append(*vx);
				}
			}

			if(selProperty && _selectXAxisInRange) {
				OVITO_ASSERT(selProperty->size() == xProperty->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount, ++s) {
					if(*vx < selectionXAxisRangeStart || *vx > selectionXAxisRangeEnd) {
						*s = 0;
						numSelected--;
					}
				}
			}
		}
	}
	if (yProperty->size() > 0) {
		if(yProperty->dataType() == qMetaTypeId<FloatType>()) {
			const FloatType* vy_begin = yProperty->constDataFloat() + yVecComponent;
			const FloatType* vy_end = vy_begin + (yProperty->size() * yVecComponentCount);
			if (!_fixYAxisRange) {
				yIntervalStart = yIntervalEnd = *vy_begin;
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount) {
					if(*vy < yIntervalStart) yIntervalStart = *vy;
					if(*vy > yIntervalEnd) yIntervalEnd = *vy;
				}
			}
			if(yIntervalEnd != yIntervalStart) {
				const int *particleTypeId = typeProperty->constDataInt();
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount, particleTypeId++) {
					_yData[*particleTypeId].append(*vy);
				}
			}

			if(selProperty && _selectYAxisInRange) {
				OVITO_ASSERT(selProperty->size() == yProperty->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount, ++s) {
					if(*vy < selectionYAxisRangeStart || *vy > selectionYAxisRangeEnd) {
						if(*s) {
							*s = 0;
							numSelected--;
						}
					}
				}
			}
		}
		else if(yProperty->dataType() == qMetaTypeId<int>()) {
			const int* vy_begin = yProperty->constDataInt() + yVecComponent;
			const int* vy_end = vy_begin + (yProperty->size() * yVecComponentCount);
			if (!_fixYAxisRange) {
				yIntervalStart = yIntervalEnd = *vy_begin;
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount) {
					if(*vy < yIntervalStart) yIntervalStart = *vy;
					if(*vy > yIntervalEnd) yIntervalEnd = *vy;
				}
			}
			if(yIntervalEnd != yIntervalStart) {
				const int *particleTypeId = typeProperty->constDataInt();
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount, particleTypeId++) {
					_yData[*particleTypeId].append(*vy);
				}
			}

			if(selProperty && _selectYAxisInRange) {
				OVITO_ASSERT(selProperty->size() == yProperty->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount, ++s) {
					if(*vy < selectionYAxisRangeStart || *vy > selectionYAxisRangeEnd) {
						if(*s) {
							*s = 0;
							numSelected--;
						}
					}
				}
			}
		}
	}
	else {
		xIntervalStart = xIntervalEnd = 0;
		yIntervalStart = yIntervalEnd = 0;
	}

	QString statusMessage;
	if(selProperty) {
		selProperty->changed();
		statusMessage += tr("%1 particles selected (%2%)").arg(numSelected).arg((FloatType)numSelected * 100 / std::max(1,(int)selProperty->size()), 0, 'f', 1);
	}

	_xAxisRangeStart = xIntervalStart;
	_xAxisRangeEnd = xIntervalEnd;
	_yAxisRangeStart = yIntervalStart;
	_yAxisRangeEnd = yIntervalEnd;

	notifyDependents(ReferenceEvent::ObjectStatusChanged);

	return PipelineStatus(PipelineStatus::Success, statusMessage);
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ScatterPlotModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Scatter plot"), rolloutParams, "particles.modifiers.scatter_plot.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	ParticlePropertyParameterUI* xPropertyUI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_xAxisProperty));
	layout->addWidget(new QLabel(tr("X-axis property:"), rollout));
	layout->addWidget(xPropertyUI->comboBox());
	ParticlePropertyParameterUI* yPropertyUI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_yAxisProperty));
	layout->addWidget(new QLabel(tr("Y-axis property:"), rollout));
	layout->addWidget(yPropertyUI->comboBox());

	_scatterPlot = new QCustomPlot();
	_scatterPlot->setMinimumHeight(240);
	_scatterPlot->setInteraction(QCP::iRangeDrag, true);
	_scatterPlot->axisRect()->setRangeDrag(Qt::Orientations(Qt::Horizontal | Qt::Vertical));
	_scatterPlot->setInteraction(QCP::iRangeZoom, true);
	_scatterPlot->axisRect()->setRangeZoom(Qt::Orientations(Qt::Horizontal | Qt::Vertical));

	QPen markerPen;
	markerPen.setColor(QColor(255, 40, 30));
	markerPen.setStyle(Qt::DotLine);
	markerPen.setWidth(2);
	_selectionXAxisRangeStartMarker = new QCPItemStraightLine(_scatterPlot);
	_selectionXAxisRangeEndMarker = new QCPItemStraightLine(_scatterPlot);
	_selectionXAxisRangeStartMarker->setVisible(false);
	_selectionXAxisRangeEndMarker->setVisible(false);
	_selectionXAxisRangeStartMarker->setPen(markerPen);
	_selectionXAxisRangeEndMarker->setPen(markerPen);
	_scatterPlot->addItem(_selectionXAxisRangeStartMarker);
	_scatterPlot->addItem(_selectionXAxisRangeEndMarker);
	_selectionYAxisRangeStartMarker = new QCPItemStraightLine(_scatterPlot);
	_selectionYAxisRangeEndMarker = new QCPItemStraightLine(_scatterPlot);
	_selectionYAxisRangeStartMarker->setVisible(false);
	_selectionYAxisRangeEndMarker->setVisible(false);
	_selectionYAxisRangeStartMarker->setPen(markerPen);
	_selectionYAxisRangeEndMarker->setPen(markerPen);
	_scatterPlot->addItem(_selectionYAxisRangeStartMarker);
	_scatterPlot->addItem(_selectionYAxisRangeEndMarker);
	connect(_scatterPlot->xAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(updateXAxisRange(const QCPRange&)));
	connect(_scatterPlot->yAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(updateYAxisRange(const QCPRange&)));

	layout->addWidget(new QLabel(tr("Scatter plot:")));
	layout->addWidget(_scatterPlot);
	connect(this, &ScatterPlotModifierEditor::contentsReplaced, this, &ScatterPlotModifierEditor::plotScatterPlot);

	QPushButton* saveDataButton = new QPushButton(tr("Save scatter plot data"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, &QPushButton::clicked, this, &ScatterPlotModifierEditor::onSaveData);

	// Selection.
	QGroupBox* selectionBox = new QGroupBox(tr("Selection"), rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(selectionBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(selectionBox);

	BooleanParameterUI* selectInRangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectXAxisInRange));
	sublayout->addWidget(selectInRangeUI->checkBox());

	QHBoxLayout* hlayout = new QHBoxLayout();
	sublayout->addLayout(hlayout);
	FloatParameterUI* selRangeStartPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectionXAxisRangeStart));
	FloatParameterUI* selRangeEndPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectionXAxisRangeEnd));
	hlayout->addWidget(new QLabel(tr("From:")));
	hlayout->addLayout(selRangeStartPUI->createFieldLayout());
	hlayout->addSpacing(12);
	hlayout->addWidget(new QLabel(tr("To:")));
	hlayout->addLayout(selRangeEndPUI->createFieldLayout());
	selRangeStartPUI->setEnabled(false);
	selRangeEndPUI->setEnabled(false);
	connect(selectInRangeUI->checkBox(), &QCheckBox::toggled, selRangeStartPUI, &FloatParameterUI::setEnabled);
	connect(selectInRangeUI->checkBox(), &QCheckBox::toggled, selRangeEndPUI, &FloatParameterUI::setEnabled);

	selectInRangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectYAxisInRange));
	sublayout->addWidget(selectInRangeUI->checkBox());

	hlayout = new QHBoxLayout();
	sublayout->addLayout(hlayout);
	selRangeStartPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectionYAxisRangeStart));
	selRangeEndPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectionYAxisRangeEnd));
	hlayout->addWidget(new QLabel(tr("From:")));
	hlayout->addLayout(selRangeStartPUI->createFieldLayout());
	hlayout->addSpacing(12);
	hlayout->addWidget(new QLabel(tr("To:")));
	hlayout->addLayout(selRangeEndPUI->createFieldLayout());
	selRangeStartPUI->setEnabled(false);
	selRangeEndPUI->setEnabled(false);
	connect(selectInRangeUI->checkBox(), &QCheckBox::toggled, selRangeStartPUI, &FloatParameterUI::setEnabled);
	connect(selectInRangeUI->checkBox(), &QCheckBox::toggled, selRangeEndPUI, &FloatParameterUI::setEnabled);

	// Axes.
	QGroupBox* axesBox = new QGroupBox(tr("Plot axes"), rollout);
	QVBoxLayout* axesSublayout = new QVBoxLayout(axesBox);
	axesSublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(axesBox);
	// x-axis.
	{
		BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_fixXAxisRange));
		axesSublayout->addWidget(rangeUI->checkBox());

		QHBoxLayout* hlayout = new QHBoxLayout();
		axesSublayout->addLayout(hlayout);
		FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_xAxisRangeStart));
		FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_xAxisRangeEnd));
		hlayout->addWidget(new QLabel(tr("From:")));
		hlayout->addLayout(startPUI->createFieldLayout());
		hlayout->addSpacing(12);
		hlayout->addWidget(new QLabel(tr("To:")));
		hlayout->addLayout(endPUI->createFieldLayout());
		startPUI->setEnabled(false);
		endPUI->setEnabled(false);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);
	}
	// y-axis.
	{
		BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_fixYAxisRange));
		axesSublayout->addWidget(rangeUI->checkBox());

		QHBoxLayout* hlayout = new QHBoxLayout();
		axesSublayout->addLayout(hlayout);
		FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_yAxisRangeStart));
		FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_yAxisRangeEnd));
		hlayout->addWidget(new QLabel(tr("From:")));
		hlayout->addLayout(startPUI->createFieldLayout());
		hlayout->addSpacing(12);
		hlayout->addWidget(new QLabel(tr("To:")));
		hlayout->addLayout(endPUI->createFieldLayout());
		startPUI->setEnabled(false);
		endPUI->setEnabled(false);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
		connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);
	}

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool ScatterPlotModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->sender() == editObject() && event->type() == ReferenceEvent::ObjectStatusChanged) {
		plotScatterPlot();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Replots the scatter plot computed by the modifier.
******************************************************************************/
void ScatterPlotModifierEditor::plotScatterPlot()
{
	ScatterPlotModifier* modifier = static_object_cast<ScatterPlotModifier>(editObject());
	if(!modifier)
		return;

	_scatterPlot->xAxis->setLabel(modifier->xAxisProperty().name());
	_scatterPlot->yAxis->setLabel(modifier->yAxisProperty().name());

	if(modifier->numberOfParticleTypeIds() == 0)
		return;

	// Make sure we have the correct number of graphs. (One graph per particle id.)
	while (_scatterPlot->graphCount() > modifier->numberOfParticleTypeIds()) {
		_scatterPlot->removeGraph(_scatterPlot->graph(0));
	}
	while (_scatterPlot->graphCount() < modifier->numberOfParticleTypeIds()) {
		_scatterPlot->addGraph();
		_scatterPlot->graph()->setLineStyle(QCPGraph::lsNone);
	}

	for (int i = 0; i < modifier->numberOfParticleTypeIds(); i++) {
		if (modifier->hasColor(i)) {
			_scatterPlot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,
																	modifier->color(i), 5.0));
		}
		else {
			_scatterPlot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5.0));
		}
		_scatterPlot->graph(i)->setData(modifier->xData(i), modifier->yData(i));
	}

	// Check if range is already correct, because setRange emits the rangeChanged signa
	// which is to be avoided if the range is not determined automatically.
	_rangeUpdate = false;
	_scatterPlot->xAxis->setRange(modifier->xAxisRangeStart(), modifier->xAxisRangeEnd());
	_scatterPlot->yAxis->setRange(modifier->yAxisRangeStart(), modifier->yAxisRangeEnd());
	_rangeUpdate = true;

	if(modifier->selectXAxisInRange()) {
		_selectionXAxisRangeStartMarker->setVisible(true);
		_selectionXAxisRangeEndMarker->setVisible(true);
		_selectionXAxisRangeStartMarker->point1->setCoords(modifier->selectionXAxisRangeStart(), 0);
		_selectionXAxisRangeStartMarker->point2->setCoords(modifier->selectionXAxisRangeStart(), 1);
		_selectionXAxisRangeEndMarker->point1->setCoords(modifier->selectionXAxisRangeEnd(), 0);
		_selectionXAxisRangeEndMarker->point2->setCoords(modifier->selectionXAxisRangeEnd(), 1);
	}
	else {
		_selectionXAxisRangeStartMarker->setVisible(false);
		_selectionXAxisRangeEndMarker->setVisible(false);
	}

	if(modifier->selectYAxisInRange()) {
		_selectionYAxisRangeStartMarker->setVisible(true);
		_selectionYAxisRangeEndMarker->setVisible(true);
		_selectionYAxisRangeStartMarker->point1->setCoords(0, modifier->selectionYAxisRangeStart());
		_selectionYAxisRangeStartMarker->point2->setCoords(1, modifier->selectionYAxisRangeStart());
		_selectionYAxisRangeEndMarker->point1->setCoords(0, modifier->selectionYAxisRangeEnd());
		_selectionYAxisRangeEndMarker->point2->setCoords(1, modifier->selectionYAxisRangeEnd());
	}
	else {
		_selectionYAxisRangeStartMarker->setVisible(false);
		_selectionYAxisRangeEndMarker->setVisible(false);
	}

	_scatterPlot->replot();
}

/******************************************************************************
* Keep x-axis range updated
******************************************************************************/
void ScatterPlotModifierEditor::updateXAxisRange(const QCPRange &newRange)
{
	if (_rangeUpdate) {
		ScatterPlotModifier* modifier = static_object_cast<ScatterPlotModifier>(editObject());
		if(!modifier)
			return;

		// Fix range if user modifies the range by a mouse action in QCustomPlot
		modifier->setFixXAxisRange(true);
		modifier->setXAxisRange(newRange.lower, newRange.upper);
	}
}

/******************************************************************************
* Keep y-axis range updated
******************************************************************************/
void ScatterPlotModifierEditor::updateYAxisRange(const QCPRange &newRange)
{
	if (_rangeUpdate) {
		ScatterPlotModifier* modifier = static_object_cast<ScatterPlotModifier>(editObject());
		if(!modifier)
			return;

		// Fix range if user modifies the range by a mouse action in QCustomPlot
		modifier->setFixYAxisRange(true);
		modifier->setYAxisRange(newRange.lower, newRange.upper);
	}
}

/******************************************************************************
* This is called when the user has clicked the "Save Data" button.
******************************************************************************/
void ScatterPlotModifierEditor::onSaveData()
{
	ScatterPlotModifier* modifier = static_object_cast<ScatterPlotModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->numberOfParticleTypeIds() == 0)
		return;

	QString fileName = QFileDialog::getSaveFileName(mainWindow(),
	    tr("Save Scatter Plot"), QString(), tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Could not open file for writing: %1").arg(file.errorString()));

		QTextStream stream(&file);

		stream << "# " << modifier->xAxisProperty().name() << " " << modifier->yAxisProperty().name() << endl;
		for(int typeId = 0; typeId < modifier->numberOfParticleTypeIds(); typeId++) {
			stream << "# Data for particle type id " << typeId << " follow." << endl;
			for(int i = 0; i < modifier->xData(typeId).size(); i++) {
				stream << modifier->xData(typeId)[i] << " " << modifier->yData(typeId)[i] << endl;
			}
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
