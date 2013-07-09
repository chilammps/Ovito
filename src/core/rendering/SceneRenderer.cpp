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
#include <core/scene/SceneNode.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetManager.h>
#include "moc_LineGeometryBuffer.cpp"
#include "moc_ParticleGeometryBuffer.cpp"
#include "moc_TextGeometryBuffer.cpp"
#include "moc_ImageGeometryBuffer.cpp"

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SceneRenderer, RefTarget);
IMPLEMENT_OVITO_OBJECT(Core, LineGeometryBuffer, OvitoObject);
IMPLEMENT_OVITO_OBJECT(Core, ParticleGeometryBuffer, OvitoObject);
IMPLEMENT_OVITO_OBJECT(Core, TextGeometryBuffer, OvitoObject);
IMPLEMENT_OVITO_OBJECT(Core, ImageGeometryBuffer, OvitoObject);

/******************************************************************************
* Constructor.
******************************************************************************/
SceneRenderer::SceneRenderer() : _dataset(nullptr), _settings(nullptr), _viewport(nullptr), _isPicking(false)
{
}

/******************************************************************************
* Computes the bounding box of the entire scene to be rendered.
******************************************************************************/
Box3 SceneRenderer::sceneBoundingBox(TimePoint time)
{
	OVITO_CHECK_OBJECT_POINTER(dataset());
	Box3 bb = dataset()->sceneRoot()->worldBoundingBox(time);
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
	OVITO_CHECK_OBJECT_POINTER(dataset());

	SceneRoot* rootNode = dataset()->sceneRoot();
	if(rootNode) {

		// Recursively render all nodes.
		renderNode(rootNode);
	}
}

/******************************************************************************
* Render a scene node (and all its children).
******************************************************************************/
void SceneRenderer::renderNode(SceneNode* node)
{
	for(SceneNode* child : node->children())
		renderNode(child);
}

};
