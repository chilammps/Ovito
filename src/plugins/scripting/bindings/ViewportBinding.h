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

#ifndef __OVITO_SCRIPTING_VIEWPORT_BINDING_H
#define __OVITO_SCRIPTING_VIEWPORT_BINDING_H

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptBinding.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportConfiguration.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief Wrapper for the Viewport C++ class.
 */
class ViewportBinding : public ScriptBinding, public QScriptable
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE ViewportBinding() {}

	/// \brief Sets up the global object of the script engine.
	virtual void setupBinding(ScriptEngine& engine) override;

	/// Renders the viewport contents to an output image or movie file.
	static QScriptValue render(QScriptContext* context, ScriptEngine* engine);

	/// Sets up a perspective camera for this viewport.
	Q_INVOKABLE void perspective(const Point3& cameraPos, const Vector3& cameraDir, FloatType fov);

	/// Sets up an orthographic camera for this viewport.
	Q_INVOKABLE void ortho(const Point3& cameraPos, const Vector3& cameraDir, FloatType fov);

private:

	/// Implementation of the 'activeViewport' global property.
	static QScriptValue activeViewport(QScriptContext* context, ScriptEngine* engine);

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif
