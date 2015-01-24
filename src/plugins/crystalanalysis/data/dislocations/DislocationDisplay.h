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

#ifndef __OVITO_CA_DISLOCATION_DISPLAY_H
#define __OVITO_CA_DISLOCATION_DISPLAY_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/rendering/ParticlePrimitive.h>
#include <core/rendering/ArrowPrimitive.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <plugins/particles/objects/SimulationCellObject.h>
#include <plugins/crystalanalysis/data/dislocations/DislocationNetwork.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

class DislocationDisplay;	// defined below

/**
 * An information record used for dislocation picking in the viewports.
 */
class OVITO_CRYSTALANALYSIS_EXPORT DislocationPickInfo : public ObjectPickInfo
{
public:

	/// Constructor.
	DislocationPickInfo(DislocationDisplay* displayObj, DislocationNetwork* dislocationObj, std::vector<int>&& subobjToSegmentMap) :
		_displayObject(displayObj), _dislocationObj(dislocationObj), _subobjToSegmentMap(std::move(subobjToSegmentMap)) {}

	/// The data object containing the dislocations.
	DislocationNetwork* dislocationObj() const { return _dislocationObj; }

	/// Returns the display object that rendered the dislocations.
	DislocationDisplay* displayObject() const { return _displayObject; }

	/// \brief Given an sub-object ID returned by the Viewport::pick() method, looks up the
	/// corresponding dislocation segment.
	int segmentIndexFromSubObjectID(quint32 subobjID) const {
		if(subobjID < _subobjToSegmentMap.size())
			return _subobjToSegmentMap[subobjID];
		else
			return -1;
	}

private:

	/// The data object containing the dislocations.
	OORef<DislocationNetwork> _dislocationObj;

	/// The display object that rendered the dislocations.
	OORef<DislocationDisplay> _displayObject;

	/// This array is used to map sub-object picking IDs back to dislocation segments.
	std::vector<int> _subobjToSegmentMap;

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief A display object for the dislocation lines.
 */
class OVITO_CRYSTALANALYSIS_EXPORT DislocationDisplay : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE DislocationDisplay(DataSet* dataset);

	/// \brief Lets the display object render a data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Dislocations"); }

	/// \brief Returns the line width used for dislocation rendering.
	FloatType lineWidth() const { return _lineWidth; }

	/// \brief Sets the line width used for dislocation rendering.
	void setLineWidth(FloatType width) { _lineWidth = width; }

	/// \brief Returns the selected shading mode for dislocation lines.
	ArrowPrimitive::ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode for dislocation lines.
	void setShadingMode(ArrowPrimitive::ShadingMode mode) { _shadingMode = mode; }

	/// \brief Renders an overlay marker for a single dislocation segment.
	void renderOverlayMarker(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, int segmentIndex, SceneRenderer* renderer, ObjectNode* contextNode);

public:

	Q_PROPERTY(FloatType lineWidth READ lineWidth WRITE setLineWidth)
	Q_PROPERTY(Ovito::ArrowPrimitive::ShadingMode shadingMode READ shadingMode WRITE setShadingMode)

protected:

	/// Clips a dislocation line at the periodic box boundaries.
	void clipDislocationLine(const QVector<Point3>& line, const SimulationCell& simulationCell, const std::function<void(const Point3&, const Point3&, bool)>& segmentCallback);

protected:

	/// The geometry buffer used to render the dislocation segments.
	std::shared_ptr<ArrowPrimitive> _segmentBuffer;

	/// The geometry buffer used to render the segment corners.
	std::shared_ptr<ParticlePrimitive> _cornerBuffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffers.
	SceneObjectCacheHelper<
		WeakVersionedOORef<DataObject>,		// Source object + revision number
		SimulationCell,						// Simulation cell geometry
		FloatType								// Line width
		> _geometryCacheHelper;

	/// The cached bounding box.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input
	/// that require recalculating the bounding box.
	SceneObjectCacheHelper<
		WeakVersionedOORef<DataObject>,		// Source object + revision number
		SimulationCell,						// Simulation cell geometry
		FloatType								// Line width
		> _boundingBoxCacheHelper;

	/// Controls the rendering width for dislocation lines.
	PropertyField<FloatType> _lineWidth;

	/// Controls the shading mode for dislocation lines.
	PropertyField<ArrowPrimitive::ShadingMode, int> _shadingMode;

	/// The data record used for picking dislocations in the viewports.
	OORef<DislocationPickInfo> _pickInfo;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_lineWidth);
	DECLARE_PROPERTY_FIELD(_shadingMode);
};


/**
 * \brief A properties editor for the DislocationDisplay class.
 */
class OVITO_CRYSTALANALYSIS_EXPORT DislocationDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE DislocationDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

}	// End of namespace
}	// End of namespace
}	// End of namespace

#endif // __OVITO_CA_DISLOCATION_DISPLAY_H
