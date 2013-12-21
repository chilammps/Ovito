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

#include "ScriptBindings.h"

#include <vector>

#include <base/linalg/Point3.h>
#include <base/linalg/Vector3.h>
#include <base/utilities/FloatType.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/gui/app/Application.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/widgets/rendering/FrameBufferWindow.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/standard/StandardSceneRenderer.h>
#include <core/plugins/PluginManager.h>
#include <core/scene/SelectionSet.h>
#include <plugins/particles/modifier/coloring/ColorCodingModifier.h>

namespace Scripting {

using namespace Ovito;


// Wrapping DataSet ////////////////////////////////////////////////////

DataSet* unwrapGlobalDataSet(QScriptEngine* engine) {
	QVariant ds = engine->property("_ovito_dataset");
	if (ds.canConvert< OORef<OvitoObject> >()) {
		DataSet* data = dynamic_object_cast<DataSet>(ds.value< OORef<OvitoObject> >()).get();
		if (data == nullptr)
			throw std::runtime_error("global DataSet is not a DataSet (but is an OvitoObject)!");
		return data;
	} else
		throw std::runtime_error("global DataSet is not a DataSet!");
}


// ViewportBinding /////////////////////////////////////////////////////

ViewportBinding::ViewportBinding(Viewport* viewport, QScriptEngine* engine,
								 DataSet* dataSet,
								 QObject* parent)
	: QObject(parent),
	  viewportConf_(dataSet->viewportConfig()),
	  dataSet_(dataSet),
	  viewport_(viewport),
	  engine_(engine)
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
	viewportConf_->setMaximizedViewport(vp);
	viewportConf_->setActiveViewport(vp);
}

void ViewportBinding::restore() {
	viewportConf_->setMaximizedViewport(nullptr);
}

void ViewportBinding::setActive() const {
	viewportConf_->setActiveViewport(getViewport());
}


void ViewportBinding::render(const QString& filename,
							 const QScriptValue& options) const {
	// Clone current render settings of the data set.
	CloneHelper cloner;
	OORef<RenderSettings> settings = cloner.cloneObject(dataSet_->renderSettings(), true);
	// Output settings.
	settings->setImageFilename(filename);
	settings->setSaveToFile(true);
	/* TODO: override these from 'options'       
	settings->setRendererClass(&StandardSceneRenderer::OOType);
	settings->setRenderingRangeType(RenderSettings::CURRENT_FRAME);
	settings->setOutputImageWidth(800);
	settings->setOutputImageHeight(600);
	settings->setSkipExistingImages(false);
	*/

	try {
		// Prepare framebuffer.
		FrameBufferWindow* frameBufferWindow = nullptr; // TODO: remove this variable
		QSharedPointer<FrameBuffer> frameBuffer;
		/* not needed.
		if(Application::instance().guiMode()) {
			MainWindow* mainWindow = dataSet_->mainWindow();
			frameBufferWindow = mainWindow->frameBufferWindow();
			frameBuffer = frameBufferWindow->frameBuffer();
		}
		*/
		if(!frameBuffer)
			frameBuffer.reset(new FrameBuffer(settings->outputImageWidth(),
											  settings->outputImageHeight()));

		// Render.
		dataSet_->renderScene(settings.get(),
							  getViewport(),
							  frameBuffer,
							  frameBufferWindow);
	} catch (const Exception& e) {
		QScriptContext* context = engine_->currentContext();
		context->throwError(QString("Exception while rendering: ") + e.what());
	}
}


// DataSetBinding //////////////////////////////////////////////////////

DataSetBinding::DataSetBinding(ObjectNode* object, QObject* parent)
	: QObject(parent), object_(object)
{}

void DataSetBinding::appendModifier(const QScriptValue& modifier) {
	QScriptEngine* engine = modifier.engine();
	QScriptContext* context = engine->currentContext();
	try {
		Modifier* mod = qobject_cast<Modifier*>(modifier.toQObject());
		if (mod == nullptr) {
			context->throwError("Not a valid modifier");
			return;
		}
		object_->applyModifier(mod);
	} catch (const Exception& e) {
		context->throwError(QString("Not a valid modifier (error: ") + e.what() + ")");
	}
}


