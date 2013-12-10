#ifndef __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H
#define __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H

#include <plugins/scripting/Scripting.h>

#include <QtScript>

#include <core/viewport/Viewport.h>
#include <core/gui/actions/ActionManager.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetContainer.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief QObject based wrapper around OORef.
 */
template<typename T>
class ScriptRef : public QObject {
public:
	explicit ScriptRef(const OORef<T>& ref) : ref_(ref) {}
	ScriptRef(const ScriptRef<T>& ref) : ref_(ref.ref_) {}
	OORef<T> getReference() { return ref_; }

private:
	OORef<T> ref_;
};


/**
 * \brief Helper to wrap OORef'd object in QScriptValue.
 *
 * The "data" property of the script value will contain an OORef that
 * will be deleted when the script no longer needs it.  This
 * guarantees correct memory management for reference counted Ovito
 * objects.
 */
template<typename T>
QScriptValue wrapOORef(OORef<T> ptr, QScriptEngine* engine) {
	// Create script value that will never be deleted, because
	// memory is managed by the smart pointer "ptr".
	QScriptValue retval = engine->newQObject(ptr.get(),
											 QScriptEngine::QtOwnership);
	// Add the smart pointer in 'data', it will be deleted when
	// the script doesn't need it anymore, so that we don't leak
	// memory.
	QScriptValue ref = engine->newQObject(new ScriptRef<T>(ptr),
										  QScriptEngine::ScriptOwnership);
	retval.setData(ref);
	// Done.
	return retval;
}


/**
 * \brief Scripting interface to the viewports.
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
 * Needs a global DataSetContainer.
 */
QScriptEngine* prepareEngine(DataSet* dataSet, QObject* parent = 0);


}



#endif

// Local variables:
// mode: c++
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
