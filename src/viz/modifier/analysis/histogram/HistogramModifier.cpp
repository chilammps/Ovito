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

#include <core/Core.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <viz/util/qcustomplot/qcustomplot.h>
#include "HistogramModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, HistogramModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, HistogramModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(HistogramModifier, HistogramModifierEditor)
DEFINE_PROPERTY_FIELD(HistogramModifier, _numberOfBins, "NumberOfBins")
DEFINE_PROPERTY_FIELD(HistogramModifier, _selectInRange, "SelectInRange")
DEFINE_PROPERTY_FIELD(HistogramModifier, _selectionRangeStart, "SelectionRangeStart")
DEFINE_PROPERTY_FIELD(HistogramModifier, _selectionRangeEnd, "SelectionRangeEnd")
SET_PROPERTY_FIELD_LABEL(HistogramModifier, _numberOfBins, "Number of histogram bins")
SET_PROPERTY_FIELD_LABEL(HistogramModifier, _selectInRange, "Select particles in range")
SET_PROPERTY_FIELD_LABEL(HistogramModifier, _selectionRangeStart, "Selection range start")
SET_PROPERTY_FIELD_LABEL(HistogramModifier, _selectionRangeEnd, "Selection range end")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
HistogramModifier::HistogramModifier() : _numberOfBins(200), _intervalStart(0), _intervalEnd(0),
	_selectInRange(false), _selectionRangeStart(0), _selectionRangeEnd(1)
{
	INIT_PROPERTY_FIELD(HistogramModifier::_numberOfBins);
	INIT_PROPERTY_FIELD(HistogramModifier::_selectInRange);
	INIT_PROPERTY_FIELD(HistogramModifier::_selectionRangeStart);
	INIT_PROPERTY_FIELD(HistogramModifier::_selectionRangeEnd);

	// Load the last number of bins from the application settings store.
	QSettings settings;
	settings.beginGroup("viz/histogram");
	setNumberOfBins(settings.value("NumberOfBins", numberOfBins()).value<int>());
	settings.endGroup();
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void HistogramModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);
	if(sourceProperty().isNull()) {
		// Select the first available particle property from the input state.
		PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
		ParticlePropertyReference bestProperty;
		for(const auto& o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
			if(property && (property->dataType() == qMetaTypeId<int>() || property->dataType() == qMetaTypeId<FloatType>())) {
				bestProperty = ParticlePropertyReference(property, (property->componentCount() > 1) ? 0 : -1);
			}
		}
		if(!bestProperty.isNull()) {
			setSourceProperty(bestProperty);
		}
	}
}

