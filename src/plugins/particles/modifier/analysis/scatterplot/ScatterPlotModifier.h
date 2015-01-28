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

#ifndef __OVITO_SCATTER_PLOT_MODIFIER_H
#define __OVITO_SCATTER_PLOT_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/ParticlePropertyComboBox.h>
#include "../../ParticleModifier.h"

#ifndef signals
#define signals Q_SIGNALS
#endif
#include <qcustomplot.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

/**
 * \brief This modifier computes a scatter plot for two particle properties.
 */
class OVITO_PARTICLES_EXPORT ScatterPlotModifier : public ParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE ScatterPlotModifier(DataSet* dataset);

	/// Sets the source particle property for which the scatter plot should be computed.
	void setXAxisProperty(const ParticlePropertyReference& prop) { _xAxisProperty = prop; }

	/// Returns the source particle property for which the scatter plot is computed.
	const ParticlePropertyReference& xAxisProperty() const { return _xAxisProperty; }

	/// Sets the source particle property for which the scatter plot should be computed.
	void setYAxisProperty(const ParticlePropertyReference& prop) { _yAxisProperty = prop; }

	/// Returns the source particle property for which the scatter plot is computed.
	const ParticlePropertyReference& yAxisProperty() const { return _yAxisProperty; }

	/// Return the number of particle type ids.
	int numberOfParticleTypeIds() const { return _xData.size(); }

	/// Returns the stored scatter plot data (x-axis).
	const QVector<double>& xData(int particleTypeId) const { return _xData[particleTypeId]; }

	/// Returns the stored scatter plot data (y-axis).
	const QVector<double>& yData(int particleTypeId) const { return _yData[particleTypeId]; }

	/// Returns whether particles within the specified range should be selected (x-axis).
	bool selectXAxisInRange() const { return _selectXAxisInRange; }

	/// Sets whether particles within the specified range should be selected (x-axis).
	void setSelectXAxisInRange(bool select) { _selectXAxisInRange = select; }

	/// Returns the start value of the selection interval (x-axis).
	FloatType selectionXAxisRangeStart() const { return _selectionXAxisRangeStart; }

	/// Returns the end value of the selection interval (x-axis).
	FloatType selectionXAxisRangeEnd() const { return _selectionXAxisRangeEnd; }

	/// Returns whether particles within the specified range should be selected (y-axis).
	bool selectYAxisInRange() const { return _selectYAxisInRange; }

	/// Sets whether particles within the specified range should be selected (y-axis).
	void setSelectYAxisInRange(bool select) { _selectYAxisInRange = select; }

	/// Returns the start value of the selection interval (y-axis).
	FloatType selectionYAxisRangeStart() const { return _selectionYAxisRangeStart; }

	/// Returns the end value of the selection interval (y-axis).
	FloatType selectionYAxisRangeEnd() const { return _selectionYAxisRangeEnd; }

	/// Set whether the range of the x-axis of the scatter plot should be fixed.
	void setFixXAxisRange(bool fix) { _fixXAxisRange = fix; }

	/// Returns whether the range of the x-axis of the scatter plot should be fixed.
	bool fixXAxisRange() const { return _fixXAxisRange; }

	/// Set start and end value of the x-axis.
	void setXAxisRange(FloatType start, FloatType end) { _xAxisRangeStart = start; _xAxisRangeEnd = end; }

	/// Returns the start value of the x-axis.
	FloatType xAxisRangeStart() const { return _xAxisRangeStart; }

	/// Returns the end value of the x-axis.
	FloatType xAxisRangeEnd() const { return _xAxisRangeEnd; }

	/// Set whether the range of the y-axis of the scatter plot should be fixed.
	void setFixYAxisRange(bool fix) { _fixYAxisRange = fix; }

	/// Returns whether the range of the y-axis of the scatter plot should be fixed.
	bool fixYAxisRange() const { return _fixYAxisRange; }

	/// Set start and end value of the y-axis.
	void setYAxisRange(FloatType start, FloatType end) { _yAxisRangeStart = start; _yAxisRangeEnd = end; }

	/// Returns the start value of the y-axis.
	FloatType yAxisRangeStart() const { return _yAxisRangeStart; }

	/// Returns the end value of the y-axis.
	FloatType yAxisRangeEnd() const { return _yAxisRangeEnd; }

	/// Check if particle id has a color assigned.
	bool hasColor(int i) const { return _colorMap.find(i) != _colorMap.end(); }

	/// Return the map from particle id to color.
	const Color &color(int i) const { return _colorMap.at(i); }

