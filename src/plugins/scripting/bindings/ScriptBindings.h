#ifndef __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H
#define __OVITO_SCRIPTING_BINDINGS_SCRIPTBINDINGS_H

#include <plugins/scripting/Scripting.h>
#include <core/viewport/Viewport.h>
#include <core/gui/actions/ActionManager.h>
#include <core/viewport/ViewportManager.h>

namespace Scripting {

using namespace Ovito;

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

protected:
	virtual Viewport* getViewport() { return viewport_; };
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
	Viewport* getViewport() override {
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
	 * \biref Set the working directory.
	 */
	void cd(QString newdir);

	void quit() {
		ActionManager::instance().invokeAction(ACTION_QUIT);
	}
private:
	Q_OBJECT
};


}



#endif

// Local variables:
// mode: c++
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