/******************************************************************************
* Sets the source particle property for which the histogram should be computed.
******************************************************************************/
void HistogramModifier::setSourceProperty(const ParticlePropertyReference& prop)
{
	if(_sourcePropertyRef == prop) return;

	// Make this change undoable.
	qRegisterMetaType<ParticlePropertyReference>();
	if(UndoManager::instance().isRecording())
		UndoManager::instance().push(new SimplePropertyChangeOperation(this, "sourceProperty"));

	_sourcePropertyRef = prop;
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus HistogramModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the source property.
	if(sourceProperty().isNull())
		throw Exception(tr("Select a particle property first."));
	ParticlePropertyObject* property = lookupInputProperty(input());
	if(!property)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(sourceProperty().name()));
	if(sourceProperty().vectorComponent() >= (int)property->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(sourceProperty().name()).arg(property->componentCount()));

	size_t vecComponent = sourceProperty().vectorComponent() >= 0 ? sourceProperty().vectorComponent() : 0;
	size_t vecComponentCount = property->componentCount();

	_histogramData.resize(std::max(1, numberOfBins()));
	std::fill(_histogramData.begin(), _histogramData.end(), 0);

	ParticlePropertyObject* selProperty = nullptr;
	FloatType selectionRangeStart = _selectionRangeStart;
	FloatType selectionRangeEnd = _selectionRangeEnd;
	size_t numSelected = 0;
	if(_selectInRange) {
		selProperty = outputStandardProperty(ParticleProperty::SelectionProperty);
		if(selectionRangeStart > selectionRangeEnd)
			std::swap(selectionRangeStart, selectionRangeEnd);
	}

	if(property->size() > 0) {
		if(property->dataType() == qMetaTypeId<FloatType>()) {
			const FloatType* v_begin = property->constDataFloat() + vecComponent;
			const FloatType* v_end = v_begin + (property->size() * vecComponentCount);
			_intervalStart = _intervalEnd = *v_begin;
			for(auto v = v_begin; v != v_end; v += vecComponentCount) {
				if(*v < _intervalStart) _intervalStart = *v;
				if(*v > _intervalEnd) _intervalEnd = *v;
			}
			if(_intervalEnd != _intervalStart) {
				FloatType binSize = (_intervalEnd - _intervalStart) / _histogramData.size();
				for(auto v = v_begin; v != v_end; v += vecComponentCount) {
					size_t binIndex = (*v - _intervalStart) / binSize;
					_histogramData[std::min(binIndex, _histogramData.size() - 1)]++;
				}
			}
			else {
				_histogramData[0] = property->size();
			}
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
		}
		else if(property->dataType() == qMetaTypeId<int>()) {
			const int* v_begin = property->constDataInt() + vecComponent;
			const int* v_end = v_begin + (property->size() * vecComponentCount);
			_intervalStart = _intervalEnd = *v_begin;
			for(auto v = v_begin; v != v_end; v += vecComponentCount) {
				if(*v < _intervalStart) _intervalStart = *v;
				if(*v > _intervalEnd) _intervalEnd = *v;
			}
			if(_intervalEnd != _intervalStart) {
				FloatType binSize = (_intervalEnd - _intervalStart) / _histogramData.size();
				for(auto v = v_begin; v != v_end; v += vecComponentCount) {
					size_t binIndex = ((FloatType)*v - _intervalStart) / binSize;
					_histogramData[std::min(binIndex, _histogramData.size() - 1)]++;
				}
			}
			else {
				_histogramData[0] = property->size();
			}
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
		}
	}
	else {
		_intervalStart = _intervalEnd = 0;
	}

	QString statusMessage;
	if(selProperty) {
		selProperty->changed();
		statusMessage += tr("%1 particles selected (%2%)").arg(numSelected).arg((FloatType)numSelected * 100 / std::max(1,(int)selProperty->size()), 0, 'f', 1);
	}

	notifyDependents(ReferenceEvent::StatusChanged);

	return ObjectStatus(ObjectStatus::Success, QString(), statusMessage);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void HistogramModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);

	stream.beginChunk(0x01);
	stream << _sourcePropertyRef;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void HistogramModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream >> _sourcePropertyRef;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> HistogramModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<HistogramModifier> clone = static_object_cast<HistogramModifier>(ParticleModifier::clone(deepCopy, cloneHelper));
	clone->_sourcePropertyRef = this->_sourcePropertyRef;

	return clone;
}

