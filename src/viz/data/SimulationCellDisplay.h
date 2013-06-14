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

/**
 * \file SimulationCellDisplay.h
 * \brief Contains the definition of the Ovito::SimulationCellDisplay class.
 */

#ifndef __OVITO_SIMULATION_CELL_DISPLAY_H
#define __OVITO_SIMULATION_CELL_DISPLAY_H

#include <core/Core.h>
#include <core/scene/display/DisplayObject.h>
#include <core/rendering/LineGeometryBuffer.h>
#include <base/utilities/Color.h>

#include "SimulationCell.h"

namespace Viz {

using namespace Ovito;

/**
 * \brief A display object for SimulationObject.
 */
class SimulationCellDisplay : public DisplayObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE SimulationCellDisplay();

	/// \brief Asks the display object if it can display the given scene object.
	virtual bool canDisplay(SceneObject* obj) override {
		return (dynamic_object_cast<SimulationCell>(obj) != nullptr);
	}

	/// \brief Lets the display object render a scene object.
	virtual void render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

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
	virtual Box3 boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Indicates whether this object should be surrounded by a selection marker in the viewports when it is selected.
	virtual bool showSelectionMarker() override { return false; }

public:

	Q_PROPERTY(FloatType simulationCellLineWidth READ simulationCellLineWidth WRITE setSimulationCellLineWidth)
	Q_PROPERTY(Color simulationCellRenderingColor READ simulationCellRenderingColor WRITE setSimulationCellRenderingColor)
	Q_PROPERTY(bool renderSimulationCell READ renderSimulationCell WRITE setRenderSimulationCell)

protected:

	/// Controls the line width used to render the simulation cell.
	PropertyField<FloatType> _simulationCellLineWidth;

	/// Controls whether the simulation cell is visible.
	PropertyField<bool> _renderSimulationCell;

	/// Controls the rendering color of the simulation cell.
	PropertyField<Color> _simulationCellColor;

	/// The buffered line geometry used to render the simulation cell.
	OORef<LineGeometryBuffer> _lineGeometry;

	/// The input object that served as the basis for the cached display geometry.
	OORef<SimulationCell> _lastObject;

	/// The revision number of the object that served as the basis for the cached display geometry.
	unsigned int _lastObjectRevision;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_renderSimulationCell);
	DECLARE_PROPERTY_FIELD(_simulationCellLineWidth);
	DECLARE_PROPERTY_FIELD(_simulationCellColor);
};

};	// End of namespace

#endif // __OVITO_SIMULATION_CELL_DISPLAY_H
