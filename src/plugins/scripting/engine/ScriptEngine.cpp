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
#include <core/plugins/PluginManager.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/SceneNode.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/GroupNode.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/animation/AnimationSettings.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/controller/TransformationController.h>
#include <core/animation/controller/LookAtController.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/SceneRenderer.h>
#include "ScriptEngine.h"
#include "ScriptBinding.h"
#include "moc_ScriptBinding.cpp"

namespace Scripting {

IMPLEMENT_OVITO_OBJECT(Scripting, ScriptBinding, OvitoObject);

/******************************************************************************
* Initializes the scripting engine and sets up the environment.
******************************************************************************/
ScriptEngine::ScriptEngine(DataSet* dataset, QObject* parent)
	: QScriptEngine(parent), _dataset(dataset)
{
	using namespace std;
	using namespace std::placeholders;

	// Register the most important classes such that they can be used in scripts.
	// This will install marshaling functions that wrap the corresponding C++ pointers
	// in a QScriptValue.
	registerOvitoObjectType<RefTarget>();
	registerOvitoObjectType<DataSet>();
	registerOvitoObjectType<AnimationSettings>();
	registerOvitoObjectType<ViewportConfiguration>();
	registerOvitoObjectType<Viewport>();
	registerOvitoObjectType<RenderSettings>();
	registerOvitoObjectType<SceneRenderer>();
	registerOvitoObjectType<SceneNode>();
	registerOvitoObjectType<ObjectNode>();
	registerOvitoObjectType<GroupNode>();
	registerOvitoObjectType<SceneRoot>();
	registerOvitoObjectType<SceneObject>();
	registerOvitoObjectType<PipelineObject>();
	registerOvitoObjectType<ModifierApplication>();
	registerOvitoObjectType<Modifier>();
	registerOvitoObjectType<SelectionSet>();
	registerOvitoObjectType<Controller>();
	registerOvitoObjectType<IntegerController>();
	registerOvitoObjectType<FloatController>();
	registerOvitoObjectType<VectorController>();
	registerOvitoObjectType<BooleanController>();
	registerOvitoObjectType<PositionController>();
	registerOvitoObjectType<RotationController>();
	registerOvitoObjectType<ScalingController>();
	registerOvitoObjectType<TransformationController>();
	registerOvitoObjectType<LookAtController>();
	registerOvitoObjectType<LinkedFileObject>();
	registerOvitoObjectType<LinkedFileImporter>();

	// Create script binding objects and let them set up the script environment.
	for(const OvitoObjectType* bindingClass : PluginManager::instance().listClasses(ScriptBinding::OOType)) {
		OORef<ScriptBinding> binding = static_object_cast<ScriptBinding>(bindingClass->createInstance(nullptr));
		_bindings.push_back(binding);
		binding->setupBinding(*this);
	}

	// Register constructor functions for all installed RefTarget-derived classes.
	// This will allow scripts to create new objects.
	for(const OvitoObjectType* objectClass : PluginManager::instance().listClasses(RefTarget::OOType)) {
		QScriptValue ctor = newFunction(&ScriptEngine::objectConstructor, const_cast<OvitoObjectType*>(objectClass));
		QScriptValue metaObj = newQMetaObject(objectClass->qtMetaObject(), ctor);
		globalObject().setProperty(objectClass->name(), metaObj);
	}

	// Set up a prototype for the RefTarget class, which provides the toString() method.
	QScriptValue refTargetProto = newObject();
	refTargetProto.setProperty("toString", newFunction(&RefTarget_toString));
	refTargetProto.setPrototype(defaultPrototype(qMetaTypeId<QObject*>()));
	setDefaultPrototype(qMetaTypeId<RefTarget*>(), refTargetProto);

	// Set up the prototype chain such that it reflects the C++ class hierarchy.
	for(auto entry = _registeredObjectTypes.begin(); entry != _registeredObjectTypes.end(); ++entry) {
		const OvitoObjectType* objectClass = entry.key();
		int metaTypeId = entry.value();
		QScriptValue prototype = defaultPrototype(metaTypeId);
		const OvitoObjectType* baseClass = objectClass->superClass();
		while(baseClass) {
			auto baseEntry = _registeredObjectTypes.find(baseClass);
			if(baseEntry != _registeredObjectTypes.end()) {
				QScriptValue basePrototype = defaultPrototype(baseEntry.value());
				if(basePrototype.isValid()) {
					if(prototype.isValid())
						prototype.setPrototype(basePrototype);
					else
						setDefaultPrototype(metaTypeId, basePrototype);
					break;
				}
			}
			baseClass = baseClass->superClass();
		}
	}
}

/******************************************************************************
* Wraps an OvitoObject pointer in a QScriptValue.
*
* The "data" property of the QScriptValue stores an additional
* OORef smart pointer to the OvitoObject to keep it alive as long as the
* QScriptValue exists.
******************************************************************************/
QScriptValue ScriptEngine::wrapOvitoObject(OvitoObject* obj)
{
	// Create script value that stores the raw pointer to the OvitoObject.
	QScriptValue retval = toScriptValue(obj);

	if(obj) {
		// Store an additional OORef<OvitoObject> smart pointer in the 'data' field of the first QScriptValue.
		// It will be deleted together with the raw pointer when the script value is garbage-collected.
		// The OORef smart pointer is encapsulated in a QVariant such that in can be stored in a QScriptValue.
		retval.setData(newVariant(QVariant::fromValue(OORef<OvitoObject>(obj))));
	}

	return retval;
}

/******************************************************************************
* Constructor function for OVITO objects.
******************************************************************************/
QScriptValue ScriptEngine::objectConstructor(QScriptContext* context, QScriptEngine* _engine, void* _objectClass)
{
	try {
		const OvitoObjectType* objectClass = static_cast<const OvitoObjectType*>(_objectClass);
		ScriptEngine* engine = static_cast<ScriptEngine*>(_engine);

		// Create instance of the OvitoObject class.
		QScriptValue object = engine->wrapOvitoObject(objectClass->createInstance(engine->dataset()));

		// The caller of the constructor function can pass a dictionary object,
		// which contains initial values for parameters of the newly created object.
		if(context->argumentCount() == 1) {
			// Iterate over all properties of the dictionary object and
			// copy them over to the newly created object.
			QScriptValueIterator it(context->argument(0));
			while(it.hasNext()) {
				it.next();
				object.setProperty(it.name(), it.value());
			}
		}

		return object;
	}
	catch(const Exception& ex) {
		return context->throwError(ex.message());
	}
	catch(const std::exception& ex) {
		return context->throwError(ex.what());
	}
	catch(...) {
		return context->throwError(tr("Uncaught C++ exception"));
	}
}

/******************************************************************************
* Create a script function object from a C++ member function.
******************************************************************************/
QScriptValue ScriptEngine::newStdFunction(const std::function<QScriptValue(QScriptContext*,ScriptEngine*)>& function, int numberOfParameters)
{
	// Create the function script value.
	QScriptValue sv = newFunction(&scriptFunctionHandler, numberOfParameters);

	// Attach the std:function to the script value so that it can be looked up again
	// once the script invokes the function. Wrap it in a QVariant so it can be stored in the 'data' field.
	sv.setData(newVariant(QVariant::fromValue(function)));

	return sv;
}

/******************************************************************************
* Dispatches script calls to C++ functions that have been registered
* with newStdFunction().
******************************************************************************/
QScriptValue ScriptEngine::scriptFunctionHandler(QScriptContext* context, QScriptEngine* engine)
{
	OVITO_ASSERT(context->callee().data().isVariant());
	OVITO_ASSERT(qobject_cast<ScriptEngine*>(engine) != nullptr);

	// Extract the stored std::function, which is the target of the function call.
	QVariant variant = context->callee().data().toVariant();
	if(variant.canConvert<std::function<QScriptValue(QScriptContext*,ScriptEngine*)>>() == false)
		return context->throwError("Could not extract std::function from callee object. Perhaps the QScriptValue 'data' field has been overwritten.");

	try {
		// Call C++ function object.
		return variant.value<std::function<QScriptValue(QScriptContext*,ScriptEngine*)>>()(context, static_cast<ScriptEngine*>(engine));
	}
	catch(const Exception& ex) {
		return context->throwError(ex.message());
	}
	catch(const std::exception& ex) {
		return context->throwError(ex.what());
	}
	catch(...) {
		return context->throwError(tr("Uncaught C++ exception"));
	}
}

/******************************************************************************
* Creates a string representation of a RefTarget script value.
******************************************************************************/
QScriptValue ScriptEngine::RefTarget_toString(QScriptContext* context, QScriptEngine* engine)
{
	QObject* qobj = context->thisObject().toQObject();
	if(!qobj)
		return engine->toScriptValue(QStringLiteral("null"));
	RefTarget* target = qobject_cast<RefTarget*>(qobj);
	if(!target)
		return engine->toScriptValue(QString(qobj->metaObject()->className()));
	return engine->toScriptValue(QString("%1(%2)").arg(target->getOOType().name()).arg(target->objectTitle()));
}

};