/******************************************************************************
* Retrieves the selected input particle property from the given input state.
******************************************************************************/
ParticlePropertyObject* HistogramModifier::lookupInputProperty(const PipelineFlowState& inputState) const
{
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* prop = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(prop) {
			if((sourceProperty().type() == ParticleProperty::UserProperty && prop->name() == sourceProperty().name()) ||
					(sourceProperty().type() != ParticleProperty::UserProperty && prop->type() == sourceProperty().type())) {
				return prop;
			}
		}
	}
	return nullptr;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void HistogramModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Histogram"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	_propertyListBox = new ParticlePropertyComboBox();
	layout->addWidget(new QLabel(tr("Property:"), rollout));
	layout->addWidget(_propertyListBox);
	connect(_propertyListBox, SIGNAL(activated(int)), this, SLOT(onPropertySelected(int)));

	// Update property list if another modifier has been loaded into the editor.
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(updatePropertyList()));

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// Number of bins parameter.
	IntegerParameterUI* numBinsPUI = new IntegerParameterUI(this, PROPERTY_FIELD(HistogramModifier::_numberOfBins));
	gridlayout->addWidget(numBinsPUI->label(), 0, 0);
	gridlayout->addLayout(numBinsPUI->createFieldLayout(), 0, 1);
	numBinsPUI->setMinValue(1);
	connect(numBinsPUI, SIGNAL(valueEntered()), this, SLOT(memorizeNumberOfBins()));

	layout->addLayout(gridlayout);

	_histogramPlot = new QCustomPlot();
	_histogramPlot->setMinimumHeight(240);
	_histogramPlot->setInteraction(QCP::iRangeDrag, true);
	_histogramPlot->axisRect()->setRangeDrag(Qt::Horizontal);
	_histogramPlot->setInteraction(QCP::iRangeZoom, true);
	_histogramPlot->axisRect()->setRangeZoom(Qt::Horizontal);
	_histogramPlot->yAxis->setLabel("Particle count");
	_histogramPlot->addGraph();
	_histogramPlot->graph()->setBrush(QBrush(QColor(255, 160, 100)));

	_selectionRangeStartMarker = new QCPItemStraightLine(_histogramPlot);
	_selectionRangeEndMarker = new QCPItemStraightLine(_histogramPlot);
	_selectionRangeStartMarker->setVisible(false);
	_selectionRangeEndMarker->setVisible(false);
	QPen markerPen;
	markerPen.setColor(QColor(255, 40, 30));
	markerPen.setStyle(Qt::DotLine);
	markerPen.setWidth(2);
	_selectionRangeStartMarker->setPen(markerPen);
	_selectionRangeEndMarker->setPen(markerPen);
	_histogramPlot->addItem(_selectionRangeStartMarker);
	_histogramPlot->addItem(_selectionRangeEndMarker);

	layout->addWidget(new QLabel(tr("Histogram:")));
	layout->addWidget(_histogramPlot);
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(plotHistogram()));

	QPushButton* saveDataButton = new QPushButton(tr("Save histogram data"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, SIGNAL(clicked(bool)), this, SLOT(onSaveData()));

	QGroupBox* selectionBox = new QGroupBox(tr("Selection"), rollout);
	QVBoxLayout* sublayout = new QVBoxLayout(selectionBox);
	sublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(selectionBox);

	BooleanParameterUI* selectInRangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(HistogramModifier::_selectInRange));
	sublayout->addWidget(selectInRangeUI->checkBox());

	QHBoxLayout* hlayout = new QHBoxLayout();
	sublayout->addLayout(hlayout);
	FloatParameterUI* selRangeStartPUI = new FloatParameterUI(this, PROPERTY_FIELD(HistogramModifier::_selectionRangeStart));
	FloatParameterUI* selRangeEndPUI = new FloatParameterUI(this, PROPERTY_FIELD(HistogramModifier::_selectionRangeEnd));
	hlayout->addWidget(new QLabel(tr("From:")));
	hlayout->addLayout(selRangeStartPUI->createFieldLayout());
	hlayout->addSpacing(12);
	hlayout->addWidget(new QLabel(tr("To:")));
	hlayout->addLayout(selRangeEndPUI->createFieldLayout());
	selRangeStartPUI->setEnabled(false);
	selRangeEndPUI->setEnabled(false);
	connect(selectInRangeUI->checkBox(), SIGNAL(toggled(bool)), selRangeStartPUI, SLOT(setEnabled(bool)));
	connect(selectInRangeUI->checkBox(), SIGNAL(toggled(bool)), selRangeEndPUI, SLOT(setEnabled(bool)));

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* Stores the current number of bins in the application settings
* so it can be used as default value for new modifiers in the future.
******************************************************************************/
void HistogramModifierEditor::memorizeNumberOfBins()
{
	if(!editObject()) return;
	HistogramModifier* modifier = static_object_cast<HistogramModifier>(editObject());

	QSettings settings;
	settings.beginGroup("viz/histogram");
	settings.setValue("NumberOfBins", modifier->numberOfBins());
	settings.endGroup();
}

