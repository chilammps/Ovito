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

#ifndef __OVITO_SIMULATION_CELL_DISPLAY_H
#define __OVITO_SIMULATION_CELL_DISPLAY_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/rendering/LinePrimitive.h>
#include <core/rendering/ArrowPrimitive.h>
#include <core/rendering/ParticlePrimitive.h>
#include <core/gui/properties/PropertiesEditor.h>

#include "SimulationCellObject.h"

namespace Ovito { namespace Particles {

/**
 * \brief A display object for SimulationCellObject.
 */
class OVITO_PARTICLES_EXPORT SimulationCellDisplay : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE SimulationCellDisplay(DataSet* dataset);

	/// \brief Lets the display object render the data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Simulation cell"); }

	/// \brief Returns the line width used to render the simulation cell box.
	/// \return The line with in world units or zero if the simulation box is not rendered.
	FloatType simulationCellLineWidth() const { return _simulationCellLineWidth; }

	/// \brief Sets the line width used to render the simulation cell box.
	/// \param newWidth The new line width in world units or zero to not render the box at all.
	/// \undoable
	void setSimulationCellLineWidth(FloatType newWidth) { _simulationCellLineWidth = newWidth; }

	/// \brief Returns whether the simulation cell is visible.
	/// \return The visibility flag.
	bool renderSimulationCell() const { return _renderSimulationCell; }

	/// \brief Sets whether the simulation cell is visible.
	/// \param on The visibility flag.
	/// \undoable
	void setRenderSimulationCell(bool on) { _renderSimulationCell = on; }

	/// \brief Returns the color used for rendering the simulation cell.
	/// \return The line color
	Color simulationCellRenderingColor() const { return _simulationCellColor; }

	/// \brief Sets the color to be used for rendering the simulation cell.
	/// \param Color The new line color.
	/// \undoable
	void setSimulationCellRenderingColor(const Color& color) { _simulationCellColor = color; }

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Indicates whether this object should be surrounded by a selection marker in the viewports when it is selected.
	virtual bool showSelectionMarker() override { return false; }

protected:

	/// Renders the given simulation using wireframe mode.
	void renderWireframe(SimulationCellObject* cell, SceneRenderer* renderer, ObjectNode* contextNode);

	/// Renders the given simulation using solid shading mode.
	void renderSolid(SimulationCellObject* cell, SceneRenderer* renderer, ObjectNode* contextNode);

protected:

	/// Controls the line width used to render the simulation cell.
	PropertyField<FloatType> _simulationCellLineWidth;

	/// Controls whether the simulation cell is visible.
	PropertyField<bool> _renderSimulationCell;

	/// Controls the rendering color of the simulation cell.
	PropertyField<Color, QColor> _simulationCellColor;

	/// The geometry buffer used to render the simulation cell in wireframe mode.
	std::shared_ptr<LinePrimitive> _wireframeGeometry;

	/// The geometry buffer used to render the wireframe simulation cell in object picking mode.
	std::shared_ptr<LinePrimitive> _wireframePickingGeometry;

	/// This helper structure is used to detect any changes in the input simulation cell
	/// that require updating the display geometry buffer for wireframe rendering.
	SceneObjectCacheHelper<
		WeakVersionedOORef<SimulationCellObject>,
		ColorA
		> _wireframeGeometryCacheHelper;

	/// The geometry buffer used to render the edges of the cell.
	std::shared_ptr<ArrowPrimitive> _edgeGeometry;

	/// The geometry buffer used to render the corners of the cell.
	std::shared_ptr<ParticlePrimitive> _cornerGeometry;

	/// This helper structure is used to detect any changes in the input simulation cell
	/// that require updating the display geometry buffer for solid rendering mode.
	SceneObjectCacheHelper<
		WeakVersionedOORef<SimulationCellObject>,			// The simulation cell + revision number
		FloatType, Color							// Line width + color
		> _solidGeometryCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_renderSimulationCell);
	DECLARE_PROPERTY_FIELD(_simulationCellLineWidth);
	DECLARE_PROPERTY_FIELD(_simulationCellColor);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the SimulationCellDisplay class.
 */
class SimulationCellDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE SimulationCellDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace

#endif // __OVITO_SIMULATION_CELL_DISPLAY_H
