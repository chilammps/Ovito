#ifndef __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H
#define __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H

#include <plugins/scripting/Scripting.h>

#include <QtScript>

#include <core/viewport/Viewport.h>
#include <core/gui/actions/ActionManager.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/dataset/DataSet.h>

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
private:
	Viewport* viewport_;
	QScriptEngine* engine_;

	Q_OBJECT
};


class ActiveViewportBinding : public ViewportBinding {
public:
	ActiveViewportBinding(QScriptEngine* engine, QObject* parent = 0)
		: ViewportBinding(nullptr, engine, parent)
	{}

private:
	Viewport* getViewport() const override {
		return ViewportManager::instance().activeViewport();
	}
};


class DataSetBinding : public QObject {
public:
	DataSetBinding(SelectionSet* dataSet, QObject* parent = 0);

public Q_SLOTS:
	void appendModifier(const QScriptValue& modifier);

private:
	SelectionSet* dataSet_;
	ObjectNode* object_;

	Q_OBJECT
};


/**
 * \brief Main interface.
 */
class OvitoBinding : public QObject {
public:
	OvitoBinding(QObject* parent = 0) : QObject(parent) {}

public Q_SLOTS:
	/**
	 * \brief Quit OVITO without asking.
	 * \todo It's still asking.
	 */
	void quit() {
		ActionManager::instance().invokeAction(ACTION_QUIT);
	}
private:
	Q_OBJECT
};

/// Return current working directory.
QScriptValue pwd(QScriptContext* context, QScriptEngine* engine);

/// Change current working directory and return it.
QScriptValue cd(QScriptContext* context, QScriptEngine* engine);

/// Import file.
QScriptValue loadFile(QScriptContext* context, QScriptEngine* engine);

QScriptValue listModifiers(QScriptContext* context, QScriptEngine* engine);

QScriptValue modifier(QScriptContext* context, QScriptEngine* engine);



/**
 * \brief Create a script engine that is already filled with global objects.
 */
QScriptEngine* prepareEngine(QObject* parent = 0);


}



#endif

// Local variables:
// mode: c++
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
