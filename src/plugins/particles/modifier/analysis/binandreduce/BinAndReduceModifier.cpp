///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/AnimationSettings.h>
#include <plugins/particles/util/ParticlePropertyParameterUI.h>
#include "BinAndReduceModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, BinAndReduceModifier, ParticleModifier);
IMPLEMENT_OVITO_OBJECT(Particles, BinAndReduceModifierEditor, ParticleModifierEditor);
SET_OVITO_OBJECT_EDITOR(BinAndReduceModifier, BinAndReduceModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(BinAndReduceModifier, _reductionOperation, "ReductionOperation", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(BinAndReduceModifier, _binAlignment, "BinAlignment", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(BinAndReduceModifier, _numberOfBins, "NumberOfBins", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(BinAndReduceModifier, _fixYAxisRange, "FixYAxisRange");
DEFINE_FLAGS_PROPERTY_FIELD(BinAndReduceModifier, _yAxisRangeStart, "YAxisRangeStart", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(BinAndReduceModifier, _yAxisRangeEnd, "YAxisRangeEnd", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(BinAndReduceModifier, _sourceProperty, "SourceProperty");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _reductionOperation, "Reduction operation");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _binAlignment, "Bin alignment");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _numberOfBins, "Number of spatial bins");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _fixYAxisRange, "Fix y-axis range");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _yAxisRangeStart, "Y-axis range start");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _yAxisRangeEnd, "Y-axis range end");
SET_PROPERTY_FIELD_LABEL(BinAndReduceModifier, _sourceProperty, "Source property");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
BinAndReduceModifier::BinAndReduceModifier(DataSet* dataset) : 
    ParticleModifier(dataset), _reductionOperation(RED_MEAN), _binAlignment(2), _numberOfBins(200),
    _fixYAxisRange(false), _yAxisRangeStart(0), _yAxisRangeEnd(0)
{
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_reductionOperation);
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_binAlignment);
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_numberOfBins);
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_fixYAxisRange);
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_yAxisRangeStart);
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_yAxisRangeEnd);
	INIT_PROPERTY_FIELD(BinAndReduceModifier::_sourceProperty);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void BinAndReduceModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Use the first available particle property from the input state as data source when the modifier is newly created.
	if(sourceProperty().isNull()) {
		PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
		ParticlePropertyReference bestProperty;
		for(SceneObject* o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
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
* This modifies the input object.
******************************************************************************/
PipelineStatus BinAndReduceModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
    size_t binDataSize = std::max(1, numberOfBins());
	_binData.resize(binDataSize);
	std::fill(_binData.begin(), _binData.end(), 0);

    // Number of particles for averaging.
    std::vector<int> numberOfParticlesPerBin(binDataSize, 0);

	// Get the source property.
	if(sourceProperty().isNull())
		throw Exception(tr("Select a particle property first."));
	ParticlePropertyObject* property = sourceProperty().findInState(input());
	if(!property)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(sourceProperty().name()));
	if(sourceProperty().vectorComponent() >= (int)property->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(sourceProperty().name()).arg(property->componentCount()));

	size_t vecComponent = sourceProperty().vectorComponent() >= 0 ? sourceProperty().vectorComponent() : 0;
	size_t vecComponentCount = property->componentCount();

    // Get bottom-left and top-right corner of the simulation cell.
    AffineTransformation reciprocalCell = expectSimulationCell()->reciprocalCellMatrix();

    // Compute the surface normal vector.
    Vector3 normal;
    if (_binAlignment == 0) {
        normal = expectSimulationCell()->edgeVector2().cross(expectSimulationCell()->edgeVector3());
    }
    else if (_binAlignment == 1) {
        normal = expectSimulationCell()->edgeVector1().cross(expectSimulationCell()->edgeVector3());
    }
    else {
        normal = expectSimulationCell()->edgeVector1().cross(expectSimulationCell()->edgeVector2());
    }

    // Compute the distance of the two cell faces (normal.length() is area of face).
    FloatType cellVolume = expectSimulationCell()->volume();
    _xAxisRangeStart = 0.0;
    _xAxisRangeEnd = cellVolume / normal.length();

	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	if(property->size() > 0) {
        const Point3* pos = posProperty->constDataPoint3();
        const Point3* pos_end = pos + posProperty->size();

        FloatType binSize = 1.0 / binDataSize;

		if(property->dataType() == qMetaTypeId<FloatType>()) {
			const FloatType* v = property->constDataFloat() + vecComponent;
			const FloatType* v_end = v + (property->size() * vecComponentCount);

            while (pos != pos_end && v != v_end) {
                if (!std::isnan(*v)) {
                    FloatType fractionalPos = reciprocalCell.prodrow(*pos, _binAlignment);
                    size_t binIndex = size_t( fractionalPos / binSize ) % binDataSize;
                    if (_reductionOperation == RED_MEAN || _reductionOperation == RED_SUM || _reductionOperation == RED_SUM_VOL) {
                        _binData[binIndex] += *v;
                    } 
                    else {
                        if (numberOfParticlesPerBin[binIndex] == 0) {
                            _binData[binIndex] = *v;  
                        }
                        else {
                            if (_reductionOperation == RED_MAX) {
                                _binData[binIndex] = std::max(_binData[binIndex], *v);
                            }
                            else if (_reductionOperation == RED_MIN) {
                                _binData[binIndex] = std::min(_binData[binIndex], *v);
                            }
                        }
                    }
                    numberOfParticlesPerBin[binIndex]++;
                }
                
                pos++;
                v += vecComponentCount;
            }
		}
		else if(property->dataType() == qMetaTypeId<int>()) {
			const int* v = property->constDataInt() + vecComponent;
			const int* v_end = v + (property->size() * vecComponentCount);

            while (pos != pos_end && v != v_end) {
                FloatType fractionalPos = reciprocalCell.prodrow(*pos, _binAlignment);
                size_t binIndex = size_t( fractionalPos / binSize ) % binDataSize;
                if (_reductionOperation == RED_MEAN || _reductionOperation == RED_SUM || _reductionOperation == RED_SUM) {
                    _binData[binIndex] += *v;
                }
                else {
                    if (numberOfParticlesPerBin[binIndex] == 0) {
                        _binData[binIndex] = *v;  
                    }
                    else {
                        if (_reductionOperation == RED_MAX) {
                            _binData[binIndex] = std::max(_binData[binIndex], FloatType(*v));
                        }
                        else if (_reductionOperation == RED_MIN) {
                            _binData[binIndex] = std::min(_binData[binIndex], FloatType(*v));
                        }
                    }
                }
                numberOfParticlesPerBin[binIndex]++;

                pos++;
                v += vecComponentCount;
            }
		}

        if (_reductionOperation == RED_MEAN) {
            // Normalize.
            std::vector<FloatType>::iterator a = _binData.begin();
            for (auto n: numberOfParticlesPerBin) {
                if (n > 0) *a /= n;
                a++;
            }
        }
        else if (_reductionOperation == RED_SUM_VOL) {
            // Divide by bin volume.
            FloatType binVolume = cellVolume / binDataSize;
            std::for_each(_binData.begin(), _binData.end(), [binVolume](FloatType &x) { x /= binVolume; });
        }
	}

	if (!_fixYAxisRange) {
		auto minmax = std::minmax_element(_binData.begin(), _binData.end());
		_yAxisRangeStart = *minmax.first;
		_yAxisRangeEnd = *minmax.second;
	}

	notifyDependents(ReferenceEvent::ObjectStatusChanged);

	return PipelineStatus(PipelineStatus::Success);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void BinAndReduceModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Bin and reduce"), rolloutParams /*, "particles.modifiers.binandreduce.html" */);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	ParticlePropertyParameterUI* sourcePropertyUI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_sourceProperty));
	layout->addWidget(new QLabel(tr("Property:"), rollout));
	layout->addWidget(sourcePropertyUI->comboBox());

	layout->addWidget(new QLabel(tr("Reduction operation:"), rollout));
	QGridLayout* gridlayout = new QGridLayout();
	IntegerRadioButtonParameterUI* reductionOperationPUI = new IntegerRadioButtonParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_reductionOperation));
    gridlayout->addWidget(reductionOperationPUI->addRadioButton(BinAndReduceModifier::RED_MEAN, tr("mean")), 0, 0);
    gridlayout->addWidget(reductionOperationPUI->addRadioButton(BinAndReduceModifier::RED_SUM, tr("sum")), 0, 1);
    gridlayout->addWidget(reductionOperationPUI->addRadioButton(BinAndReduceModifier::RED_SUM_VOL, tr("sum/volume")), 0, 2);
    gridlayout->addWidget(reductionOperationPUI->addRadioButton(BinAndReduceModifier::RED_MIN, tr("min")), 0, 3);
    gridlayout->addWidget(reductionOperationPUI->addRadioButton(BinAndReduceModifier::RED_MAX, tr("max")), 0, 4);
    layout->addLayout(gridlayout);

	layout->addWidget(new QLabel(tr("Binning direction:"), rollout));
	gridlayout = new QGridLayout();
	IntegerRadioButtonParameterUI* binAlignmentPUI = new IntegerRadioButtonParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_binAlignment));
    gridlayout->addWidget(binAlignmentPUI->addRadioButton(0, "cell vector 1"), 0, 0);
    gridlayout->addWidget(binAlignmentPUI->addRadioButton(1, "cell vector 2"), 0, 1);
    gridlayout->addWidget(binAlignmentPUI->addRadioButton(2, "cell vector 3"), 0, 2);
    layout->addLayout(gridlayout);

	gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setColumnStretch(1, 1);

	// Number of bins parameter.
	IntegerParameterUI* numBinsPUI = new IntegerParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_numberOfBins));
	gridlayout->addWidget(numBinsPUI->label(), 0, 0);
	gridlayout->addLayout(numBinsPUI->createFieldLayout(), 0, 1);
	numBinsPUI->setMinValue(1);

	layout->addLayout(gridlayout);

	_averagesPlot = new QCustomPlot();
	_averagesPlot->setMinimumHeight(240);
	_averagesPlot->setInteraction(QCP::iRangeDrag, true);
	_averagesPlot->axisRect()->setRangeDrag(Qt::Vertical);
	_averagesPlot->setInteraction(QCP::iRangeZoom, true);
	_averagesPlot->axisRect()->setRangeZoom(Qt::Vertical);
	_averagesPlot->xAxis->setLabel("Position");
	_averagesPlot->addGraph();
	connect(_averagesPlot->yAxis, SIGNAL(rangeChanged(const QCPRange&)), this, SLOT(updateYAxisRange(const QCPRange&)));

	layout->addWidget(new QLabel(tr("Reduction:")));
	layout->addWidget(_averagesPlot);
	connect(this, &BinAndReduceModifierEditor::contentsReplaced, this, &BinAndReduceModifierEditor::plotAverages);

	QPushButton* saveDataButton = new QPushButton(tr("Save data"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, &QPushButton::clicked, this, &BinAndReduceModifierEditor::onSaveData);

	// Axes.
	QGroupBox* axesBox = new QGroupBox(tr("Plot axes"), rollout);
	QVBoxLayout* axesSublayout = new QVBoxLayout(axesBox);
	axesSublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(axesBox);
    BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_fixYAxisRange));
    axesSublayout->addWidget(rangeUI->checkBox());
        
    QHBoxLayout* hlayout = new QHBoxLayout();
    axesSublayout->addLayout(hlayout);
    FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_yAxisRangeStart));
    FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(BinAndReduceModifier::_yAxisRangeEnd));
    hlayout->addWidget(new QLabel(tr("From:")));
    hlayout->addLayout(startPUI->createFieldLayout());
    hlayout->addSpacing(12);
    hlayout->addWidget(new QLabel(tr("To:")));
    hlayout->addLayout(endPUI->createFieldLayout());
    startPUI->setEnabled(false);
    endPUI->setEnabled(false);
    connect(rangeUI->checkBox(), &QCheckBox::toggled, startPUI, &FloatParameterUI::setEnabled);
    connect(rangeUI->checkBox(), &QCheckBox::toggled, endPUI, &FloatParameterUI::setEnabled);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool BinAndReduceModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->sender() == editObject() && 
       event->type() == ReferenceEvent::ObjectStatusChanged) {
		plotAverages();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Replots the averaged data computed by the modifier.
******************************************************************************/
void BinAndReduceModifierEditor::plotAverages()
{
	BinAndReduceModifier* modifier = static_object_cast<BinAndReduceModifier>(editObject());
	if(!modifier)
		return;

	_averagesPlot->yAxis->setLabel(modifier->sourceProperty().name());

	if(modifier->binData().empty())
		return;

    size_t binDataSize = modifier->binData().size();
	QVector<double> xdata(binDataSize);
	QVector<double> ydata(binDataSize);
	double binSize = ( modifier->xAxisRangeEnd() - modifier->xAxisRangeStart() ) / binDataSize;
	double maxBinData = 0.0;
	for(int i = 0; i < xdata.size(); i++) {
		xdata[i] = binSize * ((double)i + 0.5);
		ydata[i] = modifier->binData()[i];
		maxBinData = std::max(maxBinData, ydata[i]);
	}
	_averagesPlot->graph()->setLineStyle(QCPGraph::lsStepCenter);
	_averagesPlot->graph()->setData(xdata, ydata);

	// Check if range is already correct, because setRange emits the rangeChanged signal
	// which is to be avoided if the range is not determined automatically.
	_rangeUpdate = false;
	_averagesPlot->xAxis->setRange(modifier->xAxisRangeStart(), modifier->xAxisRangeEnd());
	_averagesPlot->yAxis->setRange(modifier->yAxisRangeStart(), modifier->yAxisRangeEnd());
	_rangeUpdate = true;

	_averagesPlot->replot();
}

/******************************************************************************
* Keep y-axis range updated
******************************************************************************/
void BinAndReduceModifierEditor::updateYAxisRange(const QCPRange &newRange)
{
	if (_rangeUpdate) {
		BinAndReduceModifier* modifier = static_object_cast<BinAndReduceModifier>(editObject());
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
void BinAndReduceModifierEditor::onSaveData()
{
	BinAndReduceModifier* modifier = static_object_cast<BinAndReduceModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->binData().empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(mainWindow(),
	    tr("Save Averages"), QString(), 
        tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Could not open file for writing: %1").arg(file.errorString()));

		QTextStream stream(&file);
		FloatType binSize = (modifier->xAxisRangeEnd() - modifier->xAxisRangeStart()) / modifier->binData().size();
		stream << "# " << modifier->sourceProperty().name() << " spatial averages (bin size: " << binSize << ")" << endl;
		for(int i = 0; i < modifier->binData().size(); i++) {
			stream << (binSize * (FloatType(i) + 0.5f) + modifier->xAxisRangeStart()) << " " <<
					modifier->binData()[i] << endl;
		}
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}


};	// End of namespace
