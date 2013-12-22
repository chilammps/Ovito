///////////////////////////////////////////////////////////////////////////////
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  Copyright (C) 2013 Tobias Brink
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H
#define __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H

#include <plugins/scripting/Scripting.h>
#include <core/viewport/ViewportConfiguration.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief Helper to wrap OORef'd object in a QScriptValue.
 *
 * The "data" property of the QScriptValue stores an additional
 * OORef smart pointer to the OvitoObject to keep it alive as long as the
 * QScriptValue exists.
 */
template<typename T>
QScriptValue wrapOORef(const OORef<T>& ptr, QScriptEngine* engine) {
	// Create script value that stores the raw pointer to the Ovito object.
	QScriptValue retval = engine->newQObject(ptr.get(),
											 QScriptEngine::QtOwnership);

	// Store an additional OORef<OvitoObject> smart pointer in the 'data' field of the first QScriptValue.
	// It will be deleted together with the raw pointer when the script value is garbage-collected.
	// The OORef smart pointer is encapsulated in a QVariant to make it acceptable by the QScriptValue class.
	retval.setData(engine->newVariant(qVariantFromValue(static_object_cast<OvitoObject>(ptr))));

	return retval;
}


/**
 * \brief Scripting interface to the viewports.
 *
 * \todo Viewports can be created and deleted, we cannot just store
 * the initial viewports in an array and be done!  
 * \todo Pointer to ViewportConfiguration and Viewport should be OORef!  
 * \todo Viewport has a method to get DataSet, do not store DataSet!  
 */
class ViewportBinding : public QObject {
public:
	ViewportBinding(Viewport* viewport,
					QScriptEngine* engine,
					DataSet* dataSet,
					QObject* parent = 0);

public Q_SLOTS:
	void perspective(double cam_pos_x, double cam_pos_y, double cam_pos_z,
					 double cam_dir_x, double cam_dir_y, double cam_dir_z,
					 double cam_angle);
	void ortho(double cam_pos_x, double cam_pos_y, double cam_pos_z,
			   double cam_dir_x, double cam_dir_y, double cam_dir_z,
			   double fov);

	/**
	 * \brief Maximize this viewport.
	 */
	void maximize();

	/**
	 * \brief Restore original viewport sizes (un-maximize).
	 */
	void restore();

	/**
	 * \brief Set as active viewport.
	 */
	void setActive() const;

	/**
	 * \brief Render this viewport.
	 *
	 * \todo Check return value of renderScene and either return
	 * boolean or throw error
	 */
	void render(const QString& filename,
				const QScriptValue& options = QScriptValue::UndefinedValue
				) const;

protected:
	virtual Viewport* getViewport() const { return viewport_; };
	ViewportConfiguration* viewportConf_;
	DataSet* dataSet_;
private:
	Viewport* viewport_;
	QScriptEngine* engine_;

	Q_OBJECT
};


/**
 * \brief Scripting interface to the active viewport.
 *
 * This is similar to ViewportBinding but always references the ative
 * viewport.
 *
 * \todo Always take ViewportConfiguration 
 */
class ActiveViewportBinding : public ViewportBinding {
public:
	ActiveViewportBinding(QScriptEngine* engine,
						  DataSet* dataSet,
						  QObject* parent = 0)
		: ViewportBinding(nullptr, engine, dataSet, parent)
	{}

private:
	Viewport* getViewport() const override {
		return viewportConf_->activeViewport();
	}

	Q_OBJECT
};


/**
 * \brief Wrapper for an object in a DataSet.
 *
 * \ŧodo better name
 *
 * \todo use RefTargetListener so that we can see when the pointee is
 * deleted, in this case this object is invalid (throw exception on
 * use or something).
 *
 * \todo close/delete method
 *
 * \todo Method to access Pipeline
 */
class DataSetBinding : public QObject {
public:
	DataSetBinding(ObjectNode* object, QObject* parent = 0);

public Q_SLOTS:
	void appendModifier(const QScriptValue& modifier);

private:
	ObjectNode* object_;

	Q_OBJECT
};


/// Quit Ovito.
inline
QScriptValue quit(QScriptContext* context, QScriptEngine* engine) {   
	throw Exception("Not implemented");
}


/// Return current working directory.
QScriptValue pwd(QScriptContext* context, QScriptEngine* engine);


/// Change current working directory and return it.
QScriptValue cd(QScriptContext* context, QScriptEngine* engine);


/// Import file.
QScriptValue loadFile(QScriptContext* context, QScriptEngine* engine);


/// Return array of names of available modifiers.
QScriptValue listModifiers(QScriptContext* context, QScriptEngine* engine);


/// Create a modifier.
QScriptValue modifier(QScriptContext* context, QScriptEngine* engine);


/**
 * \brief Create a script engine that is already filled with global objects.
 *
 * \param dataSet The DataSet which is the context for this script
 * engine. Keeps a hard reference (OORef) in the engine.
 *
 * \param parent QObject memory management of the engine.
 */
QScriptEngine* prepareEngine(DataSet* dataSet, QObject* parent = 0);


};	// End of namespace

#endif

// Local variables:
// mode: c++
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
