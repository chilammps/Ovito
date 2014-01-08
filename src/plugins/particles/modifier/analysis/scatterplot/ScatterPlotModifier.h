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

#ifndef __OVITO_SCATTER_PLOT_MODIFIER_H
#define __OVITO_SCATTER_PLOT_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/data/ParticleProperty.h>
#include <plugins/particles/util/ParticlePropertyComboBox.h>
#include "../../ParticleModifier.h"

class QCustomPlot;
class QCPItemStraightLine;

namespace Particles {

/*
 * This modifier computes a scatter plot for two particle properties.
 */
class OVITO_PARTICLES_EXPORT ScatterPlotModifier : public ParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE ScatterPlotModifier(DataSet* dataset);

	/// This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipelineObject, ModifierApplication* modApp) override;

	/// Sets the source particle property for which the scatter plot should be computed.
	void setXAxisProperty(const ParticlePropertyReference& prop) { _xAxisProperty = prop; }

	/// Returns the source particle property for which the scatter plot is computed.
	const ParticlePropertyReference& xAxisProperty() const { return _xAxisProperty; }

	/// Sets the source particle property for which the scatter plot should be computed.
	void setYAxisProperty(const ParticlePropertyReference& prop) { _yAxisProperty = prop; }

	/// Returns the source particle property for which the scatter plot is computed.
	const ParticlePropertyReference& yAxisProperty() const { return _yAxisProperty; }

	/// Retrieves the selected input particle property from the given modifier input state.
	ParticlePropertyObject* lookupInputProperty(const PipelineFlowState& inputState, const ParticlePropertyReference &refprop) const;

	/// Returns the stored scatter plot data (x-axis).
	const QVector<double>& xData() const { return _xData; }

	/// Returns the stored scatter plot data (y-axis).
	const QVector<double>& yData() const { return _yData; }

	/// Returns whether particles within the specified range should be selected.
	bool selectInRange() const { return _selectInRange; }

	/// Sets whether particles within the specified range should be selected.
	void setSelectInRange(bool select) { _selectInRange = select; }

	/// Returns the start value of the selection interval.
	FloatType selectionRangeStart() const { return _selectionRangeStart; }

	/// Returns the end value of the selection interval.
	FloatType selectionRangeEnd() const { return _selectionRangeEnd; }

	/// Returns whether the range of the x-axis of the scatter plot should be fixed.
	bool fixXAxisRange() const { return _fixXAxisRange; }

	/// Returns the start value of the x-axis.
	FloatType xAxisRangeStart() const { return _xAxisRangeStart; }

	/// Returns the end value of the x-axis.
	FloatType xAxisRangeEnd() const { return _xAxisRangeEnd; }

	/// Returns whether the range of the y-axis of the scatter plot should be fixed.
	bool fixYAxisRange() const { return _fixYAxisRange; }

	/// Returns the start value of the y-axis.
	FloatType yAxisRangeStart() const { return _yAxisRangeStart; }

	/// Returns the end value of the y-axis.
	FloatType yAxisRangeEnd() const { return _yAxisRangeEnd; }

public:

	Q_PROPERTY(Particles::ParticlePropertyReference xAxisProperty READ xAxisProperty WRITE setXAxisProperty);
	Q_PROPERTY(Particles::ParticlePropertyReference yAxisProperty READ yAxisProperty WRITE setYAxisProperty);
	Q_PROPERTY(bool selectInRange READ selectInRange WRITE setSelectInRange);
	Q_PROPERTY(FloatType selectionRangeStart READ selectionRangeStart);
	Q_PROPERTY(FloatType selectionRangeEnd READ selectionRangeEnd);

protected:

	/// Modifies the particle object.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// The particle type property that is used as source for the x-axis.
	PropertyField<ParticlePropertyReference> _xAxisProperty;

	/// The particle type property that is used as source for the y-axis.
	PropertyField<ParticlePropertyReference> _yAxisProperty;

	/// Controls the whether particles within the specified range should be selected.
	PropertyField<bool> _selectInRange;

	/// Controls the start value of the selection interval.
	PropertyField<FloatType> _selectionRangeStart;

	/// Controls the end value of the selection interval.
	PropertyField<FloatType> _selectionRangeEnd;

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

	/// Stores the scatter plot data.
	QVector<double> _xData, _yData;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Scatter plot");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_selectInRange);
	DECLARE_PROPERTY_FIELD(_selectionRangeStart);
	DECLARE_PROPERTY_FIELD(_selectionRangeEnd);
	DECLARE_PROPERTY_FIELD(_fixXAxisRange);
	DECLARE_PROPERTY_FIELD(_xAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_xAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_fixYAxisRange);
	DECLARE_PROPERTY_FIELD(_yAxisRangeStart);
	DECLARE_PROPERTY_FIELD(_yAxisRangeEnd);
	DECLARE_PROPERTY_FIELD(_xAxisProperty);
	DECLARE_PROPERTY_FIELD(_yAxisProperty);
};

/******************************************************************************
* A properties editor for the ScatterPlotModifier class.
******************************************************************************/
class ScatterPlotModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ScatterPlotModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Replots the scatter plot computed by the modifier.
	void plotScatterPlot();

	/// This is called when the user has clicked the "Save Data" button.
	void onSaveData();

private:

	/// The list of particle properties (x-axis).
	ParticlePropertyComboBox* _xPropertyListBox;

	/// The list of particle properties (y-axis).
	ParticlePropertyComboBox* _yPropertyListBox;

	/// The graph widget to display the scatter plot.
	QCustomPlot* _scatterPlot;

	/// Marks the selection interval in the scatter plot.
	QCPItemStraightLine* _selectionRangeStartMarker;

	/// Marks the selection interval in the scatter plot.
	QCPItemStraightLine* _selectionRangeEndMarker;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_SCATTER_PLOT_MODIFIER_H