/******************************************************************************
* Updates the contents of the combo box.
******************************************************************************/
void HistogramModifierEditor::updatePropertyList()
{
	_propertyListBox->clear();

	HistogramModifier* mod = static_object_cast<HistogramModifier>(editObject());
	if(!mod) {
		_propertyListBox->setEnabled(false);
		return;
	}
	_propertyListBox->setEnabled(true);

	// Obtain the particle property that serves as the input for the color coding modifier.
	PipelineFlowState inputState = mod->getModifierInput();

	// Populate property list from input object.
	int initialIndex = -1;
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(!property) continue;

		// Properties with a non-numeric data type cannot be used as source for the histogram.
		if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;

		if(property->componentNames().empty()) {
			// Scalar property:
			_propertyListBox->addItem(property);
		}
		else {
			// Vector property:
			for(int vectorComponent = 0; vectorComponent < (int)property->componentCount(); vectorComponent++) {
				_propertyListBox->addItem(property, vectorComponent);
			}
		}
	}

	// Select the right item in the list box.
	int selIndex = _propertyListBox->propertyIndex(mod->sourceProperty());
	if(selIndex < 0 && !mod->sourceProperty().isNull()) {
		// Add a place-holder item if the selected property does not exist anymore.
		_propertyListBox->addItem(mod->sourceProperty(), tr("%1 (no longer available)").arg(mod->sourceProperty().name()));
		selIndex = _propertyListBox->count() - 1;
	}
	_propertyListBox->setCurrentIndex(selIndex);
}

/******************************************************************************
* Is called when the user selects an input particle property.
******************************************************************************/
void HistogramModifierEditor::onPropertySelected(int index)
{
	OVITO_ASSERT(!UndoManager::instance().isRecording());

	if(index < 0) return;
	HistogramModifier* mod = static_object_cast<HistogramModifier>(editObject());
	OVITO_CHECK_OBJECT_POINTER(mod);

	UndoableTransaction::handleExceptions(tr("Select property"), [this, mod, index]() {
		mod->setSourceProperty(_propertyListBox->property(index));
	});
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool HistogramModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		HistogramModifier* mod = static_object_cast<HistogramModifier>(editObject());
		_propertyListBox->setCurrentProperty(mod->sourceProperty());
	}
	else if(event->sender() == editObject() && event->type() == ReferenceEvent::StatusChanged) {
		plotHistogram();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Replots the histogram computed by the modifier.
******************************************************************************/
void HistogramModifierEditor::plotHistogram()
{
	HistogramModifier* modifier = static_object_cast<HistogramModifier>(editObject());
	if(!modifier)
		return;

	_histogramPlot->xAxis->setLabel(modifier->sourceProperty().name());

	if(modifier->histogramData().empty())
		return;

	QVector<double> xdata(modifier->histogramData().size());
	QVector<double> ydata(modifier->histogramData().size());
	double binSize = (modifier->intervalEnd() - modifier->intervalStart()) / xdata.size();
	for(int i = 0; i < xdata.size(); i++) {
		xdata[i] = binSize * ((double)i + 0.5) + modifier->intervalStart();
		ydata[i] = modifier->histogramData()[i];
	}
	_histogramPlot->graph()->setLineStyle(QCPGraph::lsStepCenter);
	_histogramPlot->graph()->setData(xdata, ydata);

	_histogramPlot->graph()->rescaleAxes();
	_histogramPlot->xAxis->setRangeLower(modifier->intervalStart());
	_histogramPlot->xAxis->setRangeUpper(modifier->intervalEnd());
	_histogramPlot->yAxis->setRangeLower(0);

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

	_histogramPlot->replot();
}

/******************************************************************************
* This is called when the user has clicked the "Save Data" button.
******************************************************************************/
void HistogramModifierEditor::onSaveData()
{
	HistogramModifier* modifier = static_object_cast<HistogramModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->histogramData().empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(&MainWindow::instance(),
	    tr("Save Histogram"), QString(), tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Could not open file for writing: %1").arg(file.errorString()));

		QTextStream stream(&file);

		FloatType binSize = (modifier->intervalEnd() - modifier->intervalStart()) / modifier->histogramData().size();
		stream << "# " << modifier->sourceProperty().name() << " histogram (bin size: " << binSize << ")" << endl;
		for(int i = 0; i < modifier->histogramData().size(); i++) {
			stream << (binSize * (FloatType(i) + 0.5f) + modifier->intervalStart()) << " " <<
					modifier->histogramData()[i] << endl;
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}


};	// End of namespace
