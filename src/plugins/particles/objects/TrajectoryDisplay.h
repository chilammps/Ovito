///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_TRAJECTORY_DISPLAY_H
#define __OVITO_TRAJECTORY_DISPLAY_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/rendering/SceneRenderer.h>
#include <core/gui/properties/PropertiesEditor.h>
#include "TrajectoryObject.h"

namespace Ovito { namespace Particles {

/**
 * \brief A display object for particle trajectories.
 */
class OVITO_PARTICLES_EXPORT TrajectoryDisplay : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE TrajectoryDisplay(DataSet* dataset);

	/// \brief Renders the associated data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the display bounding box of the data object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Trajectory lines"); }

	/// \brief Returns the display width of trajectory lines.
	FloatType lineWidth() const { return _lineWidth; }

	/// \brief Sets the display width of trajectory lines.
	void setLineWidth(FloatType newWidth) { _lineWidth = newWidth; }

	/// \brief Returns the selected shading mode for trajectory lines.
	ArrowPrimitive::ShadingMode shadingMode() const { return _shadingMode; }

	/// \brief Sets the shading mode for trajectory lines.
	void setShadingMode(ArrowPrimitive::ShadingMode mode) { _shadingMode = mode; }

	/// Returns the display color for trajectory lines.
	const Color& lineColor() const { return _lineColor; }

	/// Sets the display color for trajectory lines.
	void setLineColor(const Color& color) { _lineColor = color; }

public:

    Q_PROPERTY(Ovito::ArrowPrimitive::ShadingMode shadingMode READ shadingMode WRITE setShadingMode);

protected:

	/// Controls the display width of trajectory lines.
	PropertyField<FloatType> _lineWidth;

	/// Controls the color of the trajectory lines.
	PropertyField<Color, QColor> _lineColor;

	/// Controls the shading mode for lines.
	PropertyField<ArrowPrimitive::ShadingMode, int> _shadingMode;

	/// The buffered geometry used to render the trajectory lines.
	std::shared_ptr<ArrowPrimitive> _buffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffers.
	SceneObjectCacheHelper<
		WeakVersionedOORef<TrajectoryObject>,			// The trajectory data object + revision number
		FloatType,										// Line width
		Color											// Line color
	> _geometryCacheHelper;

	/// The bounding box that includes all trajectories.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input data
	/// that require recomputing the bounding box.
	SceneObjectCacheHelper<
		WeakVersionedOORef<TrajectoryObject>,			// The data object + revision number
		FloatType										// Line width
	> _boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_lineWidth);
	DECLARE_PROPERTY_FIELD(_lineColor);
	DECLARE_PROPERTY_FIELD(_shadingMode);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the TrajectoryDisplay class.
 */
class TrajectoryDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE TrajectoryDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_TRAJECTORY_DISPLAY_H
