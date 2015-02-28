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

#include <plugins/particles/Particles.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <plugins/particles/util/NearestNeighborFinder.h>
#include "VoronoiAnalysisModifier.h"

#include <voro++.hh>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, VoronoiAnalysisModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(VoronoiAnalysisModifier, VoronoiAnalysisModifierEditor);
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _onlySelected, "OnlySelected");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _useRadii, "UseRadii");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _computeIndices, "ComputeIndices");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _edgeCount, "EdgeCount");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _edgeThreshold, "EdgeThreshold");
DEFINE_PROPERTY_FIELD(VoronoiAnalysisModifier, _faceThreshold, "FaceThreshold");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _onlySelected, "Use only selected particles");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _useRadii, "Use particle radii");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _computeIndices, "Compute Voronoi indices");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _edgeCount, "Maximum edge count");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _edgeThreshold, "Edge length threshold");
SET_PROPERTY_FIELD_LABEL(VoronoiAnalysisModifier, _faceThreshold, "Face area threshold");
SET_PROPERTY_FIELD_UNITS(VoronoiAnalysisModifier, _edgeThreshold, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, VoronoiAnalysisModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
VoronoiAnalysisModifier::VoronoiAnalysisModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_onlySelected(false), _computeIndices(false), _edgeCount(6),
	_useRadii(false), _edgeThreshold(0), _faceThreshold(0),
	_simulationBoxVolume(0), _voronoiVolumeSum(0), _maxFaceOrder(0)
{
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_onlySelected);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_useRadii);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_computeIndices);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_edgeCount);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_edgeThreshold);
	INIT_PROPERTY_FIELD(VoronoiAnalysisModifier::_faceThreshold);
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> VoronoiAnalysisModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get the current positions.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);

	// Get simulation cell.
	SimulationCellObject* inputCell = expectSimulationCell();

	// Get selection particle property.
	ParticlePropertyObject* selectionProperty = nullptr;
	if(onlySelected())
		selectionProperty = expectStandardProperty(ParticleProperty::SelectionProperty);

	// Get particle radii.
	std::vector<FloatType> radii;
	if(useRadii())
		radii = std::move(inputParticleRadii(time, validityInterval));

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<VoronoiAnalysisEngine>(
			validityInterval,
			posProperty->storage(),
			selectionProperty ? selectionProperty->storage() : nullptr,
			std::move(radii),
			inputCell->data(),
			qMax(1, edgeCount()),
			computeIndices(),
			edgeThreshold(),
			faceThreshold());
}

