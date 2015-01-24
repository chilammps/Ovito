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
#include <core/rendering/SceneRenderer.h>
#include <core/rendering/RenderSettings.h>
#include <core/scene/SceneNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/dataset/DataSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SceneRenderer, RefTarget);
IMPLEMENT_OVITO_OBJECT(Core, ObjectPickInfo, OvitoObject);

/******************************************************************************
* Constructor.
******************************************************************************/
SceneRenderer::SceneRenderer(DataSet* dataset) : RefTarget(dataset),
		_renderDataset(nullptr), _settings(nullptr),
		_viewport(nullptr), _isPicking(false)
{
}

/******************************************************************************
* Returns the final size of the rendered image in pixels.
******************************************************************************/
QSize SceneRenderer::outputSize() const
{
	return QSize(renderSettings()->outputImageWidth(), renderSettings()->outputImageHeight());
}

/******************************************************************************
* Computes the bounding box of the entire scene to be rendered.
******************************************************************************/
Box3 SceneRenderer::sceneBoundingBox(TimePoint time)
{
	OVITO_CHECK_OBJECT_POINTER(renderDataset());
	Box3 bb = renderDataset()->sceneRoot()->worldBoundingBox(time);
	if(!bb.isEmpty())
		return bb;
	else
		return Box3(Point3::Origin(), 100);
}

/******************************************************************************
* Renders all nodes in the scene
******************************************************************************/
void SceneRenderer::renderScene()
{
	OVITO_CHECK_OBJECT_POINTER(renderDataset());
	if(SceneRoot* rootNode = renderDataset()->sceneRoot()) {
		// Recursively render all nodes.
		renderNode(rootNode);
	}
}

/******************************************************************************
* Render a scene node (and all its children).
******************************************************************************/
void SceneRenderer::renderNode(SceneNode* node)
{
    OVITO_CHECK_OBJECT_POINTER(node);

    // Setup transformation matrix.
	TimeInterval interval;
	const AffineTransformation& nodeTM = node->getWorldTransform(time(), interval);
	setWorldTransform(nodeTM);

	if(ObjectNode* objNode = dynamic_object_cast<ObjectNode>(node)) {

		// Do not render node if it is the view node of the viewport or
		// if it is the target of the view node.
		if(viewport() && viewport()->viewNode()) {
			if(viewport()->viewNode() == objNode || viewport()->viewNode()->lookatTargetNode() == objNode)
				return;
		}

		// Evaluate geometry pipeline of object node and render the results.
		objNode->render(time(), this);
	}

	// Render child nodes.
	for(SceneNode* child : node->children())
		renderNode(child);
}

/******************************************************************************
* Renders the visual representation of the modifiers.
******************************************************************************/
void SceneRenderer::renderModifiers(bool renderOverlay)
{
	// Visit all pipeline objects in the scene.
	renderDataset()->sceneRoot()->visitObjectNodes([this, renderOverlay](ObjectNode* objNode) -> bool {
		if(PipelineObject* pipelineObj = dynamic_object_cast<PipelineObject>(objNode->dataProvider()))
			renderModifiers(pipelineObj, objNode, renderOverlay);
		return true;
	});
}

/******************************************************************************
* Renders the visual representation of the modifiers.
******************************************************************************/
void SceneRenderer::renderModifiers(PipelineObject* pipelineObj, ObjectNode* objNode, bool renderOverlay)
{
	OVITO_CHECK_OBJECT_POINTER(pipelineObj);

	// Render the visual representation of the modifier that is currently being edited.
	for(ModifierApplication* modApp : pipelineObj->modifierApplications()) {
		Modifier* mod = modApp->modifier();

		// Setup transformation.
		TimeInterval interval;
		setWorldTransform(objNode->getWorldTransform(time(), interval));

		// Render selected modifier.
		mod->render(time(), objNode, modApp, this, renderOverlay);
	}

	// Continue with nested pipeline objects.
	if(PipelineObject* input = dynamic_object_cast<PipelineObject>(pipelineObj->sourceObject()))
		renderModifiers(input, objNode, renderOverlay);
}

/******************************************************************************
* Determines the bounding box of the visual representation of the modifiers.
******************************************************************************/
void SceneRenderer::boundingBoxModifiers(PipelineObject* pipelineObj, ObjectNode* objNode, Box3& boundingBox)
{
	OVITO_CHECK_OBJECT_POINTER(pipelineObj);
	TimeInterval interval;

	// Render the visual representation of the modifier that is currently being edited.
	for(ModifierApplication* modApp : pipelineObj->modifierApplications()) {
		Modifier* mod = modApp->modifier();

		// Compute bounding box and transform it to world space.
		boundingBox.addBox(mod->boundingBox(time(), objNode, modApp).transformed(objNode->getWorldTransform(time(), interval)));
	}

	// Continue with nested pipeline objects.
	if(PipelineObject* input = dynamic_object_cast<PipelineObject>(pipelineObj->sourceObject()))
		boundingBoxModifiers(input, objNode, boundingBox);
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