// Helper functions ////////////////////////////////////////////////////

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
	DataSet* dataSet = unwrapGlobalDataSet(engine);
	DataSetContainer* container = dataSet->container();
	try {
		// TODO: check return value     
		// TODO: guess remote URL  
		container->importFile(QUrl::fromLocalFile(path),
							  nullptr,
							  FileImporter::AddToScene); // <- TODO configurable
	} catch (const Exception& e) {
		return context->throwError(e.what());
	}

	// Return wrapper around the dataset representing the current file.
	ObjectNode* root = static_cast<ObjectNode*>(dataSet->selection()->firstNode());
	return engine->newQObject(new DataSetBinding(root),
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
		// Get DataSet.
		DataSet* dataSet = unwrapGlobalDataSet(engine);
		// Get instance of modifier.
		OORef<Modifier> ptr =
			static_object_cast<Modifier>(searchResultClass->createInstance(dataSet));
		return wrapOORef<Modifier>(ptr, engine);
	}
}


// Helpers to convert values between JS and C++. ///////////////////////
// TODO: memory management of the returned QScriptValue's!  

QScriptValue fromFloatType(QScriptEngine *engine, const FloatType& x) {
	double xx = x;
	return QScriptValue(xx);
}

void toFloatType(const QScriptValue& obj, FloatType& x) {
	x = obj.toNumber();
}



QScriptValue fromVector3(QScriptEngine *engine, const Vector3& x) {
	QScriptValue retval = engine->newArray(3);
	retval.setProperty(0, x.x());
	retval.setProperty(1, x.y());
	retval.setProperty(2, x.z());
	return retval;
}

void toVector3(const QScriptValue& obj, Vector3& x) {
	QScriptEngine* engine = obj.engine();
	QScriptContext* context = engine->currentContext();
	if (!obj.isArray()) {
		context->throwError("object must be an array");
		return;
	}
	const unsigned length = obj.property("length").toInteger();
	if (length != 3) {
		context->throwError("array must have a length of 3");
		return;
	}
	x.x() = obj.property(0).toNumber();
	x.y() = obj.property(1).toNumber();
	x.z() = obj.property(2).toNumber();
}




QScriptValue fromColor(QScriptEngine *engine, const Color& x) {
	QScriptValue retval = engine->newArray(3);
	retval.setProperty(0, x.r());
	retval.setProperty(1, x.g());
	retval.setProperty(2, x.b());
	return retval;
}

void toColor(const QScriptValue& obj, Color& x) {
	QScriptEngine* engine = obj.engine();
	QScriptContext* context = engine->currentContext();
	if (!obj.isArray()) {
		context->throwError("object must be an array");
		return;
	}
	const unsigned length = obj.property("length").toInteger();
	if (length != 3) {
		context->throwError("array must have a length of 3");
		return;
	}
	x.r() = obj.property(0).toNumber();
	x.g() = obj.property(1).toNumber();
	x.b() = obj.property(2).toNumber();
}



QScriptValue fromColorCodingGradientPtr(QScriptEngine *engine, Particles::ColorCodingGradient* const& x) {
	return x->getOOType().name();
}

void toColorCodingGradientPtr(const QScriptValue& obj, Particles::ColorCodingGradient*& x) {
	QScriptEngine* engine = obj.engine();
	QScriptContext* context = engine->currentContext();
	QString gradientName = obj.toString();
	// Find the correct class.
	const OvitoObjectType* searchResultClass = nullptr;
	Q_FOREACH(const OvitoObjectType* clazz,
			  PluginManager::instance().listClasses(Particles::ColorCodingGradient::OOType)) {
		if (clazz->name() == gradientName) {
			searchResultClass = clazz;
			break;
		}
	}
	// Output it.
	if (searchResultClass == nullptr)
		context->throwError("Color gradient " + gradientName + " not found.");
	else {
		// Get DataSet.
		DataSet* dataSet = unwrapGlobalDataSet(engine);
		// Get instance of modifier.
		x = static_cast<Particles::ColorCodingGradient*>(searchResultClass->createNonRefInstance(dataSet));
	}
}



QScriptValue fromQUrl(QScriptEngine *engine, const QUrl& x) {
	return x.toDisplayString();
}

void toQUrl(const QScriptValue& obj, QUrl& x) {
	x = QUrl::fromLocalFile(obj.toString());
}




QScriptValue fromParticlePropertyReference(QScriptEngine *engine,
										   const Particles::ParticlePropertyReference& x) {
	if (x.type() == Particles::ParticleProperty::UserProperty) {
		int component = x.vectorComponent();
		if (component < 0)
			return x.name();
		else
			return x.name() + "." + QString::number(component);
	} else {
		QString name = Particles::ParticleProperty::standardPropertyName(x.type());
		if (x.vectorComponent() < 0)
			return name;
		else {
			QStringList components = Particles::ParticleProperty::standardPropertyComponentNames(x.type());
			if (x.vectorComponent() >= components.length())
				return name + "." + QString::number(x.vectorComponent());
			else
				return name + "." + components[x.vectorComponent()];
		}
	}
}

