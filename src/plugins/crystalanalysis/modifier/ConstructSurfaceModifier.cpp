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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <plugins/particles/data/SurfaceMesh.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <plugins/particles/data/SimulationCell.h>
#include <plugins/crystalanalysis/util/DelaunayTessellation.h>
#include "ConstructSurfaceModifier.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, ConstructSurfaceModifier, AsynchronousParticleModifier)
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, ConstructSurfaceModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(ConstructSurfaceModifier, ConstructSurfaceModifierEditor)
DEFINE_FLAGS_PROPERTY_FIELD(ConstructSurfaceModifier, _smoothingLevel, "SmoothingLevel", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_PROPERTY_FIELD(ConstructSurfaceModifier, _radius, "Radius", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_REFERENCE_FIELD(ConstructSurfaceModifier, _surfaceMeshObj, "SurfaceMesh", SurfaceMesh, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(ConstructSurfaceModifier, _surfaceMeshDisplay, "SurfaceMeshDisplay", SurfaceMeshDisplay, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_PROPERTY_FIELD(ConstructSurfaceModifier, _onlySelectedParticles, "OnlySelectedParticles")
SET_PROPERTY_FIELD_LABEL(ConstructSurfaceModifier, _smoothingLevel, "Smoothing level")
SET_PROPERTY_FIELD_LABEL(ConstructSurfaceModifier, _radius, "Probe sphere radius")
SET_PROPERTY_FIELD_LABEL(ConstructSurfaceModifier, _surfaceMeshObj, "Surface mesh")
SET_PROPERTY_FIELD_LABEL(ConstructSurfaceModifier, _surfaceMeshDisplay, "Surface mesh display")
SET_PROPERTY_FIELD_LABEL(ConstructSurfaceModifier, _onlySelectedParticles, "Use only selected particles")
SET_PROPERTY_FIELD_UNITS(ConstructSurfaceModifier, _radius, WorldParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ConstructSurfaceModifier::ConstructSurfaceModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_smoothingLevel(8), _radius(4), _onlySelectedParticles(false),
	_solidVolume(0), _totalVolume(0), _surfaceArea(0)
{
	INIT_PROPERTY_FIELD(ConstructSurfaceModifier::_smoothingLevel);
	INIT_PROPERTY_FIELD(ConstructSurfaceModifier::_radius);
	INIT_PROPERTY_FIELD(ConstructSurfaceModifier::_surfaceMeshObj);
	INIT_PROPERTY_FIELD(ConstructSurfaceModifier::_surfaceMeshDisplay);
	INIT_PROPERTY_FIELD(ConstructSurfaceModifier::_onlySelectedParticles);

	// Create the output object.
	_surfaceMeshObj = new SurfaceMesh(dataset);
	_surfaceMeshObj->setSaveWithScene(storeResultsWithScene());
	_surfaceMeshDisplay = static_object_cast<SurfaceMeshDisplay>(_surfaceMeshObj->displayObjects().front());
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void ConstructSurfaceModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute results when the parameters have changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(ConstructSurfaceModifier::_smoothingLevel)
				|| field == PROPERTY_FIELD(ConstructSurfaceModifier::_radius)
				|| field == PROPERTY_FIELD(ConstructSurfaceModifier::_onlySelectedParticles))
			invalidateCachedResults();
	}

	// Adopt "Save with scene" flag.
	if(field == PROPERTY_FIELD(AsynchronousParticleModifier::_saveResults)) {
		if(surfaceMesh())
			surfaceMesh()->setSaveWithScene(storeResultsWithScene());
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Handles reference events sent by reference targets of this object.
******************************************************************************/
bool ConstructSurfaceModifier::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Do not propagate messages from the attached output and display objects.
	if(source == surfaceMesh() || source == surfaceMeshDisplay())
		return false;

	return AsynchronousParticleModifier::referenceEvent(source, event);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> ConstructSurfaceModifier::createEngine(TimePoint time)
{
	// Get modifier inputs.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	ParticlePropertyObject* selProperty = _onlySelectedParticles ? inputStandardProperty(ParticleProperty::SelectionProperty) : nullptr;
	SimulationCell* simCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<ConstructSurfaceEngine>(posProperty->storage(), selProperty ? selProperty->storage() : nullptr,
			simCell->data(), radius(), smoothingLevel());
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void ConstructSurfaceModifier::retrieveModifierResults(Engine* engine)
{
	ConstructSurfaceEngine* eng = static_cast<ConstructSurfaceEngine*>(engine);
	if(surfaceMesh()) {
		surfaceMesh()->mesh().swap(eng->mesh());
		surfaceMesh()->notifyDependents(ReferenceEvent::TargetChanged);
	}
	_solidVolume = eng->solidVolume();
	_totalVolume = eng->totalVolume();
	_surfaceArea = eng->surfaceArea();
}

/******************************************************************************
* This lets the modifier insert the previously computed results into the pipeline.
******************************************************************************/
ObjectStatus ConstructSurfaceModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	// Insert output object into pipeline.
	if(surfaceMesh()) {
		output().addObject(surfaceMesh());
	}
	return ObjectStatus(ObjectStatus::Success, tr("Surface area: %1\nSolid volume: %2\nTotal volume: %3\nSolid volume fraction: %4\nSurface area per solid volume: %5\nSurface area per total volume: %6")
			.arg(surfaceArea()).arg(solidVolume()).arg(totalVolume())
			.arg(solidVolume() / totalVolume()).arg(surfaceArea() / solidVolume()).arg(surfaceArea() / totalVolume()));
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void ConstructSurfaceModifier::ConstructSurfaceEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Constructing surface mesh"));
	double alpha = _radius * _radius;

	// Generate the list of input vertices.
	const Point3* inputPositions = positions()->constDataPoint3();
	std::vector<Point3> selectedParticles;
	size_t inputCount = positions()->size();
	if(selection()) {
		const int* sel = selection()->constDataInt();
		for(const Point3& p : positions()->constPoint3Range()) {
			if(*sel++)
				selectedParticles.push_back(p);
		}
		inputPositions = selectedParticles.data();
		inputCount = selectedParticles.size();
	}

	// Generate Delaunay tessellation.
	futureInterface.setProgressText(tr("Constructing surface mesh (Delaunay tessellation step)"));
	DelaunayTessellation tessellation;
	tessellation.generateTessellation(_simCell, inputPositions, inputCount, std::abs(_radius) * 1.25f * sqrt(6.0f));
	if(futureInterface.isCanceled())
		return;

	futureInterface.setProgressRange(tessellation.number_of_tetrahedra());
	futureInterface.setProgressValue(0);
	futureInterface.setProgressText(tr("Constructing surface mesh (cell classification step)"));

	// Classify cells into solid and open tetrahedra.
	int nghost = 0, ntotal = 0;
	int solidCellCount = 0;
	_solidVolume = 0;
	for(DelaunayTessellation::CellIterator cell = tessellation.begin_cells(); cell != tessellation.end_cells(); ++cell) {
		// This determines whether a Delaunay tetrahedron is part of the solid region.
		bool isSolid = tessellation.isValidCell(cell) &&
				tessellation.dt().geom_traits().compare_squared_radius_3_object()(
						cell->vertex(0)->point(),
						cell->vertex(1)->point(),
						cell->vertex(2)->point(),
						cell->vertex(3)->point(),
						alpha) != CGAL::POSITIVE;

		cell->info().flag = isSolid;
		if(isSolid && !cell->info().isGhost) {
			cell->info().index = solidCellCount++;
		}
		else
			cell->info().index = -1;

		futureInterface.incrementProgressValue(0);
	}
	if(futureInterface.isCanceled())
		return;

	// Stores pointers to the mesh facets generated for a solid, local
	// tetrahedron of the Delaunay tessellation.
	struct Tetrahedron {
		/// Pointers to the mesh facets associated with the four faces of the tetrahedron.
		std::array<HalfEdgeMesh::Face*, 4> meshFacets;
		std::array<int,4> vertexIndices;
		int minVertexIndex, maxVertexIndex;
	};
	std::vector<Tetrahedron> tetrahedra(solidCellCount);

	futureInterface.setProgressRange(solidCellCount);
	futureInterface.setProgressText(tr("Constructing surface mesh (facet construction step)"));

	// Create the triangular mesh facets separating solid and open tetrahedra.
	std::vector<HalfEdgeMesh::Vertex*> vertexMap(inputCount, nullptr);
	for(DelaunayTessellation::CellIterator cell = tessellation.begin_cells(); cell != tessellation.end_cells(); ++cell) {

		// Start with the solid and local tetrahedra.
		if(cell->info().index == -1)
			continue;
		OVITO_ASSERT(cell->info().flag);
		futureInterface.setProgressValue(cell->info().index);

		Tetrahedron& tet = tetrahedra[cell->info().index];
		Point3 unwrappedVerts[4];
		for(size_t i = 0; i < 4; i++) {
			tet.vertexIndices[i] = cell->vertex(i)->point().index();
			unwrappedVerts[i] = cell->vertex(i)->point();
		}
		tet.minVertexIndex = *std::min_element(tet.vertexIndices.begin(), tet.vertexIndices.end());
		tet.maxVertexIndex = *std::max_element(tet.vertexIndices.begin(), tet.vertexIndices.end());

		// Compute cell volume.
		Vector3 ad = unwrappedVerts[0] - unwrappedVerts[3];
		Vector3 bd = unwrappedVerts[1] - unwrappedVerts[3];
		Vector3 cd = unwrappedVerts[2] - unwrappedVerts[3];
		if(_simCell.isWrappedVector(ad) || _simCell.isWrappedVector(bd) || _simCell.isWrappedVector(cd))
			throw Exception(tr("Cannot construct surface mesh. Simulation cell length is too small for the given radius parameter."));
		_solidVolume += std::abs(ad.dot(cd.cross(bd))) / 6.0f;

		// Iterate over the four faces of the tetrahedron cell.
		for(int f = 0; f < 4; f++) {
			tet.meshFacets[f] = nullptr;

			// Test if the adjacent tetrahedron belongs to the open region.
			DelaunayTessellation::CellHandle adjacentCell = tessellation.mirrorCell(cell, f);
			if(adjacentCell->info().flag)
				continue;

			// Create the three vertices of the face or use existing output vertices.
			std::array<HalfEdgeMesh::Vertex*,3> facetVertices;
			for(int v = 0; v < 3; v++) {
				DelaunayTessellation::VertexHandle vertex = cell->vertex(DelaunayTessellation::cellFacetVertexIndex(f, v));
				int vertexIndex = vertex->point().index();
				OVITO_ASSERT(vertexIndex >= 0 && vertexIndex < vertexMap.size());
				if(vertexMap[vertexIndex] == nullptr)
					vertexMap[vertexIndex] = facetVertices[2-v] = _mesh.createVertex(inputPositions[vertexIndex]);
				else
					facetVertices[2-v] = vertexMap[vertexIndex];
			}

			// Create a new triangle facet.
			tet.meshFacets[f] = _mesh.createFace(facetVertices.begin(), facetVertices.end());
		}
	}
	if(futureInterface.isCanceled())
		return;

	// Links half-edges to opposite half-edges.
	futureInterface.setProgressText(tr("Constructing surface mesh (facet connection step)"));
	for(DelaunayTessellation::CellIterator cell = tessellation.begin_cells(); cell != tessellation.end_cells(); ++cell) {
		if(cell->info().index == -1)
			continue;
		OVITO_ASSERT(cell->info().flag);

		futureInterface.setProgressValue(cell->info().index);
		if(futureInterface.isCanceled())
			return;

		Tetrahedron& tet = tetrahedra[cell->info().index];
		for(int f = 0; f < 4; f++) {
			HalfEdgeMesh::Face* facet = tet.meshFacets[f];
			if(facet == nullptr) continue;

			HalfEdgeMesh::Edge* edge = facet->edges();
			for(int e = 0; e < 3; e++, edge = edge->nextFaceEdge()) {
				OVITO_CHECK_POINTER(edge);
				if(edge->oppositeEdge() != nullptr) continue;
				int vertexIndex1 = DelaunayTessellation::cellFacetVertexIndex(f, 2-e);
				int vertexIndex2 = DelaunayTessellation::cellFacetVertexIndex(f, (4-e)%3);
				DelaunayTessellation::FacetCirculator circulator_start = tessellation.incident_facets(cell, vertexIndex1, vertexIndex2, cell, f);
				DelaunayTessellation::FacetCirculator circulator = circulator_start;
				OVITO_ASSERT(circulator->first == cell);
				OVITO_ASSERT(circulator->second == f);
				--circulator;
				OVITO_ASSERT(circulator != circulator_start);
				do {
					// Look for the first open cell while going around the edge.
					if(circulator->first->info().flag == false)
						break;
					--circulator;
				}
				while(circulator != circulator_start);
				OVITO_ASSERT(circulator != circulator_start);

				// Get the adjacent cell, which must be solid.
				std::pair<DelaunayTessellation::CellHandle,int> mirrorFacet = tessellation.mirrorFacet(circulator);
				OVITO_ASSERT(mirrorFacet.first->info().flag == true);
				HalfEdgeMesh::Face* oppositeFace;
				// If the cell is a ghost cell, find the corresponding real cell.
				if(mirrorFacet.first->info().isGhost) {
					OVITO_ASSERT(mirrorFacet.first->info().index == -1);
					std::array<int,3> faceVerts;
					for(size_t i = 0; i < 3; i++) {
						faceVerts[i] = mirrorFacet.first->vertex(DelaunayTessellation::cellFacetVertexIndex(mirrorFacet.second, i))->point().index();
						OVITO_ASSERT(faceVerts[i] != -1);
					}
					std::sort(faceVerts.begin(), faceVerts.end());
					for(const Tetrahedron& realTet : tetrahedra) {
						if(realTet.minVertexIndex != faceVerts.front() && realTet.maxVertexIndex != faceVerts.back()) continue;
						bool found = false;
						for(int fi = 0; fi < 4; fi++) {
							if(realTet.meshFacets[fi] == nullptr) continue;
							std::array<int,3> faceVerts2;
							for(size_t i = 0; i < 3; i++) {
								faceVerts2[i] = realTet.vertexIndices[DelaunayTessellation::cellFacetVertexIndex(fi, i)];
								OVITO_ASSERT(faceVerts2[i] != -1);
							}
							if(std::is_permutation(faceVerts.begin(), faceVerts.end(), faceVerts2.begin())) {
								oppositeFace = realTet.meshFacets[fi];
								found = true;
								break;
							}
						}
						if(found) break;
					}
				}
				else {
					const Tetrahedron& mirrorTet = tetrahedra[mirrorFacet.first->info().index];
					oppositeFace = mirrorTet.meshFacets[mirrorFacet.second];
				}
				OVITO_ASSERT(oppositeFace != nullptr);
				OVITO_ASSERT(oppositeFace != facet);
				HalfEdgeMesh::Edge* oppositeEdge = oppositeFace->edges();
				do {
					OVITO_CHECK_POINTER(oppositeEdge);
					if(oppositeEdge->vertex1() == edge->vertex2() && oppositeEdge->vertex2() == edge->vertex1()) {
						edge->linkToOppositeEdge(oppositeEdge);
						break;
					}
					oppositeEdge = oppositeEdge->nextFaceEdge();
				}
				while(oppositeEdge != oppositeFace->edges());
				OVITO_ASSERT(edge->oppositeEdge());
			}
		}
	}

	futureInterface.setProgressText(tr("Constructing surface mesh (smoothing step)"));
	futureInterface.setProgressRange(0);
	SurfaceMesh::smoothMesh(_mesh, _simCell, _smoothingLevel);

	// Compute surface area.
	_surfaceArea = 0;
	for(const HalfEdgeMesh::Face* facet : _mesh.faces()) {
		Vector3 e1 = _simCell.wrapVector(facet->edges()->vertex1()->pos() - facet->edges()->vertex2()->pos());
		Vector3 e2 = _simCell.wrapVector(facet->edges()->prevFaceEdge()->vertex1()->pos() - facet->edges()->vertex2()->pos());
		_surfaceArea += e1.cross(e2).length();
	}
	_surfaceArea *= 0.5f;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ConstructSurfaceModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the rollout.
	QWidget* rollout = createRollout(tr("Construct surface mesh"), rolloutParams);

    QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);
	layout->setColumnStretch(1, 1);

	FloatParameterUI* radiusUI = new FloatParameterUI(this, PROPERTY_FIELD(ConstructSurfaceModifier::_radius));
	layout->addWidget(radiusUI->label(), 0, 0);
	layout->addLayout(radiusUI->createFieldLayout(), 0, 1);
	radiusUI->setMinValue(0);

	IntegerParameterUI* smoothingLevelUI = new IntegerParameterUI(this, PROPERTY_FIELD(ConstructSurfaceModifier::_smoothingLevel));
	layout->addWidget(smoothingLevelUI->label(), 1, 0);
	layout->addLayout(smoothingLevelUI->createFieldLayout(), 1, 1);
	smoothingLevelUI->setMinValue(0);

	BooleanParameterUI* onlySelectedUI = new BooleanParameterUI(this, PROPERTY_FIELD(ConstructSurfaceModifier::_onlySelectedParticles));
	layout->addWidget(onlySelectedUI->checkBox(), 2, 0, 1, 2);

	// Status label.
	layout->setRowMinimumHeight(3, 10);
	layout->addWidget(statusLabel(), 4, 0, 1, 2);
	statusLabel()->setMinimumHeight(100);
}

};	// End of namespace

