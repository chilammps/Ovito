#include "ScriptBindings.h"

#include <base/linalg/Point3.h>
#include <base/linalg/Vector3.h>
#include <base/utilities/FloatType.h>
#include <core/dataset/DataSetManager.h>
#include <core/dataset/importexport/FileImporter.h>

namespace Scripting {

using namespace Ovito;

ViewportBinding::ViewportBinding(Viewport* viewport, QObject* parent)
  : QObject(parent), viewport_(viewport)
{}

void ViewportBinding::perspective(double cam_pos_x, double cam_pos_y, double cam_pos_z,
                                  double cam_dir_x, double cam_dir_y, double cam_dir_z,
                                  double cam_angle) {
  Viewport* vp = getViewport();
  vp->setViewType(Viewport::VIEW_PERSPECTIVE);
  vp->setCameraPosition(Point3(FloatType(cam_pos_x),
                               FloatType(cam_pos_y),
                               FloatType(cam_pos_z)));
  vp->setCameraDirection(Vector3(FloatType(cam_dir_x),
                                 FloatType(cam_dir_y),
                                 FloatType(cam_dir_z)));
  vp->setFieldOfView(cam_angle);
}

void ViewportBinding::ortho(double cam_pos_x, double cam_pos_y, double cam_pos_z,
                            double cam_dir_x, double cam_dir_y, double cam_dir_z,
                            double fov) {
  Viewport* vp = getViewport();
  vp->setViewType(Viewport::VIEW_ORTHO);
  vp->setCameraPosition(Point3(FloatType(cam_pos_x),
                               FloatType(cam_pos_y),
                               FloatType(cam_pos_z)));
  vp->setCameraDirection(Vector3(FloatType(cam_dir_x),
                                 FloatType(cam_dir_y),
                                 FloatType(cam_dir_z)));
  vp->setFieldOfView(fov);
}


void ViewportBinding::maximize() {
	Viewport* vp = getViewport();
	ViewportManager::instance().setMaximizedViewport(vp);
	ViewportManager::instance().setActiveViewport(vp);
}

void ViewportBinding::restore() {
	ViewportManager::instance().setMaximizedViewport(nullptr);
}




void OvitoBinding::loadFile(const QString& path) {
	// TODO: exception handling!
	DataSetManager::instance().importFile(QUrl::fromLocalFile(path),
										  nullptr,
										  FileImporter::AddToScene);
}

QString OvitoBinding::pwd() const {
	return QDir::currentPath();
}

void OvitoBinding::cd(QString newdir) {
	const bool success = QDir::setCurrent(newdir);
	if (!success)
		// TODO: JS exception
		throw Exception("Could not cd to " + newdir);
}


}

// Local variables:
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
