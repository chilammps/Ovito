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
#include <core/scene/SceneRoot.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SceneRoot, SceneNode);

/******************************************************************************
* Default constructor.
******************************************************************************/
SceneRoot::SceneRoot(DataSet* dataset) : SceneNode(dataset)
{
	setName("Scene");

	// The root node does not need a transformation controller.
	setTransformationController(nullptr);
}

/******************************************************************************
* Searches the scene for a node with the given name.
******************************************************************************/
SceneNode* SceneRoot::getNodeByName(const QString& nodeName) const
{
	SceneNode* result = nullptr;
	visitChildren([nodeName, &result](SceneNode* node) -> bool {
		if(node->name() == nodeName) {
			result = node;
			return false;
		}
		return true;
	});
	return result;
}

/******************************************************************************
* Generates a name for a node that is unique throughout the scene.
******************************************************************************/
QString SceneRoot::makeNameUnique(QString baseName) const
{
	// Remove any existing digits from end of base name.
	if(baseName.size() > 2 && 
		baseName.at(baseName.size()-1).isDigit() && baseName.at(baseName.size()-2).isDigit())
		baseName.chop(2);
		
	// Keep appending different numbers until we arrive at a unique name.
	for(int i = 1; ; i++) {
		QString newName = baseName + QString::number(i).rightJustified(2, '0');
		if(getNodeByName(newName) == nullptr)
			return newName;
	}		
}

/******************************************************************************
* Returns the bounding box of the scene.
******************************************************************************/
Box3 SceneRoot::localBoundingBox(TimePoint time)
{
	Box3 myBox;
	TimeInterval iv = TimeInterval::infinite();
	for(SceneNode* child : children()) {
		// Get local child bounding box.
		Box3 childBox = child->localBoundingBox(time);
		// Transform to parent coordinate system.
		AffineTransformation childTM = child->getLocalTransform(time, iv);
		// Add to parent box.
		myBox.addBox(childBox.transformed(childTM));
	}
	return myBox;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
