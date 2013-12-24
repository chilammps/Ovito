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

#include <plugins/scripting/Scripting.h>
#include <plugins/scripting/engine/ScriptEngine.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/FrameBuffer.h>
#include "ViewportBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, ViewportBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void ViewportBinding::setupBinding(ScriptEngine& engine)
{
	// Install prototype for Viewport class.
	engine.setDefaultPrototype(qMetaTypeId<Viewport*>(), engine.newQObject(this));

	// Create getter function for the 'activeViewport' property, which always returns the active viewport.
	engine.globalObject().setProperty("activeViewport", engine.newStdFunction(&ViewportBinding::activeViewport, 0), QScriptValue::PropertyGetter);
}

/******************************************************************************
* Implementation of the 'activeViewport' global property.
******************************************************************************/
QScriptValue ViewportBinding::activeViewport(QScriptContext* context, ScriptEngine* engine)
{
	return engine->toScriptValue(engine->dataset()->viewportConfig()->activeViewport());
}

/******************************************************************************
* Renders the viewport contents to an output image or movie file.
******************************************************************************/
bool ViewportBinding::render(RenderSettings* settings)
{
	ScriptEngine* engine = static_cast<ScriptEngine*>(this->engine());
	Viewport* viewport = qscriptvalue_cast<Viewport*>(thisObject());
	OVITO_CHECK_OBJECT_POINTER(viewport);
	OVITO_CHECK_OBJECT_POINTER(settings);

	try {
		// Prepare a frame buffer.
		QSharedPointer<FrameBuffer> frameBuffer(new FrameBuffer(
				settings->outputImageWidth(), settings->outputImageHeight()));

		// Render.
		return engine->dataset()->renderScene(settings, viewport, frameBuffer);
	}
	catch(const Exception& ex) {
		context()->throwError(tr("Rendering failed: %1").arg(ex.message()));
		return false;
	}
}

/******************************************************************************
* Sets up a perspective camera for this viewport.
******************************************************************************/
void ViewportBinding::perspective(const Point3& cameraPos, const Vector3& cameraDir, FloatType fov)
{
	Viewport* vp = qscriptvalue_cast<Viewport*>(thisObject());
	OVITO_CHECK_OBJECT_POINTER(vp);

	vp->setViewType(Viewport::VIEW_PERSPECTIVE);
	vp->setCameraPosition(cameraPos);
	vp->setCameraDirection(cameraDir);
	vp->setFieldOfView(fov);
}

/******************************************************************************
* Sets up an orthographic camera for this viewport.
******************************************************************************/
void ViewportBinding::ortho(const Point3& cameraPos, const Vector3& cameraDir, FloatType fov)
{
	Viewport* vp = qscriptvalue_cast<Viewport*>(thisObject());
	OVITO_CHECK_OBJECT_POINTER(vp);

	vp->setViewType(Viewport::VIEW_ORTHO);
	vp->setCameraPosition(cameraPos);
	vp->setCameraDirection(cameraDir);
	vp->setFieldOfView(fov);
}

};