/******************************************************************************
* Performs the actual computation. This method is executed in a worker thread.
******************************************************************************/
void VoronoiAnalysisModifier::VoronoiAnalysisEngine::perform()
{
	setProgressText(tr("Computing Voronoi cells"));

	// Compute the total simulation cell volume.
	_simulationBoxVolume = _simCell.volume();

	if(_positions->size() == 0 || _simulationBoxVolume == 0)
		return;	// Nothing to do

	// The squared edge length threshold.
	// Add additional factor of 4 because Voronoi cell vertex coordinates are all scaled by factor of 2.
	FloatType sqEdgeThreshold = _edgeThreshold * _edgeThreshold * 4;

	auto processCell = [this, sqEdgeThreshold](voro::voronoicell& v, size_t index) {
		// Compute cell volume.
		double vol = v.volume();
		_atomicVolumes->setFloat(index, (FloatType)vol);

		// Compute total volume of Voronoi cells.
		// Loop is for lock-free write access to shared max counter.
		double prevVolumeSum = _voronoiVolumeSum;
		while(!_voronoiVolumeSum.compare_exchange_weak(prevVolumeSum, prevVolumeSum + vol));

		int localMaxFaceOrder = 0;
		// Iterate over the Voronoi faces and their edges.
		int coordNumber = 0;
		for(int i = 1; i < v.p; i++) {
			for(int j = 0; j < v.nu[i]; j++) {
				int k = v.ed[i][j];
				if(k >= 0) {
					int faceOrder = 0;
					FloatType area = 0;
					// Compute length of first face edge.
					Vector3 d(v.pts[3*k] - v.pts[3*i], v.pts[3*k+1] - v.pts[3*i+1], v.pts[3*k+2] - v.pts[3*i+2]);
					if(d.squaredLength() > sqEdgeThreshold)
						faceOrder++;
					v.ed[i][j] = -1 - k;
					int l = v.cycle_up(v.ed[i][v.nu[i]+j], k);
					do {
						int m = v.ed[k][l];
						// Compute length of current edge.
						if(sqEdgeThreshold != 0) {
							Vector3 u(v.pts[3*m] - v.pts[3*k], v.pts[3*m+1] - v.pts[3*k+1], v.pts[3*m+2] - v.pts[3*k+2]);
							if(u.squaredLength() > sqEdgeThreshold)
								faceOrder++;
						}
						else faceOrder++;
						if(_faceThreshold != 0) {
							Vector3 w(v.pts[3*m] - v.pts[3*i], v.pts[3*m+1] - v.pts[3*i+1], v.pts[3*m+2] - v.pts[3*i+2]);
							area += d.cross(w).length() / 8;
							d = w;
						}
						v.ed[k][l] = -1 - m;
						l = v.cycle_up(v.ed[k][v.nu[k]+l], m);
						k = m;
					}
					while(k != i);
					if((_faceThreshold == 0 || area > _faceThreshold) && faceOrder >= 3) {
						coordNumber++;
						if(faceOrder > localMaxFaceOrder)
							localMaxFaceOrder = faceOrder;
						faceOrder--;
						if(_voronoiIndices && faceOrder < (int)_voronoiIndices->componentCount())
							_voronoiIndices->setIntComponent(index, faceOrder, _voronoiIndices->getIntComponent(index, faceOrder) + 1);
					}
				}
			}
		}

		// Store computed result.
		_coordinationNumbers->setInt(index, coordNumber);

		// Keep track of the maximum number of edges per face.
		// Loop is for lock-free write access to shared max counter.
		int prevMaxFaceOrder = _maxFaceOrder;
		while(localMaxFaceOrder > prevMaxFaceOrder && !_maxFaceOrder.compare_exchange_weak(prevMaxFaceOrder, localMaxFaceOrder));
	};

	// Decide whether to use Voro++ container class or our own implementation.
	if(_simCell.isAxisAligned()) {
		// Use Voro++ container.
		double ax = _simCell.matrix()(0,3);
		double ay = _simCell.matrix()(1,3);
		double az = _simCell.matrix()(2,3);
		double bx = ax + _simCell.matrix()(0,0);
		double by = ay + _simCell.matrix()(1,1);
		double bz = az + _simCell.matrix()(2,2);
		if(ax > bx) std::swap(ax,bx);
		if(ay > by) std::swap(ay,by);
		if(az > bz) std::swap(az,bz);
		double volumePerCell = (bx - ax) * (by - ay) * (bz - az) * voro::optimal_particles / _positions->size();
		double cellSize = pow(volumePerCell, 1.0/3.0);
		int nx = (int)std::ceil((bx - ax) / cellSize);
		int ny = (int)std::ceil((by - ay) / cellSize);
		int nz = (int)std::ceil((bz - az) / cellSize);

		if(_radii.empty()) {
			voro::container voroContainer(ax, bx, ay, by, az, bz, nx, ny, nz,
					_simCell.pbcFlags()[0], _simCell.pbcFlags()[1], _simCell.pbcFlags()[2], (int)std::ceil(voro::optimal_particles));

			// Insert particles into Voro++ container.
			size_t count = 0;
			for(size_t index = 0; index < _positions->size(); index++) {
				// Skip unselected particles (if requested).
				if(_selection && _selection->getInt(index) == 0)
					continue;
				const Point3& p = _positions->getPoint3(index);
				voroContainer.put(index, p.x(), p.y(), p.z());
				count++;
			}
			if(!count) return;

			setProgressRange(count);
			setProgressValue(0);
			voro::c_loop_all cl(voroContainer);
			voro::voronoicell v;
			if(cl.start()) {
				do {
					incrementProgressValue();
					if(isCanceled()) return;
					if(!voroContainer.compute_cell(v,cl))
						continue;
					processCell(v, cl.pid());
					count--;
				}
				while(cl.inc());
			}
			if(count)
				throw Exception(tr("Could not compute Voronoi cell for some particles."));
		}
		else {
			voro::container_poly voroContainer(ax, bx, ay, by, az, bz, nx, ny, nz,
					_simCell.pbcFlags()[0], _simCell.pbcFlags()[1], _simCell.pbcFlags()[2], (int)std::ceil(voro::optimal_particles));

			// Insert particles into Voro++ container.
			size_t count = 0;
			for(size_t index = 0; index < _positions->size(); index++) {
				// Skip unselected particles (if requested).
				if(_selection && _selection->getInt(index) == 0)
					continue;
				const Point3& p = _positions->getPoint3(index);
				voroContainer.put(index, p.x(), p.y(), p.z(), _radii[index]);
				count++;
			}

			if(!count) return;
			setProgressRange(count);
			setProgressValue(0);
			voro::c_loop_all cl(voroContainer);
			voro::voronoicell v;
			if(cl.start()) {
				do {
					incrementProgressValue();
					if(isCanceled()) return;
					if(!voroContainer.compute_cell(v,cl))
						continue;
					processCell(v, cl.pid());
					count--;
				}
				while(cl.inc());
			}
			if(count)
				throw Exception(tr("Could not compute Voronoi cell for some particles."));
		}
	}
	else {
		// Prepare the nearest neighbor list generator.
		NearestNeighborFinder nearestNeighborFinder;
		if(!nearestNeighborFinder.prepare(_positions.data(), _simCell, this))
			return;

		// Squared particle radii (input was just radii).
		for(auto& r : _radii)
			r = r*r;

		// This is the size we use to initialize Voronoi cells. Must be larger than the simulation box.
		double boxDiameter = sqrt(
				  _simCell.matrix().column(0).squaredLength()
				+ _simCell.matrix().column(1).squaredLength()
				+ _simCell.matrix().column(2).squaredLength());

		// The normal vectors of the three cell planes.
		std::array<Vector3,3> planeNormals;
		planeNormals[0] = _simCell.cellNormalVector(0);
		planeNormals[1] = _simCell.cellNormalVector(1);
		planeNormals[2] = _simCell.cellNormalVector(2);

		Point3 corner1 = Point3::Origin() + _simCell.matrix().column(3);
		Point3 corner2 = corner1 + _simCell.matrix().column(0) + _simCell.matrix().column(1) + _simCell.matrix().column(2);

		// Perform analysis, particle-wise parallel.
		parallelFor(_positions->size(), *this,
				[&nearestNeighborFinder, this, sqEdgeThreshold, boxDiameter,
				 planeNormals, corner1, corner2, &processCell](size_t index) {

			// Skip unselected particles (if requested).
			if(_selection && _selection->getInt(index) == 0)
				return;

			// Build Voronoi cell.
			voro::voronoicell v;

			// Initialize the Voronoi cell to be a cube larger than the simulation cell, centered at the origin.
			v.init(-boxDiameter, boxDiameter, -boxDiameter, boxDiameter, -boxDiameter, boxDiameter);

			// Cut Voronoi cell at simulation cell boundaries in non-periodic directions.
			bool skipParticle = false;
			for(size_t dim = 0; dim < 3; dim++) {
				if(!_simCell.pbcFlags()[dim]) {
					double r;
					r = 2 * planeNormals[dim].dot(corner2 - _positions->getPoint3(index));
					if(r <= 0) skipParticle = true;
					v.plane(planeNormals[dim].x() * r, planeNormals[dim].y() * r, planeNormals[dim].z() * r, r*r);
					r = 2 * planeNormals[dim].dot(_positions->getPoint3(index) - corner1);
					if(r <= 0) skipParticle = true;
					v.plane(-planeNormals[dim].x() * r, -planeNormals[dim].y() * r, -planeNormals[dim].z() * r, r*r);
				}
			}
			// Skip particles that are located outside of non-periodic box boundaries.
			if(skipParticle)
				return;

			// This function will be called for every neighbor particle.
			int nvisits = 0;
			auto visitFunc = [this, &v, &nvisits, index](const NearestNeighborFinder::Neighbor& n, FloatType& mrs) {
				// Skip unselected particles (if requested).
				if(!_selection || _selection->getInt(n.index)) {
					FloatType rs = n.distanceSq;
					if(!_radii.empty())
						 rs += _radii[index] - _radii[n.index];
					v.plane(n.delta.x(), n.delta.y(), n.delta.z(), rs);
				}
				if(nvisits == 0) {
					mrs = v.max_radius_squared();
					nvisits = 100;
				}
				nvisits--;
			};

			// Visit all neighbors of the current particles.
			nearestNeighborFinder.visitNeighbors(nearestNeighborFinder.particlePos(index), visitFunc);

			processCell(v,index);
		});
	}
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void VoronoiAnalysisModifier::transferComputationResults(ComputeEngine* engine)
{
	VoronoiAnalysisEngine* eng = static_cast<VoronoiAnalysisEngine*>(engine);
	_coordinationNumbers = eng->coordinationNumbers();
	_atomicVolumes = eng->atomicVolumes();
	_voronoiIndices = eng->voronoiIndices();
	_simulationBoxVolume = eng->simulationBoxVolume();
	_voronoiVolumeSum = eng->voronoiVolumeSum();
	_maxFaceOrder = eng->maxFaceOrder();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus VoronoiAnalysisModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_coordinationNumbers)
		throw Exception(tr("No computation results available."));

	if(inputParticleCount() != _coordinationNumbers->size())
		throw Exception(tr("The number of input particles has changed. The stored results have become invalid."));

	outputStandardProperty(_coordinationNumbers.data());
	outputCustomProperty(_atomicVolumes.data());
	if(_voronoiIndices)
		outputCustomProperty(_voronoiIndices.data());

	// Check computed Voronoi cell volume sum.
	if(std::abs(_voronoiVolumeSum - _simulationBoxVolume) > 1e-9 * inputParticleCount() * _simulationBoxVolume) {
		return PipelineStatus(PipelineStatus::Warning,
				tr("The volume sum of all Voronoi cells does not match the simulation box volume. "
						"This may be a result of particles being located outside of the simulation box boundaries. "
						"See user manual for more information.\n"
						"Simulation box volume: %1\n"
						"Voronoi cell volume sum: %2").arg(_simulationBoxVolume).arg(_voronoiVolumeSum));
	}

	if(_voronoiIndices && _maxFaceOrder > _voronoiIndices->componentCount()) {
		return PipelineStatus(PipelineStatus::Warning,
				tr("The Voronoi tessellation contains faces with up to %1 edges "
						"(ignoring edges below the length threshold). "
						"The current maximum edge count parameter is less than this "
						"value, and the computed Voronoi index vectors are therefore truncated. "
						"You should consider increasing the maximum edge count parameter to %1 edges "
						"to not truncate the Voronoi index vectors and avoid this message."
						).arg(_maxFaceOrder));
	}

	return PipelineStatus::Success;
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void VoronoiAnalysisModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute modifier results when the parameters have been changed.
	invalidateCachedResults();
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void VoronoiAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Voronoi analysis"), rolloutParams, "particles.modifiers.voronoi_analysis.html");

    // Create the rollout contents.
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);

	QGridLayout* gridlayout = new QGridLayout();
	QGridLayout* sublayout;
	gridlayout->setContentsMargins(4,4,4,4);
	gridlayout->setSpacing(4);
	gridlayout->setColumnStretch(1, 1);
	int row = 0;

	// Face threshold.
	FloatParameterUI* faceThresholdPUI = new FloatParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_faceThreshold));
	gridlayout->addWidget(faceThresholdPUI->label(), row, 0);
	gridlayout->addLayout(faceThresholdPUI->createFieldLayout(), row++, 1);
	faceThresholdPUI->setMinValue(0);

	// Compute indices.
	BooleanGroupBoxParameterUI* computeIndicesPUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_computeIndices));
	gridlayout->addWidget(computeIndicesPUI->groupBox(), row++, 0, 1, 2);
	sublayout = new QGridLayout(computeIndicesPUI->childContainer());
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(1, 1);

	// Edge count parameter.
	IntegerParameterUI* edgeCountPUI = new IntegerParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_edgeCount));
	sublayout->addWidget(edgeCountPUI->label(), 0, 0);
	sublayout->addLayout(edgeCountPUI->createFieldLayout(), 0, 1);
	edgeCountPUI->setMinValue(3);
	edgeCountPUI->setMaxValue(18);

	// Edge threshold.
	FloatParameterUI* edgeThresholdPUI = new FloatParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_edgeThreshold));
	sublayout->addWidget(edgeThresholdPUI->label(), 1, 0);
	sublayout->addLayout(edgeThresholdPUI->createFieldLayout(), 1, 1);
	edgeThresholdPUI->setMinValue(0);

	// Atomic radii.
	BooleanParameterUI* useRadiiPUI = new BooleanParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_useRadii));
	gridlayout->addWidget(useRadiiPUI->checkBox(), row++, 0, 1, 2);

	// Only selected particles.
	BooleanParameterUI* onlySelectedPUI = new BooleanParameterUI(this, PROPERTY_FIELD(VoronoiAnalysisModifier::_onlySelected));
	gridlayout->addWidget(onlySelectedPUI->checkBox(), row++, 0, 1, 2);

	layout->addLayout(gridlayout);

	// Status label.
	layout->addSpacing(6);
	layout->addWidget(statusLabel());
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
