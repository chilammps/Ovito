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
 * \file TriMeshDisplay.h
 * \brief Contains the definition of the Ovito::TriMeshDisplay class.
 */

#ifndef __OVITO_TRIMESH_DISPLAY_H
#define __OVITO_TRIMESH_DISPLAY_H

#include <core/Core.h>
#include <core/scene/display/DisplayObject.h>
#include <core/rendering/TriMeshGeometryBuffer.h>
#include <core/gui/properties/PropertiesEditor.h>

namespace Ovito {

/**
 * \brief A scene display object for per-particle vectors.
 */
class TriMeshDisplay : public DisplayObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE TriMeshDisplay();

	/// \brief Lets the display object render a scene object.
	virtual void render(TimePoint time, SceneObject* sceneObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, SceneObject* sceneObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Mesh"); }

	/// Returns the display color of the mesh.
	const Color& color() const { return _color; }

	/// Sets the display color of the mesh.
	void setColor(const Color& color) { _color = color; }

public:

	Q_PROPERTY(Ovito::Color color READ color WRITE setColor)

protected:

	/// Controls the display color of the mesh.
	PropertyField<Color, QColor> _color;

	/// The buffered geometry used to render the mesh.
	OORef<TriMeshGeometryBuffer> _buffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		QPointer<SceneObject>, unsigned int,		// Mesh object + revision number
		Color										// Display color
		> _geometryCacheHelper;

	/// The cached bounding box.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input
	/// that require recalculating the bounding box.
	SceneObjectCacheHelper<
		QPointer<SceneObject>, unsigned int			// Mesh object + revision number
		> _boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_color);
};

/**
 * \brief A properties editor for the TriMeshDisplay class.
 */
class TriMeshDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE TriMeshDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_TRIMESH_DISPLAY_H
