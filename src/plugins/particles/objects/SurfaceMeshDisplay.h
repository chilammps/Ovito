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

#ifndef __OVITO_SURFACE_MESH_DISPLAY_H
#define __OVITO_SURFACE_MESH_DISPLAY_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DisplayObject.h>
#include <core/scene/objects/WeakVersionedObjectReference.h>
#include <core/utilities/mesh/TriMesh.h>
#include <core/utilities/mesh/HalfEdgeMesh.h>
#include <core/rendering/MeshPrimitive.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/animation/controller/Controller.h>
#include <plugins/particles/data/SimulationCell.h>

namespace Ovito { namespace Particles {

/**
 * \brief A display object for the SurfaceMesh data object class.
 */
class OVITO_PARTICLES_EXPORT SurfaceMeshDisplay : public DisplayObject
{
public:

	/// \brief Constructor.
	Q_INVOKABLE SurfaceMeshDisplay(DataSet* dataset);

	/// \brief Lets the display object render the data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState) override;

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Surface mesh"); }

	/// Returns the color of the defect surface.
	const Color& surfaceColor() const { return _surfaceColor; }

	/// Sets the color of the defect surface.
	void setSurfaceColor(const Color& color) { _surfaceColor = color; }

	/// Returns the color of the defect surface cap.
	const Color& capColor() const { return _capColor; }

	/// Sets the color of the defect surface cap.
	void setCapColor(const Color& color) { _capColor = color; }

	/// Returns whether the cap mesh is rendered.
	bool showCap() const { return _showCap; }

	/// Sets whether the cap mesh is rendered.
	void setShowCap(bool show) { _showCap = show; }

	/// Returns whether the surface mesh is rendered using smooth shading.
	bool smoothShading() const { return _smoothShading; }

	/// Sets whether the surface mesh is rendered using smooth shading.
	void setSmoothShading(bool smoothShading) { _smoothShading = smoothShading; }

	/// Returns the transparency of the surface mesh.
	FloatType surfaceTransparency() const { return _surfaceTransparency ? _surfaceTransparency->currentFloatValue() : 0.0f; }

	/// Sets the transparency of the surface mesh.
	void setSurfaceTransparency(FloatType transparency) { if(_surfaceTransparency) _surfaceTransparency->setCurrentFloatValue(transparency); }

	/// Returns the transparency of the surface cap mesh.
	FloatType capTransparency() const { return _capTransparency ? _capTransparency->currentFloatValue() : 0.0f; }

	/// Sets the transparency of the surface cap mesh.
	void setCapTransparency(FloatType transparency) { if(_capTransparency) _capTransparency->setCurrentFloatValue(transparency); }

	/// Generates the final triangle mesh, which will be rendered.
	static bool buildSurfaceMesh(const HalfEdgeMesh& input, const SimulationCell& cell, TriMesh& output);

	/// Generates the triangle mesh for the PBC cap.
	static void buildCapMesh(const HalfEdgeMesh& input, const SimulationCell& cell, bool isCompletelySolid, TriMesh& output);

protected:

	/// Splits a triangle face at a periodic boundary.
	static bool splitFace(TriMesh& output, TriMeshFace& face, int oldVertexCount, std::vector<Point3>& newVertices, std::map<std::pair<int,int>,std::pair<int,int>>& newVertexLookupMap, const SimulationCell& cell, size_t dim);

	/// Traces the closed contour of the surface-boundary intersection.
	static std::vector<Point2> traceContour(HalfEdgeMesh::Edge* firstEdge, const std::vector<Point3>& reducedPos, const SimulationCell& cell, size_t dim);

	/// Clips a 2d contour at a periodic boundary.
	static void clipContour(std::vector<Point2>& input, std::array<bool,2> periodic, std::vector<std::vector<Point2>>& openContours, std::vector<std::vector<Point2>>& closedContours);

	/// Computes the intersection point of a 2d contour segment crossing a periodic boundary.
	static void computeContourIntersection(size_t dim, FloatType t, Point2& base, Vector2& delta, int crossDir, std::vector<std::vector<Point2>>& contours);

	/// Determines if the 2D box corner (0,0) is inside the closed region described by the 2d polygon.
	static bool isCornerInside2DRegion(const std::vector<std::vector<Point2>>& contours);

	/// Determines if the 3D box corner (0,0,0) is inside the region described by the half-edge polyhedron.
	static bool isCornerInside3DRegion(const HalfEdgeMesh& mesh, const std::vector<Point3>& reducedPos, const std::array<bool,3> pbcFlags, bool isCompletelySolid);

	/// Controls the display color of the surface mesh.
	PropertyField<Color, QColor> _surfaceColor;

	/// Controls the display color of the cap mesh.
	PropertyField<Color, QColor> _capColor;

	/// Controls whether the cap mesh is rendered.
	PropertyField<bool> _showCap;

	/// Controls whether the surface mesh is rendered using smooth shading.
	PropertyField<bool> _smoothShading;

	/// Controls the transparency of the surface mesh.
	ReferenceField<Controller> _surfaceTransparency;

	/// Controls the transparency of the surface cap mesh.
	ReferenceField<Controller> _capTransparency;

	/// The buffered geometry used to render the surface mesh.
	std::shared_ptr<MeshPrimitive> _surfaceBuffer;

	/// The buffered geometry used to render the surface cap.
	std::shared_ptr<MeshPrimitive> _capBuffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		WeakVersionedOORef<DataObject>,		// Source object + revision number
		SimulationCell,						// Simulation cell geometry
		ColorA,									// Surface color
		ColorA,									// Cap color
		bool									// Smooth shading
		> _geometryCacheHelper;

	/// The cached bounding box.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input
	/// that require recalculating the bounding box.
	SceneObjectCacheHelper<
		WeakVersionedOORef<DataObject>,		// Source object + revision number
		SimulationCell						// Simulation cell geometry
		> _boundingBoxCacheHelper;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_surfaceColor);
	DECLARE_PROPERTY_FIELD(_capColor);
	DECLARE_PROPERTY_FIELD(_showCap);
	DECLARE_PROPERTY_FIELD(_smoothShading);
	DECLARE_REFERENCE_FIELD(_surfaceTransparency);
	DECLARE_REFERENCE_FIELD(_capTransparency);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the SurfaceMeshDisplay class.
 */
class OVITO_PARTICLES_EXPORT SurfaceMeshDisplayEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE SurfaceMeshDisplayEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

}	// End of namespace
}	// End of namespace

#endif // __OVITO_SURFACE_MESH_DISPLAY_H
