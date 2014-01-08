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

#include <plugins/particles/Particles.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/AnimationSettings.h>
#include <plugins/particles/util/ParticlePropertyParameterUI.h>
#include <3rdparty/qcustomplot/qcustomplot.h>
#include "ScatterPlotModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ScatterPlotModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Particles, ScatterPlotModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(ScatterPlotModifier, ScatterPlotModifierEditor)
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _selectInRange, "SelectInRange")
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _selectionRangeStart, "SelectionRangeStart", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _selectionRangeEnd, "SelectionRangeEnd", PROPERTY_FIELD_MEMORIZE)
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _fixXAxisRange, "FixXAxisRange")
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _xAxisRangeStart, "XAxisRangeStart", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _xAxisRangeEnd, "XAxisRangeEnd", PROPERTY_FIELD_MEMORIZE)
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _fixYAxisRange, "FixYAxisRange")
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _yAxisRangeStart, "YAxisRangeStart", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(ScatterPlotModifier, _yAxisRangeEnd, "YAxisRangeEnd", PROPERTY_FIELD_MEMORIZE)
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _xAxisProperty, "XAxisProperty")
DEFINE_PROPERTY_FIELD(ScatterPlotModifier, _yAxisProperty, "YAxisProperty")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectInRange, "Select particles in range")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectionRangeStart, "Selection range start")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _selectionRangeEnd, "Selection range end")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _fixXAxisRange, "Fix x-axis range")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _xAxisRangeStart, "X-axis range start")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _xAxisRangeEnd, "X-axis range end")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _fixYAxisRange, "Fix y-axis range")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _yAxisRangeStart, "Y-axis range start")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _yAxisRangeEnd, "Y-axis range end")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _xAxisProperty, "X-axis property")
SET_PROPERTY_FIELD_LABEL(ScatterPlotModifier, _yAxisProperty, "Y-axis property")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ScatterPlotModifier::ScatterPlotModifier(DataSet* dataset) : ParticleModifier(dataset),
	_selectInRange(false),
	_selectionRangeStart(0), _selectionRangeEnd(1), _fixXAxisRange(false), _xAxisRangeStart(0),
	_xAxisRangeEnd(0), _fixYAxisRange(false), _yAxisRangeStart(0), _yAxisRangeEnd(0)
{
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectInRange);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectionRangeStart);
	INIT_PROPERTY_FIELD(ScatterPlotModifier::_selectionRangeEnd);
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
		for(const auto& o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
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
ObjectStatus ScatterPlotModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the source property.
	if(xAxisProperty().isNull())
		throw Exception(tr("Select a particle property first."));
	ParticlePropertyObject* xProperty = lookupInputProperty(input(), xAxisProperty());
	ParticlePropertyObject* yProperty = lookupInputProperty(input(), yAxisProperty());
	if(!xProperty)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(xAxisProperty().name()));
	if(!yProperty)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(yAxisProperty().name()));
	if(xAxisProperty().vectorComponent() >= (int)xProperty->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(xAxisProperty().name()).arg(xProperty->componentCount()));
	if(yAxisProperty().vectorComponent() >= (int)yProperty->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(yAxisProperty().name()).arg(yProperty->componentCount()));

	size_t xVecComponent = xAxisProperty().vectorComponent() >= 0 ? xAxisProperty().vectorComponent() : 0;
	size_t xVecComponentCount = xProperty->componentCount();
	size_t yVecComponent = yAxisProperty().vectorComponent() >= 0 ? yAxisProperty().vectorComponent() : 0;
	size_t yVecComponentCount = yProperty->componentCount();

	/*
	ParticlePropertyObject* selProperty = nullptr;
	FloatType selectionRangeStart = _selectionRangeStart;
	FloatType selectionRangeEnd = _selectionRangeEnd;
	size_t numSelected = 0;
	if(_selectInRange) {
		selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);
		if(selectionRangeStart > selectionRangeEnd)
			std::swap(selectionRangeStart, selectionRangeEnd);
	}
	*/

	double xIntervalStart = _xAxisRangeStart;
	double xIntervalEnd = _xAxisRangeEnd;
	double yIntervalStart = _yAxisRangeStart;
	double yIntervalEnd = _yAxisRangeEnd;

	_xData.clear();
	_yData.clear();

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
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount) {
					_xData.append(*vx);
				}
			}
			/*
			else {
				_scatterPlotData[0] = property->size();
			}
			*/
			/*
			if(selProperty) {
				OVITO_ASSERT(selProperty->size() == property->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto v = v_begin; v != v_end; v += vecComponentCount, ++s) {
					if(*v >= selectionRangeStart && *v <= selectionRangeEnd) {
						*s = 1;
						numSelected++;
					}
					else
						*s = 0;
				}
			}
			*/
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
				for(auto vx = vx_begin; vx != vx_end; vx += xVecComponentCount) {
					_xData.append(*vx);
				}
			}
			/*
			else {
				_scatterPlotData[0] = property->size();
			}
			*/
			/*
			if(selProperty) {
				OVITO_ASSERT(selProperty->size() == property->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto v = v_begin; v != v_end; v += vecComponentCount, ++s) {
					if(*v >= selectionRangeStart && *v <= selectionRangeEnd) {
						*s = 1;
						numSelected++;
					}
					else
						*s = 0;
				}
			}
			*/
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
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount) {
					_yData.append(*vy);
				}
			}
			/*
			else {
				_scatterPlotData[0] = property->size();
			}
			*/
			/*
			if(selProperty) {
				OVITO_ASSERT(selProperty->size() == property->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto v = v_begin; v != v_end; v += vecComponentCount, ++s) {
					if(*v >= selectionRangeStart && *v <= selectionRangeEnd) {
						*s = 1;
						numSelected++;
					}
					else
						*s = 0;
				}
			}
			*/
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
				for(auto vy = vy_begin; vy != vy_end; vy += yVecComponentCount) {
					_yData.append(*vy);
				}
			}
			/*
			else {
				_scatterPlotData[0] = property->size();
			}
			*/
			/*
			if(selProperty) {
				OVITO_ASSERT(selProperty->size() == property->size());
				int* s = selProperty->dataInt();
				int* s_end = s + selProperty->size();
				for(auto v = v_begin; v != v_end; v += vecComponentCount, ++s) {
					if(*v >= selectionRangeStart && *v <= selectionRangeEnd) {
						*s = 1;
						numSelected++;
					}
					else
						*s = 0;
				}
			}
			*/
		}
	}
	else {
		xIntervalStart = xIntervalEnd = 0;
		yIntervalStart = yIntervalEnd = 0;
	}

	QString statusMessage;
	/*
	if(selProperty) {
		selProperty->changed();
		statusMessage += tr("%1 particles selected (%2%)").arg(numSelected).arg((FloatType)numSelected * 100 / std::max(1,(int)selProperty->size()), 0, 'f', 1);
	}
	*/

	_xAxisRangeStart = xIntervalStart;
	_xAxisRangeEnd = xIntervalEnd;
	_yAxisRangeStart = yIntervalStart;
	_yAxisRangeEnd = yIntervalEnd;

	notifyDependents(ReferenceEvent::ObjectStatusChanged);

	return ObjectStatus(ObjectStatus::Success, statusMessage);
}

