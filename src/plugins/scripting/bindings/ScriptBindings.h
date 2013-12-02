#ifndef __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H
#define __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H

#include <plugins/scripting/Scripting.h>

#include <QtScript>

#include <core/viewport/Viewport.h>
#include <core/gui/actions/ActionManager.h>
#include <core/viewport/ViewportManager.h>
#include <core/scene/pipeline/Modifier.h>

namespace Scripting {

using namespace Ovito;

/**
 * \brief QObject based wrapper around OORef.
 *
 * Will expose all properties of the pointee.
 */
template<typename T>
class ScriptRef : public QObject {
public:
	ScriptRef(OORef<T> ref) : ref_(ref) {
		// Export all properties of the pointee.
		//GEHT NICHT
	}

private:
	OORef<T> ref_;
};

/**
 * \brief Scripting interface to the viewports.
 */
class ViewportBinding : public QObject {
public:
	ViewportBinding(Viewport* viewport, QObject* parent = 0);

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
	void render(const QString& filename) const;

protected:
	virtual Viewport* getViewport() const { return viewport_; };
private:
	Viewport* viewport_;

	Q_OBJECT
};


class ActiveViewportBinding : public ViewportBinding {
public:
	ActiveViewportBinding(QObject* parent = 0)
		: ViewportBinding(nullptr, parent)
	{}

private:
	Viewport* getViewport() const override {
		return ViewportManager::instance().activeViewport();
	}
};


/**
 * \brief Main interface.
 */
class OvitoBinding : public QObject {
public:
	OvitoBinding(QObject* parent = 0) : QObject(parent) {}

public Q_SLOTS:
	/**
	 * \brief Import file.
	 *
	 * \todo Return a data object wrapper that can be used to add
	 * modifiers etc.
	 *
	 * \todo Optionally allow giving the file type.
	 */
	void loadFile(const QString& path);

	/**
	 * \brief Return current working directory.
	 */
	QString pwd() const;

	/**
	 * \brief Set the working directory.
	 */
	void cd(QString newdir);

	/**
	 * \brief Quit OVITO without asking.
	 * \todo It's still asking.
	 */
	void quit() {
		ActionManager::instance().invokeAction(ACTION_QUIT);
	}

	/**
	 * \brief Create new modifier.
	 * \todo Take an optional argument that is a JavaScript object, to
	 *       set the properties of the modifier before returning it.
	 */
	Modifier* modifierFactory(const QString& name) const;

private:
	Q_OBJECT
};


QScriptValue listModifiers(QScriptContext* context, QScriptEngine* engine);

QScriptValue modifier(QScriptContext* context, QScriptEngine* engine);


}



#endif

// Local variables:
// mode: c++
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
