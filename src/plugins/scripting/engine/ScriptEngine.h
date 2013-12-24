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

#ifndef __OVITO_SCRIPTING_ENGINE_H
#define __OVITO_SCRIPTING_ENGINE_H

#include <plugins/scripting/Scripting.h>
#include <core/dataset/DataSet.h>
#include "ScriptBinding.h"

namespace Scripting {

using namespace Ovito;

/**
 * \brief A scripting engine that provides bindings to OVITO's C++ classes.
 */
class OVITO_SCRIPTING_EXPORT ScriptEngine : public QScriptEngine
{
public:

	/// \brief Initializes the scripting engine and sets up the environment.
	/// \param dataset The engine will execute scripts in the context of this dataset.
	/// \param parent The owner of this QObject.
	ScriptEngine(DataSet* dataset, QObject* parent = nullptr);

	/// \break Returns the dataset that provides the context for the script.
	DataSet* dataset() const { return _dataset.get(); }

	/// \brief Create a script function object from a C++ std::function.
	///
	/// This method behaves like QScriptEngine::newFunction(), which accepts only non-member function pointers.
	QScriptValue newStdFunction(const std::function<QScriptValue(QScriptContext*,ScriptEngine*)>& function, int numberOfParameters = 0);

	/// \brief Wraps an OvitoObject pointer in a QScriptValue.
	QScriptValue wrapOvitoObject(OvitoObject* obj);

	/// \brief Wraps an OvitoObject smart pointer in a QScriptValue.
	QScriptValue wrapOvitoObject(const OORef<OvitoObject>& obj) {
		return wrapOvitoObject(obj.get());
	}

	/// \brief Make an Ovito object class (which is derived from RefTarget) available to the script.
	template<typename T>
	void registerOvitoObjectType() {
		// Construct the name under which to register the pointer type.
		QByteArray typeName(T::OOType.name().toLatin1());
		typeName.append('*');

		// Register the object pointer type with the Qt meta type system.
		int id = qRegisterMetaType<T*>(typeName.constData());
		_registeredObjectTypes.insert(&T::OOType, id);

		// Register type pointer type of the script system. Make sure the wrapOvitoObject() function
		// is used to convert an OvitoObject pointer to a script value.
		qScriptRegisterMetaType<T*>(this, &ScriptEngine::objectPointerToScriptValue<T>, &ScriptEngine::scriptValueToObjectPointer<T>);
	}

private:

	/// \brief Dispatches script function calls to C++ functions that have been registered with newStdFunction().
	static QScriptValue scriptFunctionHandler(QScriptContext* context, QScriptEngine* engine);

	/// \brief Constructor function for OVITO objects, which can be invoked by scripts.
	static QScriptValue objectConstructor(QScriptContext* context, QScriptEngine* engine, void* objectClass);

	/// \brief Converts a pointer to an OvitoObject-derived class to a script value.
	template<typename T>
	static QScriptValue objectPointerToScriptValue(QScriptEngine* engine, T* const& obj) {
		return static_cast<ScriptEngine*>(engine)->wrapOvitoObject(obj);
	}

	/// \brief Converts a script value to an OvitoObject-derived class pointer.
	template<typename T>
	static void scriptValueToObjectPointer(const QScriptValue& sv, T*& obj) {
		obj = dynamic_object_cast<T>(sv.toQObject());
#ifdef OVITO_DEBUG
		if(obj && sv.data().toVariant().value<OORef<OvitoObject>>().get() != obj)
			qDebug() << "WARNING: Script value storing a" << obj->getOOType().name() << "does not carry a reference counting smart pointer.";
#endif
	}

	/// Creates a string representation of a RefTarget script value.
	static QScriptValue RefTarget_toString(QScriptContext* context, QScriptEngine* engine);

private:

	/// The dataset that provides the context for the script execution.
	OORef<DataSet> _dataset;

	/// The binding objects attached to the engine.
	QVector<OORef<ScriptBinding>> _bindings;

	/// List of registered OVITO object pointer types
	/// and their Qt meta type IDs.
	QMap<const OvitoObjectType*, int> _registeredObjectTypes;

	Q_OBJECT
};

};	// End of namespace


// Register this function type with the Qt system so that it can be stored in a QVariant.
// This is needed by the implementation of the ScriptEngine::newStdFunction() method.
Q_DECLARE_METATYPE(std::function<QScriptValue(QScriptContext*,Scripting::ScriptEngine*)>);

#endif
