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

#ifndef __OVITO_SCENEROOT_H
#define __OVITO_SCENEROOT_H

#include <core/Core.h>
#include <core/animation/TimeInterval.h>
#include "SceneNode.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief This is the scene's root node.
 */
class OVITO_CORE_EXPORT SceneRoot : public SceneNode
{
public:

	/// \brief Creates a root node.
	Q_INVOKABLE SceneRoot(DataSet* dataset);

	/// \brief Searches the scene for a node with the given name.
	/// \param nodeName The name to look for. 
	/// \return The scene node or \c NULL, if there is no node with the given name.
	SceneNode* getNodeByName(const QString& nodeName) const;

	/// \brief Generates a name for a node that is unique throughout the scene.
	/// \param baseName A base name that will be made unique by appending a number.
	/// \return The generated unique name.
	QString makeNameUnique(QString baseName) const;

	/// \brief Returns the bounding box of the scene.
	/// \param time The time at which the bounding box should be computed.
	/// \return An world axis-aligned box that contains the bounding boxes of all child nodes.
	virtual Box3 localBoundingBox(TimePoint time) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_SCENEROOT_H
