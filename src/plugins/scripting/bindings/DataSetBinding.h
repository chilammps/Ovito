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

#ifndef __OVITO_SCRIPTING_DATASET_BINDING_H
#define __OVITO_SCRIPTING_DATASET_BINDING_H

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptBinding.h>
#include <core/dataset/DataSet.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief Wrapper for the DataSet C++ class.
 */
class DataSetBinding : public ScriptBinding, public QScriptable
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE DataSetBinding() {}

	/// \brief Sets up the global object of the script engine.
	virtual void setupBinding(ScriptEngine& engine) override;

	/// Returns the currently selected scene node.
	SceneNode* selectedNode() const;

	/// Selects a scene node.
	void setSelectedNode(SceneNode* node);

	Q_PROPERTY(SceneNode* selectedNode READ selectedNode WRITE setSelectedNode);

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif
