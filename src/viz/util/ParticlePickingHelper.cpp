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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/dataset/DataSetManager.h>
#include <core/scene/ObjectNode.h>
#include <core/animation/AnimManager.h>

#include <viz/data/ParticlePropertyObject.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleDisplay.h>
#include "ParticlePickingHelper.h"

namespace Viz {

/******************************************************************************
* Finds the particle under the mouse cursor.
******************************************************************************/
bool ParticlePickingHelper::pickParticle(Viewport* vp, const QPoint& clickPoint, PickResult& result)
{
	ViewportPickResult vpPickResult = vp->pick(clickPoint);
	if(vpPickResult.valid) {

		// Check if user has really clicked on a particle.
		OORef<ParticlePropertyObject> posProperty = dynamic_object_cast<ParticlePropertyObject>(vpPickResult.sceneObject);
		if(posProperty && posProperty->type() == ParticleProperty::PositionProperty && vpPickResult.subobjectId < posProperty->size()) {

			// Save reference to the selected particle.
			TimeInterval iv;
			result.objNode = vpPickResult.objectNode;
			result.particleIndex = vpPickResult.subobjectId;
			result.localPos = posProperty->getPoint3(result.particleIndex);
			result.worldPos = result.objNode->getWorldTransform(AnimManager::instance().time(), iv) * result.localPos;

			// Determine particle ID.
			result.particleId = -1;
			const PipelineFlowState& state = result.objNode->evalPipeline(AnimManager::instance().time());
			for(const auto& sceneObj : state.objects()) {
				ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
				if(property && property->type() == ParticleProperty::IdentifierProperty && result.particleIndex < property->size()) {
					result.particleId = property->getInt(result.particleIndex);
				}
			}

			return true;
		}
	}

	result.objNode = nullptr;
	return false;
}

/******************************************************************************
* Renders the atom selection overlay in a viewport.
******************************************************************************/
void ParticlePickingHelper::renderSelectionMarker(Viewport* vp, ViewportSceneRenderer* renderer, const PickResult& pickRecord)
{
	if(!pickRecord.objNode)
		return;

	if(!renderer->isInteractive() || renderer->isPicking())
		return;

	const PipelineFlowState& flowState = pickRecord.objNode->evalPipeline(AnimManager::instance().time());

	// If particle selection is based on ID, find particle with the given ID.
	size_t particleIndex = pickRecord.particleIndex;
	if(pickRecord.particleId >= 0) {
		for(const auto& sceneObj : flowState.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
			if(property && property->type() == ParticleProperty::IdentifierProperty) {
				const int* begin = property->constDataInt();
				const int* end = begin + property->size();
				const int* iter = std::find(begin, end, pickRecord.particleId);
				if(iter != end)
					particleIndex = (iter - begin);
			}
		}
	}

	// Fetch properties of selected particle needed to render the overlay.
	ParticlePropertyObject* posProperty = nullptr;
	ParticlePropertyObject* radiusProperty = nullptr;
	ParticlePropertyObject* typeProperty = nullptr;
	for(const auto& sceneObj : flowState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
		if(!property) continue;
		if(property->type() == ParticleProperty::PositionProperty && property->size() >= particleIndex)
			posProperty = property;
		else if(property->type() == ParticleProperty::RadiusProperty && property->size() >= particleIndex)
			radiusProperty = property;
		if(property->type() == ParticleProperty::ParticleTypeProperty && property->size() >= particleIndex)
			typeProperty = property;
	}
	if(!posProperty)
		return;

	ParticleDisplay* particleDisplay = dynamic_object_cast<ParticleDisplay>(posProperty->displayObject());
	if(!particleDisplay)
		return;

	// Determine position of selected particle.
	Point3 pos = posProperty->getPoint3(particleIndex);

	// Determine radius of selected particle.
	FloatType radius = particleDisplay->particleRadius(particleIndex, radiusProperty, dynamic_object_cast<ParticleTypeProperty>(typeProperty));
	if(radius <= 0)
		return;

	TimeInterval iv;
	const AffineTransformation& nodeTM = pickRecord.objNode->getWorldTransform(AnimManager::instance().time(), iv);

	// Prepare marker geometry buffer.
	if(!_markerBuffer2 || !_markerBuffer2->isValid(renderer)
			|| !_markerBuffer2->setShadingMode(particleDisplay->shadingMode())
			|| !_markerBuffer2->setRenderingQuality(particleDisplay->renderingQuality())) {
		Color markerColor(1.0, 0.0, 0.0);
		_markerBuffer2 = renderer->createParticleGeometryBuffer(particleDisplay->shadingMode(), particleDisplay->renderingQuality());
		_markerBuffer2->setSize(1);
		_markerBuffer2->setParticleColor(markerColor);
	}
	_markerBuffer2->setParticlePositions(&pos);
	_markerBuffer2->setParticleRadius(radius);

	renderer->setWorldTransform(nodeTM);
	int oldDepthFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFunc);
	glDepthFunc(GL_LEQUAL);
	_markerBuffer2->render(renderer);
	glDepthFunc(oldDepthFunc);

	// Prepare marker geometry buffer.
	if(!_markerBuffer || !_markerBuffer->isValid(renderer)) {
		ColorA markerColor(1.0, 1.0, 1.0);
		_markerBuffer = renderer->createLineGeometryBuffer();
		Point3 vertices[64];
		for(int i = 0; i < sizeof(vertices)/sizeof(vertices[0])/2; i++) {
			FloatType angle = (FloatType)i * 2.0 * FLOATTYPE_PI / (sizeof(vertices)/sizeof(vertices[0])/2);
			vertices[i*2] = Point3(0, cos(angle), sin(angle));
		}
		for(int i = 0; i < sizeof(vertices)/sizeof(vertices[0])/2; i++) {
			vertices[i*2+1] = vertices[(i*2+2)%(sizeof(vertices)/sizeof(vertices[0]))];
		}
		_markerBuffer->setSize(sizeof(vertices)/sizeof(vertices[0]));
		_markerBuffer->setVertexPositions(vertices);
		_markerBuffer->setVertexColor(markerColor);
	}
	AffineTransformation particleTM = nodeTM * AffineTransformation::translation(pos - Point3::Origin()) * AffineTransformation::scaling(radius);
	glDisable(GL_DEPTH_TEST);
	for(int i = 0; i < 6; i++) {
		renderer->setWorldTransform(particleTM * AffineTransformation::rotationZ(FLOATTYPE_PI/6 * i));
		_markerBuffer->render(renderer);
	}
#if 0
	ViewProjectionParameters vparams = renderer->projParams();
	AffineTransformation oldViewMatrix = vparams.viewMatrix;
	vparams.viewMatrix = nodeTM * AffineTransformation::translation(oldViewMatrix * pos - Point3::Origin()) * AffineTransformation::scaling(radius);
	vparams.inverseViewMatrix = vparams.viewMatrix.inverse();
	renderer->setProjParams(vparams);
	renderer->setWorldTransform(AffineTransformation::Identity());
	_markerBuffer->render(renderer);
	vparams.viewMatrix = oldViewMatrix;
	vparams.inverseViewMatrix = oldViewMatrix.inverse();
	renderer->setProjParams(vparams);
#endif
	glEnable(GL_DEPTH_TEST);
}

};	// End of namespace AtomViz
