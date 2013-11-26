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
#include <plugins/crystalanalysis/data/surface/DefectSurface.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <plugins/particles/data/SimulationCell.h>
#include "SmoothSurfaceModifier.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, SmoothSurfaceModifier, Modifier)
IMPLEMENT_OVITO_OBJECT(CrystalAnalysis, SmoothSurfaceModifierEditor, PropertiesEditor)
SET_OVITO_OBJECT_EDITOR(SmoothSurfaceModifier, SmoothSurfaceModifierEditor)
DEFINE_FLAGS_PROPERTY_FIELD(SmoothSurfaceModifier, _smoothingLevel, "SmoothingLevel", PROPERTY_FIELD_MEMORIZE)
SET_PROPERTY_FIELD_LABEL(SmoothSurfaceModifier, _smoothingLevel, "Smoothing level")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
SmoothSurfaceModifier::SmoothSurfaceModifier() : _smoothingLevel(8)
{
	INIT_PROPERTY_FIELD(SmoothSurfaceModifier::_smoothingLevel);
}

/******************************************************************************
* Asks the modifier whether it can be applied to the given input data.
******************************************************************************/
bool SmoothSurfaceModifier::isApplicableTo(const PipelineFlowState& input)
{
	return (input.findObject<DefectSurface>() != nullptr);
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus SmoothSurfaceModifier::modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state)
{
	if(_smoothingLevel <= 0)
		return ObjectStatus::Success;

	DefectSurface* inputSurface = state.findObject<DefectSurface>();
	if(!inputSurface)
		return ObjectStatus::Success;	// Nothing to smooth in the modifier's input.

	// Get simulation cell geometry and periodic boundary flags.
	SimulationCellData cell;
	if(SimulationCell* simulationCellObj = state.findObject<SimulationCell>())
		cell = simulationCellObj->data();
	else
		cell.setPbcFlags(false, false, false);

	CloneHelper cloneHelper;
	OORef<DefectSurface> outputSurface = cloneHelper.cloneObject(inputSurface, false);

	// This is the implementation of the mesh smoothing algorithm:
	//
	// Gabriel Taubin
	// A Signal Processing Approach To Fair Surface Design
	// In SIGGRAPH 95 Conference Proceedings, pages 351-358 (1995)

	FloatType k_PB = 0.1f;
	FloatType lambda = 0.5f;
	FloatType mu = 1.0f / (k_PB - 1.0f/lambda);

	for(int iteration = 0; iteration < _smoothingLevel; iteration++) {
		smoothMesh(outputSurface->mesh(), lambda, cell, false);
		smoothMesh(outputSurface->mesh(), mu, cell, false);
	}

	outputSurface->notifyDependents(ReferenceEvent::TargetChanged);
	state.replaceObject(inputSurface, outputSurface);
	return ObjectStatus::Success;
}

/******************************************************************************
* Performs one iteration of the smoothing algorithm.
******************************************************************************/
void SmoothSurfaceModifier::smoothMesh(HalfEdgeMesh& mesh, FloatType prefactor, const SimulationCellData& cell, bool projectToNormals)
{
	const AffineTransformation absoluteToReduced = cell.matrix().inverse();
	const AffineTransformation reducedToAbsolute = cell.matrix();

	// Compute displacement for each vertex.
	std::vector<Vector3> displacements(mesh.vertices().size());
	parallelFor(mesh.vertices().size(), [&mesh, &displacements, prefactor, cell, absoluteToReduced](int index) {
		HalfEdgeMesh::Vertex* vertex = mesh.vertices()[index];
		Vector3& d = displacements[index];
		d = Vector3::Zero();
		for(HalfEdgeMesh::Edge* edge = vertex->edges(); edge != nullptr; edge = edge->nextVertexEdge()) {
			Vector3 delta = edge->vertex2()->pos() - vertex->pos();
			Vector3 delta_r = absoluteToReduced * delta;
			for(size_t dim = 0; dim < 3; dim++) {
				if(cell.pbcFlags()[dim]) {
					while(delta_r[dim] > FloatType(0.5)) { delta_r[dim] -= FloatType(1); delta -= cell.matrix().column(dim); }
					while(delta_r[dim] < FloatType(-0.5)) { delta_r[dim] += FloatType(1); delta += cell.matrix().column(dim); }
				}
			}
			d += delta;
		}
		d *= (prefactor / vertex->numEdges());
	});

	// Apply displacements.
	auto d = displacements.cbegin();
	for(HalfEdgeMesh::Vertex* vertex : mesh.vertices())
		vertex->pos() += *d++;
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SmoothSurfaceModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the first rollout.
	QWidget* rollout = createRollout(tr("Smooth surface"), rolloutParams);

    QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(6);
	layout->setColumnStretch(1, 1);

	IntegerParameterUI* smoothingLevelUI = new IntegerParameterUI(this, PROPERTY_FIELD(SmoothSurfaceModifier::_smoothingLevel));
	layout->addWidget(smoothingLevelUI->label(), 0, 0);
	layout->addLayout(smoothingLevelUI->createFieldLayout(), 0, 1);
	smoothingLevelUI->setMinValue(0);
}


};	// End of namespace

