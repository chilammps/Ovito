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
#include <viz/util/qcustomplot/qcustomplot.h>
#include "HistogramModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, HistogramModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, HistogramModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(HistogramModifier, HistogramModifierEditor)
DEFINE_PROPERTY_FIELD(HistogramModifier, _numberOfBins, "NumberOfBins")
SET_PROPERTY_FIELD_LABEL(HistogramModifier, _numberOfBins, "Number of bins")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
HistogramModifier::HistogramModifier() : _numberOfBins(100)
{
	INIT_PROPERTY_FIELD(HistogramModifier::_numberOfBins);

	// Load the last number of bins from the application settings store.
	QSettings settings;
	settings.beginGroup("viz/histogram");
	setNumberOfBins(settings.value("NumberOfBins", numberOfBins()).value<int>());
	settings.endGroup();
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
	_histogramPlot->setMinimumHeight(180);
	_histogramPlot->xAxis->setLabel("Pair separation distance");
	_histogramPlot->yAxis->setLabel("g(r)");
	_histogramPlot->addGraph();

	layout->addWidget(new QLabel(tr("Histogram:")));
	layout->addWidget(_histogramPlot);
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(plotHistogram()));

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
void ColorCodingModifierEditor::updatePropertyList()
{
	propertyListBox->clear();

	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	if(!mod) {
		propertyListBox->setEnabled(false);
		return;
	}
	propertyListBox->setEnabled(true);

	// Obtain the particle property that serves as the input for the color coding modifier.
	PipelineFlowState inputState = mod->getModifierInput();

	// Populate property list from input object.
	int initialIndex = -1;
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(!property) continue;

		// Properties with a non-numeric data type cannot be used as source for the color coding.
		if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;

		if(property->componentNames().empty()) {
			// Scalar property:
			propertyListBox->addItem(property);
		}
		else {
			// Vector property:
			for(int vectorComponent = 0; vectorComponent < (int)property->componentCount(); vectorComponent++) {
				propertyListBox->addItem(property, vectorComponent);
			}
		}
	}

	// Select the right item in the list box.
	int selIndex = propertyListBox->propertyIndex(mod->sourceProperty());
	if(selIndex < 0 && !mod->sourceProperty().isNull()) {
		// Add a place-holder item if the selected property does not exist anymore.
		propertyListBox->addItem(mod->sourceProperty(), tr("%1 (no longer available)").arg(mod->sourceProperty().name()));
		selIndex = propertyListBox->count() - 1;
	}
	propertyListBox->setCurrentIndex(selIndex);
}


/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool HistogramModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
		propertyListBox->setCurrentProperty(mod->sourceProperty());
	}s
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

#if 0
	if(modifier->rdfX().empty())
		return;

	_histogramPlot->graph()->setData(modifier->rdfX(), modifier->rdfY());
	_histogramPlot->graph()->rescaleAxes();
	_histogramPlot->replot();
#endif
}


};	// End of namespace
