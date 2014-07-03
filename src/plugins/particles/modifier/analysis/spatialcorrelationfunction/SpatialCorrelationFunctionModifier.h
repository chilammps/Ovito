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

#ifndef __OVITO_SPATIAL_CORRELATION_FUNCTION_MODIFIER_H
#define __OVITO_SPATIAL_CORRELATION_FUNCTION_MODIFIER_H

#include <complex>

#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/data/ParticlePropertyObject.h>
#include "../../ParticleModifier.h"
#include <qcustomplot.h>

class QCustomPlot;
class QCPItemStraightLine;

namespace Particles {

/*
 * This modifier computes the Fourier transform of a (spatial) cross correlation function
 * between two particle properties.
 */
class OVITO_PARTICLES_EXPORT SpatialCorrelationFunctionModifier : public ParticleModifier
{
public:

    enum BinDirectionType { CELL_VECTORS_1_2 = 0, CELL_VECTORS_1_3 = 1, CELL_VECTORS_2_3 = 2 };
    Q_ENUMS(BinDirectionType);

	/// Constructor.
	Q_INVOKABLE SpatialCorrelationFunctionModifier(DataSet* dataset);

	/// This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipelineObject, ModifierApplication* modApp) override;

	/// Sets the first source particle property for which the correlation function is computed.
	void setSourceProperty1(const ParticlePropertyReference& prop) { _sourceProperty1 = prop; }

	/// Returns the first source particle property for which the correlation function is computed.
	const ParticlePropertyReference& sourceProperty1() const { return _sourceProperty1; }

	/// Sets the second source particle property for which the correlation function is computed.
	void setSourceProperty2(const ParticlePropertyReference& prop) { _sourceProperty2 = prop; }

	/// Returns the second source particle property for which the correlation function is computed.
	const ParticlePropertyReference& sourceProperty2() const { return _sourceProperty2; }

	/// Returns the bin direction
	BinDirectionType binDirection() const { return _binDirection; }

	/// Sets the bin direction
	void setBinDirection(BinDirectionType o) { _binDirection = o; }

	/// Returns the number of spatial bins of the computed correlation value.
	FloatType maxWaveVector() const { return _maxWaveVector; }

	/// Sets the number of spatial bins of the computed correlation value.
	void setMaxWaveVector(FloatType v) { _maxWaveVector = v; }

	/// Returns the number of spatial bins of the computed correlation value.
	int numberOfRadialBins() const { return _numberOfRadialBins; }

	/// Sets the number of spatial bins of the computed correlation value.
	void setNumberOfRadialBins(int n) { _numberOfRadialBins = n; }

	/// Returns the number of spatial bins of the computed correlation value.
	int numberOfBinsX() const { return _numberOfBinsX; }

	/// Sets the number of spatial bins of the computed correlation value.
	void setNumberOfBinsX(int n) { _numberOfBinsX = n; }

	/// Returns the number of spatial bins of the computed correlation value.
	int numberOfBinsY() const { return _numberOfBinsY; }

	/// Sets the number of spatial bins of the computed correlation value.
	void setNumberOfBinsY(int n) { _numberOfBinsY = n; }

	/// Returns compute first derivative
	bool radialAverage() const { return _radialAverage; }

	/// Returns the stored correlation function.
	const std::vector<FloatType>& binData() const { return _binData; }

	/// Returns the stored radially averaged correlation function.
	const std::vector<FloatType>& radialBinData() const { return _radialBinData; }

	/// Returns the start value of the plotting x-axis.
	FloatType xAxisRangeStart() const { return _xAxisRangeStart; }

	/// Returns the end value of the plotting x-axis.
	FloatType xAxisRangeEnd() const { return _xAxisRangeEnd; }

	/// Returns the start value of the plotting y-axis.
	FloatType yAxisRangeStart() const { return _yAxisRangeStart; }

	/// Returns the end value of the plotting y-axis.
	FloatType yAxisRangeEnd() const { return _yAxisRangeEnd; }

	/// Returns the start value of the plotting x-data.
	FloatType xDataRangeStart() const { return _xDataRangeStart; }

	/// Returns the end value of the plotting x-data.
	FloatType xDataRangeEnd() const { return _xDataRangeEnd; }

	/// Returns the start value of the plotting y-data.
	FloatType yDataRangeStart() const { return _yDataRangeStart; }

	/// Returns the end value of the plotting y-data.
	FloatType yDataRangeEnd() const { return _yDataRangeEnd; }

	/// Set whether the plotting range of the property axis should be fixed.
	void setFixPropertyAxisRange(bool fix) { _fixPropertyAxisRange = fix; }

	/// Returns whether the plotting range of the property axis should be fixed.
	bool fixPropertyAxisRange() const { return _fixPropertyAxisRange; }

	/// Set start and end value of the plotting property axis.
	void setPropertyAxisRange(FloatType start, FloatType end) { _propertyAxisRangeStart = start; _propertyAxisRangeEnd = end; }

