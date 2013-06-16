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
#include <core/gui/undo/UndoManager.h>
#include <core/viewport/Viewport.h>

namespace Ovito {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, SceneRoot, SceneNode)

/******************************************************************************
* Default constructor.
******************************************************************************/
SceneRoot::SceneRoot()
{
	setName("Scene Root");

	// Root node does not need a tm controller.
	setTransformationController(nullptr);
}

/******************************************************************************
* Searches recursively for the scene node with the given display name.
******************************************************************************/
SceneNode* SceneRoot::getNodeByNameImpl(const QString& nodeName, const SceneNode* node) const
{
	for(SceneNode* child : node->children()) {
		if(child->name() == nodeName) return child;
		SceneNode* result = getNodeByNameImpl(nodeName, child);
		if(result) return result;
	}
	return nullptr;
}

/******************************************************************************
* Alters the given base name of a scene node so that it
* becomes unique in the whole scene. Returns the generated unique name.
******************************************************************************/
QString SceneRoot::makeNameUnique(QString baseName) const
{
	// Remove digits from end of base name.
	if(baseName.size() > 2 && 
		baseName.at(baseName.size()-1).isDigit() && baseName.at(baseName.size()-2).isDigit())
		baseName.chop(2);
		
	int startIndex = 1;
	for(int i = startIndex; ; i++) {
		QString newName = baseName + QString::number(i).rightJustified(2, '0');
		if(getNodeByName(newName) == NULL)
			return newName;
	}		
}


/******************************************************************************
* Returns the bounding box of the scene.
******************************************************************************/
Box3 SceneRoot::localBoundingBox(TimePoint time)
{
	Box3 myBox;
	TimeInterval iv = TimeInterval::forever();
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

};