void toParticlePropertyReference(const QScriptValue& obj,
								 Particles::ParticlePropertyReference& x) {
	QScriptEngine* engine = obj.engine();
	QScriptContext* context = engine->currentContext();
	// Parse
	QStringList parts = obj.toString().split(".");
	if (parts.length() > 2) {
		context->throwError("too many dots in property string...");
		return;
	}
	// Get property type.
	Particles::ParticleProperty::Type type;
	QString name = parts[0];
	const QMap<QString, Particles::ParticleProperty::Type> m = Particles::ParticleProperty::standardPropertyList();
	const auto i = m.find(name);
	if (i == m.end()) {
		// User property.
		type = Particles::ParticleProperty::UserProperty;
	} else {
		// Standard property.
		type = *i;
	}

	// Possibly get component.
	int component = -1;
	if (parts.length() == 2) {
		// We have a component given. First try to convert to integer.
		bool ok;
		component = parts[1].toInt(&ok);
		if (!ok) {
			// Perhaps the name was used instead of an integer.
			const QStringList l = Particles::ParticleProperty::standardPropertyComponentNames(type);
			const QString componentName = parts[1].toUpper();
			component = -1;
			for (int i = 0; i != l.length(); ++i)
				if (parts[1] == l[i]) {
					component = i;
					break;
				}
			if (component < 0) {
				context->throwError("unknown component name: " + parts[1]);
				return;
			}
		}
	}

	// Construct object.
	if (type == Particles::ParticleProperty::UserProperty)
		if (component < 0)
			x = Particles::ParticlePropertyReference(name);
		else
			x = Particles::ParticlePropertyReference(name, component);
	else
		if (component < 0)
			x = Particles::ParticlePropertyReference(type);
		else
			x = Particles::ParticlePropertyReference(type, component);
}



// Factory function for script engine //////////////////////////////////

QScriptEngine* prepareEngine(DataSet* dataSet, QObject* parent) {
	// Set up engine.
	QScriptEngine* engine = new QScriptEngine(parent);

	// Store global DataSet.
	engine->setProperty("_ovito_dataset", QVariant::fromValue(OORef<OvitoObject>(dataSet)));

	// Register automatic conversions.
	const int FloatTypeTypeId = qRegisterMetaType<FloatType>("FloatType");
	qScriptRegisterMetaType<FloatType>(engine, fromFloatType, toFloatType);

	const int Vector3TypeId = qRegisterMetaType<Vector3>("Vector3");
	qScriptRegisterMetaType<Vector3>(engine, fromVector3, toVector3);

	const int ColorTypeId = qRegisterMetaType<Color>("Color");
	qScriptRegisterMetaType<Color>(engine, fromColor, toColor);

	const int ColorCodingGradientPtrTypeId =
		qRegisterMetaType<Particles::ColorCodingGradient*>("ColorCodingGradient*");
	qScriptRegisterMetaType<Particles::ColorCodingGradient*>(engine,
															 fromColorCodingGradientPtr,
															 toColorCodingGradientPtr);

	const int QUrlId = qRegisterMetaType<QUrl>("QUrl");
	qScriptRegisterMetaType<QUrl>(engine, fromQUrl, toQUrl);

	const int ParticlePropertyReferenceTypeId =
		qRegisterMetaType<Particles::ParticlePropertyReference>("ParticlePropertyReference");
	qScriptRegisterMetaType<Particles::ParticlePropertyReference>(engine,
																  fromParticlePropertyReference,
																  toParticlePropertyReference);

	// Set up namespace. ///////////////////////////////////////////////

	ViewportConfiguration* viewportConf = dataSet->viewportConfig();

	// All viewports.
	const QVector<Viewport*>& allViewports = viewportConf->viewports();
	QScriptValue viewport = engine->newArray(allViewports.size());
	for (int i = 0; i != allViewports.size(); ++i)
		viewport.setProperty(i, engine->newQObject(new ViewportBinding(allViewports[i],
																	   engine,
																	   dataSet),
												   QScriptEngine::ScriptOwnership));
	engine->globalObject().setProperty("viewport", viewport);

	// Active viewport
	QScriptValue activeViewport =
		engine->newQObject(new ActiveViewportBinding(engine, dataSet),
						   QScriptEngine::ScriptOwnership);
	engine->globalObject().setProperty("activeViewport", activeViewport);

	// quit function.
	engine->globalObject().setProperty("quit",
									   engine->newFunction(quit, 0));

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

	// modifier function.
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