	/// Returns the start value of the plotting y-axis.
	FloatType propertyAxisRangeStart() const { return _propertyAxisRangeStart; }

	/// Returns the end value of the plotting y-axis.
	FloatType propertyAxisRangeEnd() const { return _propertyAxisRangeEnd; }

public:

	Q_PROPERTY(Particles::ParticlePropertyReference sourceProperty1 READ sourceProperty1 WRITE setSourceProperty1);
	Q_PROPERTY(Particles::ParticlePropertyReference sourceProperty2 READ sourceProperty2 WRITE setSourceProperty2);
	Q_PROPERTY(Particles::SpatialCorrelationFunctionModifier::BinDirectionType binDirection READ binDirection WRITE setBinDirection);
	Q_PROPERTY(int numberOfBinsX READ numberOfBinsX WRITE setNumberOfBinsX);
	Q_PROPERTY(int numberOfBinsY READ numberOfBinsY WRITE setNumberOfBinsY);

protected:

	/// Modifies the particle object.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// First particle property that serves as data source for the correlation.
	PropertyField<ParticlePropertyReference> _sourceProperty1;

	/// Second particle property that serves as data source for the correlation.
	PropertyField<ParticlePropertyReference> _sourceProperty2;

	/// Bin alignment
	PropertyField<BinDirectionType,int> _binDirection;

	/// Controls the wave-vector cutoff.
	PropertyField<FloatType> _maxWaveVector;

	/// Controls whether to compute a radial average.
	PropertyField<bool> _radialAverage;

	/// Controls the number of spatial bins.
	PropertyField<int> _numberOfRadialBins;

	/// Controls the number of spatial bins.
	PropertyField<int> _numberOfBinsX;

	/// Controls the number of spatial bins.
	PropertyField<int> _numberOfBinsY;

	/// Controls the whether the plotting range along the y-axis should be fixed.
	PropertyField<bool> _fixPropertyAxisRange;

	/// Controls the start value of the plotting y-axis.
	PropertyField<FloatType> _propertyAxisRangeStart;

	/// Controls the end value of the plotting y-axis.
	PropertyField<FloatType> _propertyAxisRangeEnd;

	/// Stores the start value of the plotting x-axis.
	FloatType _xAxisRangeStart;

	/// Stores the end value of the plotting x-axis.
	FloatType _xAxisRangeEnd;

	/// Stores the start value of the plotting y-axis.
	FloatType _yAxisRangeStart;

	/// Stores the end value of the plotting y-axis.
	FloatType _yAxisRangeEnd;

	/// Stores the start value of the plotting x-data.
	FloatType _xDataRangeStart;

	/// Stores the end value of the plotting x-data.
	FloatType _xDataRangeEnd;

	/// Stores the start value of the plotting y-data.
	FloatType _yDataRangeStart;

	/// Stores the end value of the plotting y-data.
	FloatType _yDataRangeEnd;

	/// Stores the Fourier transform of property 1.
	std::vector<std::complex<FloatType>> _binData1;

	/// Stores the Fourier transform of property 2.
	std::vector<std::complex<FloatType>> _binData2;

	/// Stores the correlation function.
	std::vector<FloatType> _binData;

	/// Stores the radially averaged correlation function.
	std::vector<FloatType> _radialBinData;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Spatial correlation function");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_binDirection);
	DECLARE_PROPERTY_FIELD(_maxWaveVector);
	DECLARE_PROPERTY_FIELD(_radialAverage);
	DECLARE_PROPERTY_FIELD(_numberOfRadialBins);
	DECLARE_PROPERTY_FIELD(_numberOfBinsX);
	DECLARE_PROPERTY_FIELD(_numberOfBinsY);
	DECLARE_PROPERTY_FIELD(_fixPropertyAxisRange);
	DECLARE_PROPERTY_FIELD(_propertyAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_propertyAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_sourceProperty1);
	DECLARE_PROPERTY_FIELD(_sourceProperty2);
};

/******************************************************************************
* A properties editor for the SpatialCorrelationFunctionModifier class.
******************************************************************************/
class SpatialCorrelationFunctionModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE SpatialCorrelationFunctionModifierEditor() : _correlationFunctionGraph(nullptr), _correlationFunctionColorMap(nullptr) {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Replots the average data computed by the modifier.
	void plotSpatialCorrelationFunction();

	/// This is called when the user has clicked the "Save Data" button.
	void onSaveData();

private:

	/// The plot widget to display the average data.
	QCustomPlot* _correlationFunctionPlot;

	/// The graph widget to display the average data.
	QCPGraph* _correlationFunctionGraph;

	/// The color map widget to display the average data on a 2D grid.
	QCPColorMap* _correlationFunctionColorMap;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

Q_DECLARE_METATYPE(Particles::SpatialCorrelationFunctionModifier::BinDirectionType);
Q_DECLARE_TYPEINFO(Particles::SpatialCorrelationFunctionModifier::BinDirectionType, Q_PRIMITIVE_TYPE);

#endif // __OVITO_SPATIAL_CORRELATION_FUNCTION_MODIFIER_H
