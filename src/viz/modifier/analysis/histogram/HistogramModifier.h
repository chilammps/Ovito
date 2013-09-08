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

public:

	Q_PROPERTY(Viz::ParticlePropertyReference sourceProperty READ sourceProperty WRITE setSourceProperty)
	Q_PROPERTY(int numberOfBins READ numberOfBins WRITE setNumberOfBins)

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

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Histogram");
	Q_CLASSINFO("ModifierCategory", "Analysis");

	DECLARE_PROPERTY_FIELD(_numberOfBins);
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

	/// Stores the current number of bins in the application settings
	/// so it can be used as default value for new modifiers in the future.
	void memorizeNumberOfBins();

	/// Replots the histogram computed by the modifier.
	void plotHistogram();

	/// Updates the contents of the property list combo box.
	void updatePropertyList();

	/// This is called when the user has selected another item in the particle property list.
	void onPropertySelected(int index);

private:

	/// The list of particle properties.
	ParticlePropertyComboBox* _propertyListBox;

	/// The graph widget to display the histogram.
	QCustomPlot* _histogramPlot;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_HISTOGRAM_MODIFIER_H
