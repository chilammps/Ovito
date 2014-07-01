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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/AnimationSettings.h>
#include <plugins/particles/util/ParticlePropertyParameterUI.h>
#include "SpatialCorrelationFunctionModifier.h"

namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, SpatialCorrelationFunctionModifier, ParticleModifier);
IMPLEMENT_OVITO_OBJECT(Particles, SpatialCorrelationFunctionModifierEditor, ParticleModifierEditor);
SET_OVITO_OBJECT_EDITOR(SpatialCorrelationFunctionModifier, SpatialCorrelationFunctionModifierEditor);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _binDirection, "BinDirection", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _maxWaveVector, "MaxWaveVector", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _radialAverage, "radialAverage", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _numberOfRadialBins, "NumberOfRadialBins", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _numberOfBinsX, "NumberOfBinsX", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _numberOfBinsY, "NumberOfBinsY", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _fixPropertyAxisRange, "FixPropertyAxisRange");
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _propertyAxisRangeStart, "PropertyAxisRangeStart", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _propertyAxisRangeEnd, "PropertyAxisRangeEnd", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _sourceProperty1, "SourceProperty1");
DEFINE_PROPERTY_FIELD(SpatialCorrelationFunctionModifier, _sourceProperty2, "SourceProperty2");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _binDirection, "Bin direction");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _maxWaveVector, "Maximum wavevector");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _radialAverage, "Radial average");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _numberOfRadialBins, "Number of radial bins");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _numberOfBinsX, "Max. wavevector");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _numberOfBinsY, "Max. wavevector");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _fixPropertyAxisRange, "Fix property axis range");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _propertyAxisRangeStart, "Property axis range start");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _propertyAxisRangeEnd, "Property axis range end");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _sourceProperty1, "First source property");
SET_PROPERTY_FIELD_LABEL(SpatialCorrelationFunctionModifier, _sourceProperty2, "Second source property");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
SpatialCorrelationFunctionModifier::SpatialCorrelationFunctionModifier(DataSet* dataset) : 
    ParticleModifier(dataset),
    _binDirection(CELL_VECTORS_1_2), _maxWaveVector(0.5), _radialAverage(false),
    _numberOfRadialBins(20), _numberOfBinsX(20), _numberOfBinsY(20),
    _fixPropertyAxisRange(false), _propertyAxisRangeStart(0), _propertyAxisRangeEnd(0)
{
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_binDirection);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_maxWaveVector);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_radialAverage);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_numberOfRadialBins);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_numberOfBinsX);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_numberOfBinsY);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_fixPropertyAxisRange);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_propertyAxisRangeStart);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_propertyAxisRangeEnd);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_sourceProperty1);
	INIT_PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_sourceProperty2);
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void SpatialCorrelationFunctionModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

	// Use the first available particle property from the input state as data source when the modifier is newly created.
	if(sourceProperty1().isNull() || sourceProperty1().isNull()) {
		PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
		ParticlePropertyReference bestProperty;
		for(SceneObject* o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
			if(property && (property->dataType() == qMetaTypeId<int>() || property->dataType() == qMetaTypeId<FloatType>())) {
				bestProperty = ParticlePropertyReference(property, (property->componentCount() > 1) ? 0 : -1);
			}
		}
		if(!bestProperty.isNull()) {
            if (sourceProperty1().isNull()) {
                setSourceProperty1(bestProperty);
            }
            if (sourceProperty2().isNull()) {
                setSourceProperty2(bestProperty);
            }
		}
	}
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus SpatialCorrelationFunctionModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the source property.
	if(sourceProperty1().isNull() || sourceProperty2().isNull())
		throw Exception(tr("Select particle properties first."));

    // Get the first property.
    ParticlePropertyObject* property1 = sourceProperty1().findInState(input());
	if(!property1)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(sourceProperty1().name()));
	if(sourceProperty1().vectorComponent() >= (int)property1->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(sourceProperty1().name()).arg(property1->componentCount()));

	size_t vecComponent1 = sourceProperty1().vectorComponent() >= 0 ? sourceProperty1().vectorComponent() : 0;
	size_t vecComponentCount1 = property1->componentCount();

    // Get the second property.
    ParticlePropertyObject* property2 = sourceProperty2().findInState(input());
	if(!property2)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(sourceProperty2().name()));
	if(sourceProperty2().vectorComponent() >= (int)property2->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(sourceProperty2().name()).arg(property2->componentCount()));

	size_t vecComponent2 = sourceProperty2().vectorComponent() >= 0 ? sourceProperty2().vectorComponent() : 0;
	size_t vecComponentCount2 = property2->componentCount();

    // Get bottom-left and top-right corner of the simulation cell.
    AffineTransformation reciprocalCell = expectSimulationCell()->reciprocalCellMatrix();

    // Compute the surface normal vector.
    Vector3 recX, recY;
    if (_binDirection == CELL_VECTORS_1_2) {
        recX = reciprocalCell.column(0);
        recY = reciprocalCell.column(1);
    }
    else if (_binDirection == CELL_VECTORS_2_3) {
        recX = reciprocalCell.column(1);
        recY = reciprocalCell.column(2);
    }
    else if (_binDirection == CELL_VECTORS_1_3) {
        recX = reciprocalCell.column(0);
        recY = reciprocalCell.column(2);
    }

    FloatType recXLength = recX.length();
    FloatType recYLength = recY.length();
    size_t binDataSizeX = size_t(std::ceil(_maxWaveVector/recXLength))+1;
    size_t binDataSizeY = size_t(std::ceil(_maxWaveVector/recYLength))+1;
    _numberOfBinsX = binDataSizeX;
    _numberOfBinsY = binDataSizeY;

    size_t binDataSize = binDataSizeX*binDataSizeY;
	_binData1.resize(binDataSize);
	_binData2.resize(binDataSize);
    _binData.resize(binDataSize);
	std::fill(_binData1.begin(), _binData1.end(), 0.0);
	std::fill(_binData2.begin(), _binData2.end(), 0.0);
	std::fill(_binData.begin(), _binData.end(), 0.0);

    // Compute the distance of the two cell faces (normal.length() is area of face).
    FloatType cellVolume = expectSimulationCell()->volume();
    _xAxisRangeStart = -0.5*recXLength;
    _xAxisRangeEnd = (binDataSizeX-0.5)*recXLength;
    _yAxisRangeStart = -0.5*recYLength;
    _yAxisRangeEnd = (binDataSizeY-0.5)*recYLength;
    _xDataRangeStart = 0.0;
    _xDataRangeEnd = (binDataSizeX-1.0)*recXLength;
    _yDataRangeStart = 0.0;
    _yDataRangeEnd = (binDataSizeY-1.0)*recYLength;

	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	if(property1->size() > 0 && property2->size() > 0) {
        const Point3* pos = posProperty->constDataPoint3();
        const Point3* pos_end = pos + posProperty->size();

        int particleCount = 0;
		if(property1->dataType() == qMetaTypeId<FloatType>() && property2->dataType() == qMetaTypeId<FloatType>()) {
			const FloatType* v1 = property1->constDataFloat() + vecComponent1;
			const FloatType* v1_end = v1 + (property1->size() * vecComponentCount1);
			const FloatType* v2 = property2->constDataFloat() + vecComponent2;
			const FloatType* v2_end = v2 + (property2->size() * vecComponentCount2);

            while (pos != pos_end && v1 != v1_end && v2 != v2_end) {
                if (!std::isnan(*v1) && !std::isnan(*v2)) {
                    FloatType X = 2*M_PI*(recX.x()*pos->x()+recX.y()*pos->y()+recX.z()*pos->z());
                    FloatType Y = 2*M_PI*(recY.x()*pos->x()+recY.y()*pos->y()+recY.z()*pos->z());
                    for (int binIndexY = 0; binIndexY < binDataSizeY; binIndexY++) {
                        for (int binIndexX = 0; binIndexX < binDataSizeX; binIndexX++) {
                            size_t binIndex = binIndexY*binDataSizeX+binIndexX;
                            std::complex<FloatType> phase = std::exp(std::complex<FloatType>(0.0, -binIndexX*X-binIndexY*Y));
                            _binData1[binIndex] += (*v1)*phase;
                            _binData2[binIndex] += (*v2)*phase;
                        }
                    }
                }
                
                pos++;
                v1 += vecComponentCount1;
                v2 += vecComponentCount2;
                particleCount++;
            }
		}

        // Normalize and compute correlation function.
        if (particleCount > 0) {
            for (int binIndex = 0; binIndex < binDataSize; binIndex++) {
                _binData1[binIndex] /= particleCount;
                _binData2[binIndex] /= particleCount;
                _binData[binIndex] = std::real(_binData1[binIndex]*std::conj(_binData2[binIndex]));
            }
        }
	}

    if (_radialAverage) {
        std::vector<int> numberOfDataPoints(numberOfRadialBins(), 0);
        _radialBinData.resize(numberOfRadialBins());
        std::fill(_radialBinData.begin(), _radialBinData.end(), 0.0);

        for (int j = 0; j < binDataSizeY; j++) {
            for (int i = 0; i < binDataSizeX; i++) {
                FloatType waveVector = sqrt(i*recXLength*i*recXLength + j*recYLength*j*recYLength);
                int binIndex = int(std::floor(waveVector*numberOfRadialBins()/_maxWaveVector));

                if (binIndex < numberOfRadialBins()) {
                    _radialBinData[binIndex] += _binData[j*binDataSizeX+i];
                    numberOfDataPoints[binIndex]++;
                }
            }
        }

        for (int binIndex = 0; binIndex < numberOfRadialBins(); binIndex++) {
            _radialBinData[binIndex] /= numberOfDataPoints[binIndex];
        }
    }

	if (!_fixPropertyAxisRange) {
        if (_radialAverage) {
            auto minmax = std::minmax_element(_radialBinData.begin(), _radialBinData.end());
            _propertyAxisRangeStart = *minmax.first;
            _propertyAxisRangeEnd = *minmax.second;
        }
        else {
            auto minmax = std::minmax_element(_binData.begin(), _binData.end());
            _propertyAxisRangeStart = *minmax.first;
            _propertyAxisRangeEnd = *minmax.second;
        }
	}

	notifyDependents(ReferenceEvent::ObjectStatusChanged);

	return PipelineStatus(PipelineStatus::Success);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SpatialCorrelationFunctionModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Spatial correlation function"), rolloutParams /*, "particles.modifiers.binandreduce.html" */);

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	ParticlePropertyParameterUI* sourceProperty1UI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_sourceProperty1));
	ParticlePropertyParameterUI* sourceProperty2UI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_sourceProperty2));
	layout->addWidget(new QLabel(tr("Property:"), rollout));
	layout->addWidget(sourceProperty1UI->comboBox());
    layout->addWidget(sourceProperty2UI->comboBox());

	QGridLayout *gridlayout = new QGridLayout();
	gridlayout->addWidget(new QLabel(tr("Binning direction:"), rollout), 0, 0);
	VariantComboBoxParameterUI* binDirectionPUI = new VariantComboBoxParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_binDirection));
    binDirectionPUI->comboBox()->addItem("reciprocal vectors 1 and 2", qVariantFromValue(SpatialCorrelationFunctionModifier::CELL_VECTORS_1_2));
    binDirectionPUI->comboBox()->addItem("reciprocal vectors 1 and 3", qVariantFromValue(SpatialCorrelationFunctionModifier::CELL_VECTORS_1_3));
    binDirectionPUI->comboBox()->addItem("reciprocal vectors 2 and 3", qVariantFromValue(SpatialCorrelationFunctionModifier::CELL_VECTORS_2_3));
    gridlayout->addWidget(binDirectionPUI->comboBox(), 0, 1);
    layout->addLayout(gridlayout);

	gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(0,0,0,0);
	gridlayout->setColumnStretch(1, 1);
	//gridlayout->setColumnStretch(2, 1);

	// Number of bins parameters.
	FloatParameterUI* maxWaveVectorPUI = new FloatParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_maxWaveVector));
	gridlayout->addWidget(maxWaveVectorPUI->label(), 0, 0);
	gridlayout->addLayout(maxWaveVectorPUI->createFieldLayout(), 0, 1);
	maxWaveVectorPUI->setMinValue(0.0);

	layout->addLayout(gridlayout);

	QGroupBox* radialAverageBox = new QGroupBox(tr("Radial average"), rollout);
	QVBoxLayout* radialAverageSublayout = new QVBoxLayout(radialAverageBox);
	radialAverageSublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(radialAverageBox);

	BooleanParameterUI *radialAveragePUI = new BooleanParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_radialAverage));
	radialAverageSublayout->addWidget(radialAveragePUI->checkBox());

	// Number of bins parameters.
	IntegerParameterUI* numRadialBinsPUI = new IntegerParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_numberOfRadialBins));
    gridlayout = new QGridLayout();
	gridlayout->addWidget(numRadialBinsPUI->label(), 0, 0);
    gridlayout->addLayout(numRadialBinsPUI->createFieldLayout(), 0, 1);
	radialAverageSublayout->addLayout(gridlayout);
	numRadialBinsPUI->setMinValue(1);
    numRadialBinsPUI->setEnabled(false);
    connect(radialAveragePUI->checkBox(), &QCheckBox::toggled, numRadialBinsPUI, &IntegerParameterUI::setEnabled);

	_correlationFunctionPlot = new QCustomPlot();
	_correlationFunctionPlot->setMinimumHeight(240);
    _correlationFunctionPlot->axisRect()->setRangeDrag(Qt::Vertical);
    _correlationFunctionPlot->axisRect()->setRangeZoom(Qt::Vertical);
	_correlationFunctionPlot->xAxis->setLabel("Wavevector");

	layout->addWidget(new QLabel(tr("Spatial correlation function:")));
	layout->addWidget(_correlationFunctionPlot);
	connect(this, &SpatialCorrelationFunctionModifierEditor::contentsReplaced, this, &SpatialCorrelationFunctionModifierEditor::plotSpatialCorrelationFunction);

	QPushButton* saveDataButton = new QPushButton(tr("Save data"));
	layout->addWidget(saveDataButton);
	connect(saveDataButton, &QPushButton::clicked, this, &SpatialCorrelationFunctionModifierEditor::onSaveData);

	// Axes.
	QGroupBox* axesBox = new QGroupBox(tr("Plot axes"), rollout);
	QVBoxLayout* axesSublayout = new QVBoxLayout(axesBox);
	axesSublayout->setContentsMargins(4,4,4,4);
	layout->addWidget(axesBox);
    BooleanParameterUI* rangeUI = new BooleanParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_fixPropertyAxisRange));
    axesSublayout->addWidget(rangeUI->checkBox());
        
    QHBoxLayout* hlayout = new QHBoxLayout();
    axesSublayout->addLayout(hlayout);
    FloatParameterUI* startPUI = new FloatParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_propertyAxisRangeStart));
    FloatParameterUI* endPUI = new FloatParameterUI(this, PROPERTY_FIELD(SpatialCorrelationFunctionModifier::_propertyAxisRangeEnd));
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
bool SpatialCorrelationFunctionModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->sender() == editObject()
			&& event->type() == ReferenceEvent::ObjectStatusChanged) {
			plotSpatialCorrelationFunction();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Replots the averaged data computed by the modifier.
******************************************************************************/
void SpatialCorrelationFunctionModifierEditor::plotSpatialCorrelationFunction()
{
	SpatialCorrelationFunctionModifier* modifier = static_object_cast<SpatialCorrelationFunctionModifier>(editObject());
	if(!modifier)
		return;
    
    if (modifier->radialAverage()) {
        size_t binDataSize = modifier->numberOfRadialBins();

        // If previous plot was a color map, delete and create graph.
        if (!_correlationFunctionGraph) {
            if (_correlationFunctionColorMap) {
                _correlationFunctionPlot->removePlottable(_correlationFunctionColorMap);
                _correlationFunctionColorMap = NULL;
            }
            _correlationFunctionGraph = _correlationFunctionPlot->addGraph();
        }

        _correlationFunctionPlot->setInteraction(QCP::iRangeDrag, true);
        _correlationFunctionPlot->axisRect()->setRangeDrag(Qt::Vertical);
        _correlationFunctionPlot->setInteraction(QCP::iRangeZoom, true);
        _correlationFunctionPlot->axisRect()->setRangeZoom(Qt::Vertical);
        _correlationFunctionPlot->yAxis->setLabel("Re[<"+modifier->sourceProperty1().name()+"*"+modifier->sourceProperty1().name()+">]");

        if(modifier->binData().empty())
            return;

        QVector<double> xdata(binDataSize);
        QVector<double> ydata(binDataSize);
        double binSize = modifier->maxWaveVector() / binDataSize;
        for(int i = 0; i < binDataSize; i++) {
            xdata[i] = binSize * ((double)i + 0.5);
            ydata[i] = modifier->radialBinData()[i];
        }
        _correlationFunctionPlot->graph()->setData(xdata, ydata);

        // Check if range is already correct, because setRange emits the rangeChanged signal
        // which is to be avoided if the range is not determined automatically.
        _correlationFunctionPlot->xAxis->setRange(0.0, modifier->maxWaveVector());
        _correlationFunctionPlot->yAxis->setRange(modifier->propertyAxisRangeStart(), modifier->propertyAxisRangeEnd());
    }
    else {
        size_t binDataSizeX = std::max(1, modifier->numberOfBinsX());
        size_t binDataSizeY = std::max(1, modifier->numberOfBinsY());
        size_t bnDataSize = binDataSizeX*binDataSizeY;

        // If previous plot was a graph, delete and create color map.
        if (!_correlationFunctionColorMap) {
            if (_correlationFunctionGraph) {
                _correlationFunctionPlot->removeGraph(_correlationFunctionGraph);
                _correlationFunctionGraph = NULL;
            }
            _correlationFunctionColorMap = new QCPColorMap(_correlationFunctionPlot->xAxis, _correlationFunctionPlot->yAxis);
            _correlationFunctionPlot->addPlottable(_correlationFunctionColorMap);
        }
        
        _correlationFunctionPlot->setInteraction(QCP::iRangeDrag, false);
        _correlationFunctionPlot->setInteraction(QCP::iRangeZoom, false);
        _correlationFunctionPlot->yAxis->setLabel("Wavevector");

        if(modifier->binData().empty())
            return;

        _correlationFunctionColorMap->setInterpolate(false);
        _correlationFunctionColorMap->setTightBoundary(false);
        _correlationFunctionColorMap->setGradient(QCPColorGradient::gpJet);

        _correlationFunctionColorMap->data()->setSize(binDataSizeX, binDataSizeY);
        _correlationFunctionColorMap->data()->setRange(QCPRange(modifier->xDataRangeStart(), modifier->xDataRangeEnd()),
                                                       QCPRange(modifier->yDataRangeStart(), modifier->yDataRangeEnd()));
    
        _correlationFunctionPlot->xAxis->setRange(QCPRange(modifier->xAxisRangeStart(), modifier->xAxisRangeEnd()));
        _correlationFunctionPlot->yAxis->setRange(QCPRange(modifier->yAxisRangeStart(), modifier->yAxisRangeEnd()));
    
        // Copy data to QCPColorMapData object.
        for (int j = 0; j < binDataSizeY; j++) {
            for (int i = 0; i < binDataSizeX; i++) {
                _correlationFunctionColorMap->data()->setCell(i, j, modifier->binData()[j*binDataSizeX+i]);
            }
        }
        
        // Check if range is already correct, because setRange emits the rangeChanged signal
        // which is to be avoided if the range is not determined automatically.
        _correlationFunctionColorMap->setDataRange(QCPRange(modifier->propertyAxisRangeStart(), modifier->propertyAxisRangeEnd()));
    }
    
    _correlationFunctionPlot->replot();
}

/******************************************************************************
* This is called when the user has clicked the "Save Data" button.
******************************************************************************/
void SpatialCorrelationFunctionModifierEditor::onSaveData()
{
	SpatialCorrelationFunctionModifier* modifier = static_object_cast<SpatialCorrelationFunctionModifier>(editObject());
	if(!modifier)
		return;

	if(modifier->binData().empty())
		return;

	QString fileName = QFileDialog::getSaveFileName(mainWindow(),
	    tr("Save Data"), QString(), 
        tr("Text files (*.txt);;All files (*)"));
	if(fileName.isEmpty())
		return;

	try {

		QFile file(fileName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
			throw Exception(tr("Could not open file for writing: %1").arg(file.errorString()));

        size_t binDataSizeX = std::max(1, modifier->numberOfBinsX());
        size_t binDataSizeY = std::max(1, modifier->numberOfBinsY());
		FloatType binSizeX = (modifier->xAxisRangeEnd() - modifier->xAxisRangeStart()) / binDataSizeX;
		FloatType binSizeY = (modifier->yAxisRangeEnd() - modifier->yAxisRangeStart()) / binDataSizeY;

		QTextStream stream(&file);
        if (binDataSizeY == 1) {
            stream << "# " << modifier->sourceProperty1().name() << " bin size: " << binSizeX << endl;
            for(int i = 0; i < modifier->binData().size(); i++) {
                stream << (binSizeX * (FloatType(i) + 0.5f) + modifier->xAxisRangeStart()) << " " << modifier->binData()[i] << endl;
            }
        }
        else {
            stream << "# " << modifier->sourceProperty1().name() << " bin size X: " << binDataSizeX << ", bin size Y: " << binDataSizeY << endl;
            for(int i = 0; i < binDataSizeY; i++) {
                for(int j = 0; j < binDataSizeX; j++) {
                    stream << modifier->binData()[i*binDataSizeX+j] << " ";
                }
                stream << endl;
            }
        }
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}


};	// End of namespace
