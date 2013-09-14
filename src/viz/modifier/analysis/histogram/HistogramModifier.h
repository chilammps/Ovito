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

#ifndef __OVITO_HISTOGRAM_MODIFIER_H
#define __OVITO_HISTOGRAM_MODIFIER_H

#include <core/Core.h>
#include <viz/data/ParticleProperty.h>
#include <viz/util/ParticlePropertyComboBox.h>
#include "../../ParticleModifier.h"

class QCustomPlot;
class QCPItemStraightLine;

namespace Viz {

/*
 * This modifier computes a value histogram for a particle property.
 */
class HistogramModifier : public ParticleModifier
{
public:

	/// Default constructor.
	Q_INVOKABLE HistogramModifier();

	/// This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipelineObject, ModifierApplication* modApp) override;

	/// Sets the source particle property for which the histogram should be computed.
	void setSourceProperty(const ParticlePropertyReference& prop);

	/// Returns the source particle property for which the histogram is computed.
	const ParticlePropertyReference& sourceProperty() const { return _sourcePropertyRef; }

	/// Retrieves the selected input particle property from the given modifier input state.
	ParticlePropertyObject* lookupInputProperty(const PipelineFlowState& inputState) const;

	/// Returns the number of bins in the computed histogram.
	int numberOfBins() const { return _numberOfBins; }

	/// Sets the number of bins in the computed histogram.
	void setNumberOfBins(int n) { _numberOfBins = n; }

	/// Returns the stored histogram data.
	const std::vector<int>& histogramData() const { return _histogramData; }

	/// Returns the start value of the histogram interval.
	FloatType intervalStart() const { return _intervalStart; }

	/// Returns the end value of the histogram interval.
	FloatType intervalEnd() const { return _intervalEnd; }

	/// Returns the whether particles within the specified range should be selected.
	bool selectInRange() const { return _selectInRange; }

	/// Returns the start value of the selection interval.
	FloatType selectionRangeStart() const { return _selectionRangeStart; }

	/// Returns the end value of the selection interval.
	FloatType selectionRangeEnd() const { return _selectionRangeEnd; }

public:

	Q_PROPERTY(Viz::ParticlePropertyReference sourceProperty READ sourceProperty WRITE setSourceProperty)
	Q_PROPERTY(int numberOfBins READ numberOfBins WRITE setNumberOfBins)
	Q_PROPERTY(FloatType selectionRangeStart READ selectionRangeStart)
	Q_PROPERTY(FloatType selectionRangeEnd READ selectionRangeEnd)

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// Modifies the particle object.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

private:

	/// The particle type property that is used as source for the histogram.
	ParticlePropertyReference _sourcePropertyRef;

	/// Controls the number of histogram bins.
	PropertyField<int> _numberOfBins;

	/// Controls the whether particles within the specified range should be selected.
	PropertyField<bool> _selectInRange;

	/// Controls the start value of the selection interval.
	PropertyField<FloatType> _selectionRangeStart;

	/// Controls the end value of the selection interval.
	PropertyField<FloatType> _selectionRangeEnd;

	/// Stores the histogram data.
	std::vector<int> _histogramData;

	/// The start value of the histogram interval.
	FloatType _intervalStart;

	/// The end value of the histogram interval.
	FloatType _intervalEnd;

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Histogram");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_numberOfBins);
	DECLARE_PROPERTY_FIELD(_selectInRange);
	DECLARE_PROPERTY_FIELD(_selectionRangeStart);
	DECLARE_PROPERTY_FIELD(_selectionRangeEnd);
};

/******************************************************************************
* A properties editor for the HistogramModifier class.
******************************************************************************/
class HistogramModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE HistogramModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

protected Q_SLOTS:

	/// Replots the histogram computed by the modifier.
	void plotHistogram();

	/// Updates the contents of the property list combo box.
	void updatePropertyList();

	/// This is called when the user has selected another item in the particle property list.
	void onPropertySelected(int index);

	/// This is called when the user has clicked the "Save Data" button.
	void onSaveData();

private:

	/// The list of particle properties.
	ParticlePropertyComboBox* _propertyListBox;

	/// The graph widget to display the histogram.
	QCustomPlot* _histogramPlot;

	/// Marks the selection interval in the histogram plot.
	QCPItemStraightLine* _selectionRangeStartMarker;

	/// Marks the selection interval in the histogram plot.
	QCPItemStraightLine* _selectionRangeEndMarker;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_HISTOGRAM_MODIFIER_H