protected:

	/// Modifies the particle object.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipelineObject, ModifierApplication* modApp) override;

private:

	/// The particle type property that is used as source for the x-axis.
	PropertyField<ParticlePropertyReference> _xAxisProperty;

	/// The particle type property that is used as source for the y-axis.
	PropertyField<ParticlePropertyReference> _yAxisProperty;

	/// Controls the whether particles within the specified range should be selected (x-axis).
	PropertyField<bool> _selectXAxisInRange;

	/// Controls the start value of the selection interval (x-axis).
	PropertyField<FloatType> _selectionXAxisRangeStart;

	/// Controls the end value of the selection interval (x-axis).
	PropertyField<FloatType> _selectionXAxisRangeEnd;

	/// Controls the whether particles within the specified range should be selected (y-axis).
	PropertyField<bool> _selectYAxisInRange;

	/// Controls the start value of the selection interval (y-axis).
	PropertyField<FloatType> _selectionYAxisRangeStart;

	/// Controls the end value of the selection interval (y-axis).
	PropertyField<FloatType> _selectionYAxisRangeEnd;

	/// Controls the whether the range of the x-axis of the scatter plot should be fixed.
	PropertyField<bool> _fixXAxisRange;

	/// Controls the start value of the x-axis.
	PropertyField<FloatType> _xAxisRangeStart;

	/// Controls the end value of the x-axis.
	PropertyField<FloatType> _xAxisRangeEnd;

	/// Controls the whether the range of the y-axis of the scatter plot should be fixed.
	PropertyField<bool> _fixYAxisRange;

	/// Controls the start value of the y-axis.
	PropertyField<FloatType> _yAxisRangeStart;

	/// Controls the end value of the y-axis.
	PropertyField<FloatType> _yAxisRangeEnd;

	/// Stores the scatter plot data for each particle type separately.
	QVector<QVector<double>> _xData, _yData;

	/// Map from particle id to color.
	std::map<int, Color> _colorMap;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Scatter plot");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_selectXAxisInRange);
	DECLARE_PROPERTY_FIELD(_selectionXAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_selectionXAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_selectYAxisInRange);
	DECLARE_PROPERTY_FIELD(_selectionYAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_selectionYAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_fixXAxisRange);
	DECLARE_PROPERTY_FIELD(_xAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_xAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_fixYAxisRange);
	DECLARE_PROPERTY_FIELD(_yAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_yAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_xAxisProperty);
	DECLARE_PROPERTY_FIELD(_yAxisProperty);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the ScatterPlotModifier class.
 */
class ScatterPlotModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ScatterPlotModifierEditor() : _rangeUpdate(true) {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Replots the scatter plot computed by the modifier.
	void plotScatterPlot();

	/// Keep x-axis range updated
	void updateXAxisRange(const QCPRange &newRange);

	/// Keep y-axis range updated
	void updateYAxisRange(const QCPRange &newRange);

	/// This is called when the user has clicked the "Save Data" button.
	void onSaveData();

private:

	/// The graph widget to display the scatter plot.
	QCustomPlot* _scatterPlot;

	/// Marks the selection interval in the scatter plot (x-axis).
	QCPItemStraightLine* _selectionXAxisRangeStartMarker;

	/// Marks the selection interval in the scatter plot (x-axis).
	QCPItemStraightLine* _selectionXAxisRangeEndMarker;

	/// Marks the selection interval in the scatter plot (y-axis).
	QCPItemStraightLine* _selectionYAxisRangeStartMarker;

	/// Marks the selection interval in the scatter plot (y-axis).
	QCPItemStraightLine* _selectionYAxisRangeEndMarker;

	/// Update range when plot ranges change?
	bool _rangeUpdate;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_SCATTER_PLOT_MODIFIER_H
