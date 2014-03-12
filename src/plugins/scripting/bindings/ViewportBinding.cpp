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
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include "ViewportBinding.h"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, ViewportBinding, ScriptBinding);

/******************************************************************************
* Sets up the global object of the script engine.
******************************************************************************/
void ViewportBinding::setupBinding(ScriptEngine& engine)
{
	// Install prototype for Viewport class.
	QScriptValue viewportProto = engine.newQObject(this);
	viewportProto.setProperty("render", engine.newStdFunction(&ViewportBinding::render, 0));
	engine.setDefaultPrototype(qMetaTypeId<Viewport*>(), viewportProto);

	// Create getter function for the 'activeViewport' property, which always returns the active viewport.
	engine.globalObject().setProperty("activeViewport", engine.newStdFunction(&ViewportBinding::activeViewport, 0), QScriptValue::PropertyGetter);
}

/******************************************************************************
* Implementation of the 'activeViewport' global property.
******************************************************************************/
QScriptValue ViewportBinding::activeViewport(QScriptContext* context, ScriptEngine* engine)
{
	return engine->wrapOvitoObject(engine->dataset()->viewportConfig()->activeViewport());
}

/******************************************************************************
* Renders the viewport contents to an output image or movie file.
******************************************************************************/
QScriptValue ViewportBinding::render(QScriptContext* context, ScriptEngine* engine)
{
	Viewport* viewport = ScriptEngine::getThisObject<Viewport>(context);
	if(!viewport)
		return context->throwError(QScriptContext::TypeError, tr("Viewport.prototype.render: This is not a Viewport."));

	// Check if a RenderSettings object has been passed to the function.
	OORef<RenderSettings> settings;
	if(context->argumentCount() > 0) {
		settings = qscriptvalue_cast<RenderSettings*>(context->argument(0));

		// Check if an object literal has been passed, which can be used to initialize a new RenderSettings object.
		if(!settings && context->argument(0).isObject()) {
			settings = new RenderSettings(engine->dataset());
			QScriptValue sv = engine->wrapOvitoObject(settings);

			// Iterate over all properties of the object literal and
			// copy them over to the newly created object.
			QScriptValueIterator it(context->argument(0));
			while(it.hasNext()) {
				it.next();
				sv.setProperty(it.name(), it.value());
			}
		}
	}

	// If no RenderSettings object has been passed to the function, use the global settings from the DataSet.
	if(!settings)
		settings = viewport->dataset()->renderSettings();
	OVITO_CHECK_OBJECT_POINTER(settings);

	// Prepare the frame buffer.
	QSharedPointer<FrameBuffer> frameBuffer;
	FrameBufferWindow* frameBufferWindow = nullptr;
	if(Application::instance().guiMode()) {
		frameBufferWindow = viewport->dataset()->mainWindow()->frameBufferWindow();
		frameBuffer = frameBufferWindow->frameBuffer();
	}
	if(!frameBuffer)
		frameBuffer.reset(new FrameBuffer(settings->outputImageWidth(), settings->outputImageHeight()));

	// Render.
	return engine->toScriptValue(engine->dataset()->renderScene(settings.get(), viewport, frameBuffer, frameBufferWindow));
}

/******************************************************************************
* Sets up a perspective camera for this viewport.
******************************************************************************/
void ViewportBinding::perspective(const Point3& cameraPos, const Vector3& cameraDir, FloatType fov)
{
	Viewport* vp = ScriptEngine::getThisObject<Viewport>(context());
	if(!vp) {
		context()->throwError(QScriptContext::TypeError, tr("Viewport.prototype.perspective: This is not a Viewport."));
		return;
	}
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
	Viewport* vp = ScriptEngine::getThisObject<Viewport>(context());
	if(!vp) {
		context()->throwError(QScriptContext::TypeError, tr("Viewport.prototype.ortho: This is not a Viewport."));
		return;
	}
	OVITO_CHECK_OBJECT_POINTER(vp);

	vp->setViewType(Viewport::VIEW_ORTHO);
	vp->setCameraPosition(cameraPos);
	vp->setCameraDirection(cameraDir);
	vp->setFieldOfView(fov);
}

};