/******************************************************************************
* Retrieves the selected input particle property from the given input state.
******************************************************************************/
ParticlePropertyObject* ScatterPlotModifier::lookupInputProperty(const PipelineFlowState& inputState, const ParticlePropertyReference &refprop) const
{
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* prop = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(prop) {
			if((refprop.type() == ParticleProperty::UserProperty && prop->name() == refprop.name()) ||
					(refprop.type() != ParticleProperty::UserProperty && prop->type() == refprop.type())) {
				return prop;
			}
		}
	}
	return nullptr;
}

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
	_scatterPlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
	_scatterPlot->setInteraction(QCP::iRangeZoom, true);
	_scatterPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
	_scatterPlot->addGraph();
	_scatterPlot->graph()->setLineStyle(QCPGraph::lsNone);
	_scatterPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

	_selectionRangeStartMarker = new QCPItemStraightLine(_scatterPlot);
	_selectionRangeEndMarker = new QCPItemStraightLine(_scatterPlot);
	_selectionRangeStartMarker->setVisible(false);
	_selectionRangeEndMarker->setVisible(false);
	QPen markerPen;
	markerPen.setColor(QColor(255, 40, 30));
	markerPen.setStyle(Qt::DotLine);
	markerPen.setWidth(2);
	_selectionRangeStartMarker->setPen(markerPen);
	_selectionRangeEndMarker->setPen(markerPen);
	_scatterPlot->addItem(_selectionRangeStartMarker);
	_scatterPlot->addItem(_selectionRangeEndMarker);

	layout->addWidget(new QLabel(tr("Scatter plot:")));
	layout->addWidget(_scatterPlot);
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(plotScatterPlot()));

	QPushButton* saveDataButton = new QPushButton(tr("Save scatter plot data"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, SIGNAL(clicked(bool)), this, SLOT(onSaveData()));

	// Selection.
	QGroupBox* selectionBox = new QGroupBox(tr("Selection"), rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(selectionBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(selectionBox);

	BooleanParameterUI* selectInRangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectInRange));
	sublayout->addWidget(selectInRangeUI->checkBox());

	QHBoxLayout* hlayout = new QHBoxLayout();
	sublayout->addLayout(hlayout);
	FloatParameterUI* selRangeStartPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectionRangeStart));
	FloatParameterUI* selRangeEndPUI = new FloatParameterUI(this, PROPERTY_FIELD(ScatterPlotModifier::_selectionRangeEnd));
	hlayout->addWidget(new QLabel(tr("From:")));
	hlayout->addLayout(selRangeStartPUI->createFieldLayout());
	hlayout->addSpacing(12);
	hlayout->addWidget(new QLabel(tr("To:")));
	hlayout->addLayout(selRangeEndPUI->createFieldLayout());
	selRangeStartPUI->setEnabled(false);
	selRangeEndPUI->setEnabled(false);
	connect(selectInRangeUI->checkBox(), SIGNAL(toggled(bool)), selRangeStartPUI, SLOT(setEnabled(bool)));
	connect(selectInRangeUI->checkBox(), SIGNAL(toggled(bool)), selRangeEndPUI, SLOT(setEnabled(bool)));

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
		connect(rangeUI->checkBox(), SIGNAL(toggled(bool)), startPUI, SLOT(setEnabled(bool)));
		connect(rangeUI->checkBox(), SIGNAL(toggled(bool)), endPUI, SLOT(setEnabled(bool)));
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
		connect(rangeUI->checkBox(), SIGNAL(toggled(bool)), startPUI, SLOT(setEnabled(bool)));
		connect(rangeUI->checkBox(), SIGNAL(toggled(bool)), endPUI, SLOT(setEnabled(bool)));
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

	if(modifier->xData().empty() || modifier->yData().empty())
		return;

	_scatterPlot->graph()->setData(modifier->xData(), modifier->yData());

	_scatterPlot->xAxis->setRange(modifier->xAxisRangeStart(), modifier->xAxisRangeEnd());
	_scatterPlot->yAxis->setRange(modifier->yAxisRangeStart(), modifier->yAxisRangeEnd());

	/*
	if(modifier->selectInRange()) {
		_selectionRangeStartMarker->setVisible(true);
		_selectionRangeEndMarker->setVisible(true);
		_selectionRangeStartMarker->point1->setCoords(modifier->selectionRangeStart(), 0);
		_selectionRangeStartMarker->point2->setCoords(modifier->selectionRangeStart(), 1);
		_selectionRangeEndMarker->point1->setCoords(modifier->selectionRangeEnd(), 0);
		_selectionRangeEndMarker->point2->setCoords(modifier->selectionRangeEnd(), 1);
	}
	else {
		_selectionRangeStartMarker->setVisible(false);
		_selectionRangeEndMarker->setVisible(false);
	}
	*/

	_scatterPlot->replot();
}

/******************************************************************************
* This is called when the user has clicked the "Save Data" button.
******************************************************************************/
void ScatterPlotModifierEditor::onSaveData()
{
	ScatterPlotModifier* modifier = static_object_cast<ScatterPlotModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->xData().empty() || modifier->yData().empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(mainWindow(),
	    tr("Save ScatterPlot"), QString(), tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Could not open file for writing: %1").arg(file.errorString()));

		QTextStream stream(&file);

		stream << "# " << modifier->xAxisProperty().name() << " " << modifier->yAxisProperty().name() << endl;
		for(int i = 0; i < modifier->xData().size(); i++) {
			stream << modifier->xData()[i] << " " << modifier->yData()[i] << endl;
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}


};	// End of namespace
