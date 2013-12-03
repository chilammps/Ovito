#include "ScriptBindings.h"

#include <vector>

#include <base/linalg/Point3.h>
#include <base/linalg/Vector3.h>
#include <base/utilities/FloatType.h>
#include <core/dataset/DataSetManager.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/standard/StandardSceneRenderer.h>
#include <core/plugins/PluginManager.h>

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

void ViewportBinding::setActive() const {
	ViewportManager::instance().setActiveViewport(getViewport());
}


void ViewportBinding::render(const QString& filename) const {
	// TODO: exception handling!

	// Prepare settings.
	RenderSettings settings;
	settings.setRendererClass(&StandardSceneRenderer::OOType);
	settings.setRenderingRangeType(RenderSettings::CURRENT_FRAME);
	settings.setOutputImageWidth(800);
	settings.setOutputImageHeight(600);
	settings.setImageFilename(filename);
	settings.setSaveToFile(true);
	settings.setSkipExistingImages(false);

	// Prepare framebuffer.
	FrameBufferWindow* frameBufferWindow = nullptr;
	QSharedPointer<FrameBuffer> frameBuffer;
	if(Application::instance().guiMode()) {
		frameBufferWindow = MainWindow::instance().frameBufferWindow();
		frameBuffer = frameBufferWindow->frameBuffer();
	}
	if(!frameBuffer)
		frameBuffer.reset(new FrameBuffer(settings.outputImageWidth(),
										  settings.outputImageHeight()));

	// Render.
	DataSetManager::instance().currentSet()->renderScene(&settings,
														 getViewport(),
														 frameBuffer,
														 frameBufferWindow);
}


DataSetBinding::DataSetBinding(SelectionSet* dataSet, QObject* parent)
	: QObject(parent), dataSet_(dataSet),
	  object_(static_cast<ObjectNode*>(dataSet->firstNode()))
	  // TODO ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ good way to do it??????   
{}

#include <iostream>
void DataSetBinding::appendModifier(const QScriptValue& modifier) {
	QScriptEngine* engine = modifier.engine();
	QScriptContext* context = engine->currentContext();
	try {
		QObject* data = modifier.data().toQObject();
		if (data == nullptr) {
			context->throwError("Not a valid modifier");
			return;
		}
		// TODO: this cast is not necessarily safe.   
		ScriptRef<Modifier> modWrapper = *dynamic_cast<ScriptRef<Modifier>*>(data);
		OORef<Modifier> mod = modWrapper.getReference();
		// TODO: object_ doesn't take ownership of the OORef for some reason!?    
		object_->applyModifier(mod);
	} catch (const Exception& e) {
		context->throwError(QString("Not a valid modifier (error: ") + e.what() + ")");
	}
}


QScriptValue pwd(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 0)
		return context->throwError("This function takes no arguments.");
	// Return.
	return QScriptValue(QDir::currentPath());
}

QScriptValue cd(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 1)
		return context->throwError("This function takes one argument.");
	const QString newdir = context->argument(0).toString();

	// Change dir.
	const bool success = QDir::setCurrent(newdir);
	if (success)
		return QScriptValue(QDir::currentPath());
	else
		return context->throwError("Can't change directory to " + newdir);
}

QScriptValue loadFile(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 1)
		return context->throwError("This function takes one argument.");
	const QString path = context->argument(0).toString();

	// Load it.
	try {
		DataSetManager::instance().importFile(QUrl::fromLocalFile(path),
											  nullptr,
											  FileImporter::AddToScene);
	} catch (const Exception& e) {
		return context->throwError(e.what());
	}

	// Return wrapper around the dataset representing the current file.
	// TODO: this is racy, perhaps? Is the new file guaranteed to be     
 	// the active selection?                                             
	SelectionSet* dataSet = DataSetManager::instance().currentSelection();
	return engine->newQObject(new DataSetBinding(dataSet),
							  QScriptEngine::ScriptOwnership);
}





QScriptValue listModifiers(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 0)
		return context->throwError("This function takes no arguments.");

	// Build array of modifier names.
	std::vector<QString> names;
	Q_FOREACH(const OvitoObjectType* clazz,
			  PluginManager::instance().listClasses(Modifier::OOType)) {
		names.push_back(clazz->name());
	}
	QScriptValue retval = engine->newArray(names.size());
	for (int i = 0; i != names.size(); ++i)
		retval.setProperty(i, QScriptValue(names[i]));
	return retval;
}

QScriptValue modifier(QScriptContext* context, QScriptEngine* engine) {
	// Process arguments.
	if (context->argumentCount() != 1)
		return context->throwError("This function takes one argument.");
	QString name = context->argument(0).toString();

	// Search modifier.
	const OvitoObjectType* searchResultClass = nullptr;
	Q_FOREACH(const OvitoObjectType* clazz,
			  PluginManager::instance().listClasses(Modifier::OOType)) {
		if (clazz->name() == name) {
			searchResultClass = clazz;
			break;
		}
	}

	// Return.
	if (searchResultClass == nullptr)
		return context->throwError("Modifier " + name + " not found.");
	else {
		// Get instance of modifier.
		OORef<Modifier> ptr =
			static_object_cast<Modifier>(searchResultClass->createInstance());
		return wrapOORef<Modifier>(ptr, engine);
	}
}

QScriptEngine* prepareEngine(QObject* parent) {
	// Set up engine.
	QScriptEngine* engine = new QScriptEngine(parent);

	// Set up namespace. ///////////////////////////////////////////////

	// Active viewport
	QScriptValue activeViewport =
		engine->newQObject(new ActiveViewportBinding(),
						   QScriptEngine::ScriptOwnership);
	engine->globalObject().setProperty("activeViewport", activeViewport);

	// All viewports.
	const QVector<Viewport*>& allViewports =
		ViewportManager::instance().viewports();
	QScriptValue viewport = engine->newArray(allViewports.size());
	for (int i = 0; i != allViewports.size(); ++i)
		viewport.setProperty(i, engine->newQObject(new ViewportBinding(allViewports[i]),
												   QScriptEngine::ScriptOwnership));
	engine->globalObject().setProperty("viewport", viewport);

	// The global Ovito object.
	QScriptValue ovito = engine->newQObject(new OvitoBinding(),
										   QScriptEngine::ScriptOwnership);
	engine->globalObject().setProperty("ovito", ovito);

	// pwd function.
	engine->globalObject().setProperty("pwd",
									   engine->newFunction(pwd, 0));

	// cd function.
	engine->globalObject().setProperty("cd",
									   engine->newFunction(cd, 0));

	// loadFile function.
	engine->globalObject().setProperty("loadFile",
									   engine->newFunction(loadFile, 0));

	// listModifiers function.
	engine->globalObject().setProperty("listModifiers",
									   engine->newFunction(listModifiers, 0));

	// modifiers function.
	engine->globalObject().setProperty("modifier",
									   engine->newFunction(modifier, 1));

	// Done.
	return engine;
}


}

// Local variables:
// indent-tabs-mode: t
// tab-width: 4
// c-basic-offset: 4
// End:
