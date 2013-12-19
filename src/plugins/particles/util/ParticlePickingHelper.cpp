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

#include <plugins/particles/Particles.h>
#include <core/viewport/Viewport.h>
#include <core/scene/ObjectNode.h>
#include <core/animation/AnimationSettings.h>

#include <plugins/particles/data/ParticlePropertyObject.h>
#include <plugins/particles/data/ParticleTypeProperty.h>
#include <plugins/particles/data/ParticleDisplay.h>
#include "ParticlePickingHelper.h"

namespace Particles {

/******************************************************************************
* Finds the particle under the mouse cursor.
******************************************************************************/
bool ParticlePickingHelper::pickParticle(Viewport* vp, const QPoint& clickPoint, PickResult& result)
{
	ViewportPickResult vpPickResult = vp->pick(clickPoint);
	// Check if user has clicked on something.
	if(vpPickResult.valid) {

		// Check if that was a particle.
		OORef<ParticlePropertyObject> posProperty = dynamic_object_cast<ParticlePropertyObject>(vpPickResult.sceneObject);
		if(posProperty && posProperty->type() == ParticleProperty::PositionProperty && vpPickResult.subobjectId < posProperty->size()) {

			// Save reference to the selected particle.
			TimeInterval iv;
			result.objNode = vpPickResult.objectNode;
			result.particleIndex = vpPickResult.subobjectId;
			result.localPos = posProperty->getPoint3(result.particleIndex);
			result.worldPos = result.objNode->getWorldTransform(vp->dataset()->animationSettings()->time(), iv) * result.localPos;

			// Determine particle ID.
			result.particleId = -1;
			const PipelineFlowState& state = result.objNode->evalPipeline(vp->dataset()->animationSettings()->time());
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

	const PipelineFlowState& flowState = pickRecord.objNode->evalPipeline(vp->dataset()->animationSettings()->time());

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
	ParticlePropertyObject* colorProperty = nullptr;
	ParticlePropertyObject* selectionProperty = nullptr;
	ParticleTypeProperty* typeProperty = nullptr;
	for(const auto& sceneObj : flowState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(sceneObj.get());
		if(!property) continue;
		if(property->type() == ParticleProperty::PositionProperty && property->size() >= particleIndex)
			posProperty = property;
		else if(property->type() == ParticleProperty::RadiusProperty && property->size() >= particleIndex)
			radiusProperty = property;
		else if(property->type() == ParticleProperty::ParticleTypeProperty && property->size() >= particleIndex)
			typeProperty = dynamic_object_cast<ParticleTypeProperty>(property);
		else if(property->type() == ParticleProperty::ColorProperty && property->size() >= particleIndex)
			colorProperty = property;
		else if(property->type() == ParticleProperty::SelectionProperty && property->size() >= particleIndex)
			selectionProperty = property;
	}
	if(!posProperty)
		return;

	// Get the particle display object, which is attached to the position property object.
	ParticleDisplay* particleDisplay = nullptr;
	for(DisplayObject* displayObj : posProperty->displayObjects()) {
		if((particleDisplay = dynamic_object_cast<ParticleDisplay>(displayObj)) != nullptr)
			break;
	}
	if(!particleDisplay)
		return;

	// Determine position of selected particle.
	Point3 pos = posProperty->getPoint3(particleIndex);

	// Determine radius of selected particle.
	FloatType radius = particleDisplay->particleRadius(particleIndex, radiusProperty, typeProperty);
	if(radius <= 0)
		return;

	// Determine the display color of selected particle.
	Color color = particleDisplay->particleColor(particleIndex, colorProperty, typeProperty, selectionProperty);
	Color highlightColor = particleDisplay->selectionParticleColor();

	// Determine rendering quality used to render the particles.
	ParticleGeometryBuffer::RenderingQuality renderQuality = particleDisplay->effectiveRenderingQuality(renderer, posProperty);

	TimeInterval iv;
	const AffineTransformation& nodeTM = pickRecord.objNode->getWorldTransform(vp->dataset()->animationSettings()->time(), iv);

	if(!_particleBuffer || !_particleBuffer->isValid(renderer)
			|| !_particleBuffer->setShadingMode(particleDisplay->shadingMode())
			|| !_particleBuffer->setRenderingQuality(renderQuality)) {
		_particleBuffer = renderer->createParticleGeometryBuffer(
				particleDisplay->shadingMode(),
				renderQuality,
				particleDisplay->particleShape());
		_particleBuffer->setSize(1);
	}
	_particleBuffer->setParticleColor(color * 0.5f + highlightColor * 0.5f);
	_particleBuffer->setParticlePositions(&pos);
	_particleBuffer->setParticleRadius(radius);

	// Prepare marker geometry buffer.
	if(!_highlightBuffer || !_highlightBuffer->isValid(renderer)
			|| !_highlightBuffer->setShadingMode(particleDisplay->shadingMode())
			|| !_highlightBuffer->setRenderingQuality(renderQuality)) {
		_highlightBuffer = renderer->createParticleGeometryBuffer(
				particleDisplay->shadingMode(),
				renderQuality,
				particleDisplay->particleShape());
		_highlightBuffer->setSize(1);
		_highlightBuffer->setParticleColor(highlightColor);
	}
	_highlightBuffer->setParticlePositions(&pos);
	_highlightBuffer->setParticleRadius(radius + vp->nonScalingSize(nodeTM * pos) * 1e-1f);

	renderer->setWorldTransform(nodeTM);
	glEnable(GL_DEPTH_TEST);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0x1, 0x1);
	glStencilMask(0x1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glDepthFunc(GL_LEQUAL);
	_particleBuffer->render(renderer);
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
	glStencilMask(0x1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	_highlightBuffer->render(renderer);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

};	// End of namespace
