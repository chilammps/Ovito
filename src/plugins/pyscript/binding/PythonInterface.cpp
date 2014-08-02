///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#include <plugins/pyscript/PyScript.h>
#include <core/animation/TimeInterval.h>
#include <core/animation/AnimationSettings.h>
#include <core/dataset/DataSet.h>
#include <core/dataset/DataSetContainer.h>
#include <core/viewport/ViewportConfiguration.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/SceneRenderer.h>
#include <core/rendering/standard/StandardSceneRenderer.h>
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/dataset/importexport/FileExporter.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/scene/SceneNode.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/GroupNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/SelectionSet.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/display/DisplayObject.h>
#include <core/scene/display/geometry/TriMeshDisplay.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/utilities/io/FileManager.h>

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

// Implementation of the __str__ method for OvitoObject derived classes.
inline object OvitoObject__str__(OvitoObject* o) {
	if(!o) return str("<OvitoObject nullptr>");
	else return "<%s at address 0x%x>" % make_tuple(o->getOOType().name(), (std::intptr_t)o);
}

// A model of the Boost.Python concept ResultConverterGenerator which wraps
// a raw pointer to an OvitoObject derived class in a OORef<> smart pointer.
struct ovito_object_reference
{
	struct make_ooref_holder {
		template <class T> static PyObject* execute(T* p) {
			typedef objects::pointer_holder<OORef<T>, T> holder_t;
			OORef<T> ptr(const_cast<T*>(p));
			return objects::make_ptr_instance<T, holder_t>::execute(ptr);
		}
	};
	template <class T> struct apply {
        typedef to_python_indirect<T, make_ooref_holder> type;
    };
};

/// Indexing suite for QVector<T*> containers, where T is an OvitoObject derived class.
/// This indexing suite exposes only read-only methods which do not modify the QVector container.
template<class T, typename Container = QVector<T*>>
class QVector_OO_readonly_indexing_suite : public def_visitor<QVector_OO_readonly_indexing_suite<T>>
{
public:
    typedef T* data_type;
    typedef T* key_type;
    typedef typename Container::size_type index_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::difference_type difference_type;
    typedef return_value_policy<ovito_object_reference> iterator_return_policy;

    typedef boost::python::iterator<Container, iterator_return_policy> def_iterator;

    template<class Class>
    void visit(Class& cl) const {
        cl
            .def("__len__", &get_size)
            .def("__setitem__", &set_item)
            .def("__delitem__", &delete_item)
            .def("__getitem__", make_function(&get_item, return_value_policy<ovito_object_reference>()))
            .def("__contains__", &contains)
            .def("__iter__", def_iterator())
        ;
    }

    static size_type get_size(Container& container) {
    	return container.size();
    }

    static T* get_item(back_reference<Container&> container, PyObject* i) {
        if(PySlice_Check(i)) {
        	PyErr_SetString(PyExc_NotImplementedError, "This sequence type does not support slicing.");
        	throw_error_already_set();
        }
        return container.get()[convert_index(container.get(), i)];
    }

	static void set_item(Container& container, PyObject* i, PyObject* v) {
		PyErr_SetString(PyExc_NotImplementedError, "This sequence type is read-only.");
		throw_error_already_set();
	}

	static void delete_item(Container& container, PyObject* i) {
		PyErr_SetString(PyExc_NotImplementedError, "This sequence type is read-only.");
		throw_error_already_set();
	}

	static bool contains(Container& container, key_type const& key) {
		return container.contains(key);
	}

	static index_type convert_index(Container& container, PyObject* i_) {
		extract<index_type> i(i_);
		if(i.check()) {
			index_type index = i();
			if(index < 0)
				index += container.size();
			if(index >= container.size() || index < 0) {
				PyErr_SetString(PyExc_IndexError, "Index out of range");
				throw_error_already_set();
			}
			return index;
		}

		PyErr_SetString(PyExc_TypeError, "Invalid index type");
		throw_error_already_set();
		return index_type();
	}
};

BOOST_PYTHON_MODULE(PyScript)
{
	class_<TimeInterval>("TimeInterval", init<>())
		.def(init<TimePoint>())
		.def(init<TimePoint, TimePoint>())
		.add_property("start", &TimeInterval::start, &TimeInterval::setStart)
		.add_property("end", &TimeInterval::end, &TimeInterval::setEnd)
		.add_property("isEmpty", &TimeInterval::isEmpty)
		.add_property("isInfinite", &TimeInterval::isInfinite)
		.add_property("duration", &TimeInterval::duration, &TimeInterval::setDuration)
		.def("setInfinite", &TimeInterval::setInfinite)
		.def("setEmpty", &TimeInterval::setEmpty)
		.def("setInstant", &TimeInterval::setInstant)
		.def("contains", &TimeInterval::contains)
		.def("intersect", &TimeInterval::intersect)
		.def("timeToSeconds", &timeToSeconds)
		.def("secondsToTime", &secondsToTime)
		.staticmethod("timeToSeconds")
		.staticmethod("secondsToTime")
		.add_static_property("infinite", &TimeInterval::infinite)
		.add_static_property("empty", &TimeInterval::empty)
		.setattr("TimeNegativeInfinity", TimeNegativeInfinity())
		.setattr("TimePositiveInfinity", TimePositiveInfinity())
		.def(self == TimeInterval())
		.def(self != TimeInterval())
	;

	class_<QUrl>("QUrl", init<>())
		.def(init<const QString&>())
		.add_property("errorString", &QUrl::errorString)
		.add_property("isEmpty", &QUrl::isEmpty)
		.add_property("isLocalFile", &QUrl::isLocalFile)
		.add_property("isValid", &QUrl::isValid)
		.add_property("localFile", &QUrl::toLocalFile)
	;

	class_<OvitoObject, OORef<OvitoObject>, boost::noncopyable>("OvitoObject", no_init)
		.def("__str__", OvitoObject__str__)
	;

	class_<RefMaker, bases<OvitoObject>, OORef<RefMaker>, boost::noncopyable>("RefMaker", no_init)
		.add_property("dataset", make_function(&RefMaker::dataset, return_value_policy<ovito_object_reference>()))
	;

	class_<RefTarget, bases<RefMaker>, OORef<RefTarget>, boost::noncopyable>("RefTarget", no_init)
		.def("isReferencedBy", &RefTarget::isReferencedBy)
		.def("deleteReferenceObject", &RefTarget::deleteReferenceObject)
		.add_property("isBeingEdited", &RefTarget::isBeingEdited)
		.add_property("objectTitle", &RefTarget::objectTitle)
	;

	class_<AnimationSettings, bases<RefTarget>, OORef<AnimationSettings>, boost::noncopyable>("AnimationSettings", no_init)
		.add_property("time", &AnimationSettings::time, &AnimationSettings::setTime)
		.add_property("animationInterval", make_function(&AnimationSettings::animationInterval, return_value_policy<copy_const_reference>()), &AnimationSettings::setAnimationInterval)
		.add_property("framesPerSecond", &AnimationSettings::framesPerSecond, &AnimationSettings::setFramesPerSecond)
		.add_property("ticksPerFrame", &AnimationSettings::ticksPerFrame, &AnimationSettings::setTicksPerFrame)
		.add_property("currentFrame", &AnimationSettings::currentFrame, &AnimationSettings::setCurrentFrame)
		.add_property("lastFrame", &AnimationSettings::lastFrame, &AnimationSettings::setLastFrame)
		.add_property("firstFrame", &AnimationSettings::firstFrame, &AnimationSettings::setFirstFrame)
		.add_property("playbackSpeed", &AnimationSettings::playbackSpeed, &AnimationSettings::setPlaybackSpeed)
		.add_property("isAnimating", &AnimationSettings::isAnimating)
		.add_property("autoKeyMode", &AnimationSettings::autoKeyMode, &AnimationSettings::setAutoKeyMode)
		.add_property("isTimeChanging", &AnimationSettings::isTimeChanging)
		.def("frameToTime", &AnimationSettings::frameToTime)
		.def("timeToFrame", &AnimationSettings::timeToFrame)
		.def("snapTime", &AnimationSettings::snapTime)
		.def("timeToString", &AnimationSettings::timeToString)
		.def("stringToTime", &AnimationSettings::stringToTime)
		.def("jumpToAnimationStart", &AnimationSettings::jumpToAnimationStart)
		.def("jumpToAnimationEnd", &AnimationSettings::jumpToAnimationEnd)
		.def("jumpToNextFrame", &AnimationSettings::jumpToNextFrame)
		.def("jumpToPreviousFrame", &AnimationSettings::jumpToPreviousFrame)
		.def("startAnimationPlayback", &AnimationSettings::startAnimationPlayback)
		.def("stopAnimationPlayback", &AnimationSettings::stopAnimationPlayback)
	;

	class_<Viewport, bases<RefTarget>, OORef<Viewport>, boost::noncopyable>("Viewport", no_init)
		.add_property("isRendering", &Viewport::isRendering)
		.add_property("isPerspectiveProjection", &Viewport::isPerspectiveProjection)
		.add_property("viewType", &Viewport::viewType, &Viewport::setViewType)
		.add_property("fieldOfView", &Viewport::fieldOfView, &Viewport::setFieldOfView)
		.add_property("cameraTransformation", make_function(&Viewport::cameraTransformation, return_value_policy<copy_const_reference>()), &Viewport::setCameraTransformation)
		.add_property("cameraDirection", &Viewport::cameraDirection, &Viewport::setCameraDirection)
		.add_property("cameraPosition", &Viewport::cameraPosition, &Viewport::setCameraPosition)
		.add_property("viewMatrix", make_function(&Viewport::viewMatrix, return_value_policy<copy_const_reference>()))
		.add_property("inverseViewMatrix", make_function(&Viewport::inverseViewMatrix, return_value_policy<copy_const_reference>()))
		.add_property("projectionMatrix", make_function(&Viewport::projectionMatrix, return_value_policy<copy_const_reference>()))
		.add_property("inverseProjectionMatrix", make_function(&Viewport::inverseProjectionMatrix, return_value_policy<copy_const_reference>()))
		.add_property("renderFrameShown", &Viewport::renderFrameShown, &Viewport::setRenderFrameShown)
		.add_property("gridVisible", &Viewport::isGridVisible, &Viewport::setGridVisible)
		.add_property("viewNode", make_function(&Viewport::viewNode, return_value_policy<ovito_object_reference>()), &Viewport::setViewNode)
		.add_property("gridMatrix", make_function(&Viewport::gridMatrix, return_value_policy<copy_const_reference>()), &Viewport::setGridMatrix)
		.add_property("viewportTitle", make_function(&Viewport::viewportTitle, return_value_policy<copy_const_reference>()))
		.def("updateViewport", &Viewport::updateViewport)
		.def("redrawViewport", &Viewport::redrawViewport)
		.def("nonScalingSize", &Viewport::nonScalingSize)
		.def("zoomToSceneExtents", &Viewport::zoomToSceneExtents)
		.def("zoomToSelectionExtents", &Viewport::zoomToSelectionExtents)
		.def("zoomToBox", &Viewport::zoomToBox)
	;

	enum_<Viewport::ViewType>("ViewType")
		.value("NONE", Viewport::VIEW_NONE)
		.value("TOP", Viewport::VIEW_TOP)
		.value("BOTTOM", Viewport::VIEW_BOTTOM)
		.value("FRONT", Viewport::VIEW_FRONT)
		.value("BACK", Viewport::VIEW_BACK)
		.value("LEFT", Viewport::VIEW_LEFT)
		.value("RIGHT", Viewport::VIEW_RIGHT)
		.value("ORTHO", Viewport::VIEW_ORTHO)
		.value("PERSPECTIVE", Viewport::VIEW_PERSPECTIVE)
		.value("SCENENODE", Viewport::VIEW_SCENENODE)
	;

	class_<ViewportConfiguration, bases<RefTarget>, OORef<ViewportConfiguration>, boost::noncopyable>("ViewportConfiguration", no_init)
		.add_property("activeViewport", make_function(&ViewportConfiguration::activeViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setActiveViewport)
		.add_property("maximizedViewport", make_function(&ViewportConfiguration::maximizedViewport, return_value_policy<ovito_object_reference>()), &ViewportConfiguration::setMaximizedViewport)
		.def("zoomToSelectionExtents", &ViewportConfiguration::zoomToSelectionExtents)
		.def("zoomToSceneExtents", &ViewportConfiguration::zoomToSceneExtents)
		.def("updateViewports", &ViewportConfiguration::updateViewports)
	;

	class_<RenderSettings, bases<RefTarget>, OORef<RenderSettings>, boost::noncopyable>("RenderSettings", init<DataSet*>())
		.add_property("renderer", make_function(&RenderSettings::renderer, return_value_policy<ovito_object_reference>()), &RenderSettings::setRenderer)
		.add_property("renderingRangeType", &RenderSettings::renderingRangeType, &RenderSettings::setRenderingRangeType)
		.add_property("outputImageWidth", &RenderSettings::outputImageWidth, &RenderSettings::setOutputImageWidth)
		.add_property("outputImageHeight", &RenderSettings::outputImageHeight, &RenderSettings::setOutputImageHeight)
		.add_property("outputImageAspectRatio", &RenderSettings::outputImageAspectRatio)
		.add_property("imageFilename", make_function(&RenderSettings::imageFilename, return_value_policy<copy_const_reference>()), &RenderSettings::setImageFilename)
		.add_property("backgroundColor", &RenderSettings::backgroundColor, &RenderSettings::setBackgroundColor)
		.add_property("generateAlphaChannel", &RenderSettings::generateAlphaChannel, &RenderSettings::setGenerateAlphaChannel)
		.add_property("saveToFile", &RenderSettings::saveToFile, &RenderSettings::setSaveToFile)
		.add_property("skipExistingImages", &RenderSettings::skipExistingImages, &RenderSettings::setSkipExistingImages)
		.add_property("customRangeStart", &RenderSettings::customRangeStart, &RenderSettings::setCustomRangeStart)
		.add_property("customRangeEnd", &RenderSettings::customRangeEnd, &RenderSettings::setCustomRangeEnd)
		.add_property("everyNthFrame", &RenderSettings::everyNthFrame, &RenderSettings::setEveryNthFrame)
		.add_property("fileNumberBase", &RenderSettings::fileNumberBase, &RenderSettings::setFileNumberBase)
	;

	enum_<RenderSettings::RenderingRangeType>("RenderingRangeType")
		.value("CURRENT_FRAME", RenderSettings::CURRENT_FRAME)
		.value("ANIMATION_INTERVAL", RenderSettings::ANIMATION_INTERVAL)
		.value("CUSTOM_INTERVAL", RenderSettings::CUSTOM_INTERVAL)
	;

	class_<SceneRenderer, bases<RefTarget>, OORef<SceneRenderer>, boost::noncopyable>("SceneRenderer", no_init)
		.add_property("isInteractive", &SceneRenderer::isInteractive)
	;

	class_<StandardSceneRenderer, bases<SceneRenderer>, OORef<StandardSceneRenderer>, boost::noncopyable>("StandardSceneRenderer", init<DataSet*>())
		.add_property("antialiasingLevel", &StandardSceneRenderer::antialiasingLevel, &StandardSceneRenderer::setAntialiasingLevel)
	;

	class_<FileImporter, bases<RefTarget>, OORef<FileImporter>, boost::noncopyable>("FileImporter", no_init)
		.add_property("fileFilter", &FileImporter::fileFilter)
		.add_property("fileFilterDescription", &FileImporter::fileFilterDescription)
		.def("importFile", &FileImporter::importFile)
		.def("checkFileFormat", &FileImporter::checkFileFormat)
	;

	enum_<FileImporter::ImportMode>("ImportMode")
		.value("AskUser", FileImporter::AskUser)
		.value("AddToScene", FileImporter::AddToScene)
		.value("ReplaceSelected", FileImporter::ReplaceSelected)
		.value("ResetScene", FileImporter::ResetScene)
	;

	class_<ImportExportManager, boost::noncopyable>("ImportExportManager", no_init)
		.add_static_property("instance", make_function(&ImportExportManager::instance, return_value_policy<reference_existing_object>()))
		.def("autodetectFileFormat", (OORef<FileImporter> (ImportExportManager::*)(DataSet*, const QUrl&))&ImportExportManager::autodetectFileFormat)
	;

	class_<FileManager, boost::noncopyable>("FileManager", no_init)
		.add_static_property("instance", make_function(&FileManager::instance, return_value_policy<reference_existing_object>()))
		.def("removeFromCache", &FileManager::removeFromCache)
		.def("urlFromUserInput", &FileManager::urlFromUserInput)
	;

	class_<LinkedFileImporter, bases<FileImporter>, OORef<LinkedFileImporter>, boost::noncopyable>("LinkedFileImporter", no_init)
		.def("requestReload", &LinkedFileImporter::requestReload)
		.def("requestFramesUpdate", &LinkedFileImporter::requestFramesUpdate)
	;

	class_<FileExporter, bases<RefTarget>, OORef<FileExporter>, boost::noncopyable>("FileExporter", no_init)
		.add_property("fileFilter", &FileExporter::fileFilter)
		.add_property("fileFilterDescription", &FileExporter::fileFilterDescription)
		.def("exportToFile", &FileExporter::exportToFile)
	;

	class_<PipelineStatus>("PipelineStatus", init<optional<PipelineStatus::StatusType, const QString&>>())
		.add_property("type", &PipelineStatus::type)
		.add_property("text", make_function(&PipelineStatus::text, return_value_policy<copy_const_reference>()))
		.def(self == PipelineStatus())
		.def(self != PipelineStatus())
	;

	enum_<PipelineStatus::StatusType>("PipelineStatusType")
		.value("Success", PipelineStatus::Success)
		.value("Warning", PipelineStatus::Warning)
		.value("Error", PipelineStatus::Error)
		.value("Pending", PipelineStatus::Pending)
	;

	class_<PipelineFlowState>("PipelineFlowState", init<>())
		.def(init<SceneObject*, TimeInterval>())
		.def(init<const PipelineStatus&, const QVector<SceneObject*>&, const TimeInterval&>())
		.def("clear", &PipelineFlowState::clear)
		.def("addObject", &PipelineFlowState::addObject)
		.def("replaceObject", &PipelineFlowState::replaceObject)
		.def("removeObject", &PipelineFlowState::removeObject)
		.add_property("count", &PipelineFlowState::count)
		.add_property("isEmpty", &PipelineFlowState::isEmpty)
		.add_property("status", make_function(&PipelineFlowState::status, return_internal_reference<>()), &PipelineFlowState::setStatus)
		.add_property("objects", make_function(&PipelineFlowState::objects, return_internal_reference<>()))
	;

	class_<DisplayObject, bases<RefTarget>, OORef<DisplayObject>, boost::noncopyable>("DisplayObject", no_init)
		.add_property("enabled", &DisplayObject::isEnabled, &DisplayObject::setEnabled)
	;

	class_<TriMeshDisplay, bases<DisplayObject>, OORef<TriMeshDisplay>, boost::noncopyable>("TriMeshDisplay", init<DataSet*>())
		.add_property("color", make_function(&TriMeshDisplay::color, return_value_policy<copy_const_reference>()), &TriMeshDisplay::setColor)
		.add_property("transparency", &TriMeshDisplay::transparency, &TriMeshDisplay::setTransparency)
	;

	class_<SceneObject, bases<RefTarget>, OORef<SceneObject>, boost::noncopyable>("SceneObject", no_init)
		.def("objectValidity", &SceneObject::objectValidity)
		.def("evaluate", &SceneObject::evaluate)
		.def("addDisplayObject", &SceneObject::addDisplayObject)
		.def("setDisplayObject", &SceneObject::setDisplayObject)
		.def("inputObject", make_function(&SceneObject::inputObject, return_value_policy<ovito_object_reference>()))
		.add_property("status", &SceneObject::status)
		.add_property("displayObjects", make_function(&SceneObject::displayObjects, return_internal_reference<>()))
		.add_property("saveWithScene", &SceneObject::saveWithScene, &SceneObject::setSaveWithScene)
		.add_property("inputObjectCount", &SceneObject::inputObjectCount)
	;

	class_<Modifier, bases<RefTarget>, OORef<Modifier>, boost::noncopyable>("Modifier", no_init)
		.add_property("enabled", &Modifier::isEnabled, &Modifier::setEnabled)
		.add_property("status", &Modifier::status)
		.def("modifierValidity", &Modifier::modifierValidity)
		.def("modifierApplications", &Modifier::modifierApplications)
		.def("isApplicableTo", &Modifier::isApplicableTo)
	;

	class_<ModifierApplication, bases<RefTarget>, OORef<ModifierApplication>, boost::noncopyable>("ModifierApplication", init<DataSet*>())
		.def(init<DataSet*, Modifier*>())
		.add_property("modifier", make_function(&ModifierApplication::modifier, return_value_policy<ovito_object_reference>()))
		.add_property("pipelineObject", make_function(&ModifierApplication::pipelineObject, return_value_policy<ovito_object_reference>()))
		.add_property("objectNodes", &ModifierApplication::objectNodes)
	;

	class_<PipelineObject, bases<SceneObject>, OORef<PipelineObject>, boost::noncopyable>("PipelineObject", init<DataSet*>())
		.add_property("inputObject", make_function((SceneObject* (PipelineObject::*)() const)&PipelineObject::inputObject, return_value_policy<ovito_object_reference>()), &PipelineObject::setInputObject)
		.add_property("modifierApplications", make_function(&PipelineObject::modifierApplications, return_internal_reference<>()))
		.def("insertModifier", make_function(&PipelineObject::insertModifier, return_value_policy<ovito_object_reference>()))
		.def("insertModifierApplication", &PipelineObject::insertModifierApplication)
		.def("removeModifier", &PipelineObject::removeModifier)
	;

	class_<LinkedFileObject, bases<SceneObject>, OORef<LinkedFileObject>, boost::noncopyable>("LinkedFileObject", init<DataSet*>())
		.add_property("importer", make_function(&LinkedFileObject::importer, return_value_policy<ovito_object_reference>()))
	// Add setter method.
		.add_property("sourceUrl", make_function(&LinkedFileObject::sourceUrl, return_value_policy<copy_const_reference>()))
		.add_property("status", &LinkedFileObject::status)
		.add_property("numberOfFrames", &LinkedFileObject::numberOfFrames)
		.add_property("loadedFrame", &LinkedFileObject::loadedFrame)
		.add_property("adjustAnimationIntervalEnabled", &LinkedFileObject::adjustAnimationIntervalEnabled, &LinkedFileObject::setAdjustAnimationIntervalEnabled)
		.add_property("sceneObjects", make_function(&LinkedFileObject::sceneObjects, return_internal_reference<>()))
		.def("refreshFromSource", &LinkedFileObject::refreshFromSource)
		.def("updateFrames", &LinkedFileObject::updateFrames)
		.def("animationTimeToInputFrame", &LinkedFileObject::animationTimeToInputFrame)
		.def("inputFrameToAnimationTime", &LinkedFileObject::inputFrameToAnimationTime)
		.def("adjustAnimationInterval", &LinkedFileObject::adjustAnimationInterval)
		.def("addSceneObject", &LinkedFileObject::addSceneObject)
	;

	class_<SceneNode, bases<RefTarget>, OORef<SceneNode>, boost::noncopyable>("SceneNode", no_init)
		.add_property("name", make_function(&SceneNode::name, return_value_policy<copy_const_reference>()), &SceneNode::setName)
		.add_property("displayColor", make_function(&SceneNode::displayColor, return_value_policy<copy_const_reference>()), &SceneNode::setDisplayColor)
		.add_property("parentNode", make_function(&SceneNode::parentNode, return_value_policy<ovito_object_reference>()))
		.add_property("childCount", &SceneNode::childCount)
		.add_property("children", make_function(&SceneNode::children, return_internal_reference<>()))
		.add_property("targetNode", make_function(&SceneNode::targetNode, return_value_policy<ovito_object_reference>()))
		.add_property("selected", &SceneNode::isSelected, &SceneNode::setSelected)
		.def("deleteNode", &SceneNode::deleteNode)
		.def("addChild", &SceneNode::addChild)
		.def("removeChild", &SceneNode::removeChild)
		.def("localBoundingBox", &SceneNode::localBoundingBox)
		.def("worldBoundingBox", make_function(&SceneNode::worldBoundingBox, return_value_policy<copy_const_reference>()))
	;

	class_<QVector<DisplayObject*>, boost::noncopyable>("DisplayObjectQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<DisplayObject>())
	;
	class_<QVector<SceneNode*>, boost::noncopyable>("SceneNodeQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneNode>())
	;
	class_<QVector<SceneObject*>, boost::noncopyable>("SceneObjectQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject>())
	;
	class_<QVector<OORef<SceneObject>>, boost::noncopyable>("OORefSceneObjectQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<SceneObject, QVector<OORef<SceneObject>>>())
	;
	class_<QVector<ModifierApplication*>, boost::noncopyable>("ModifierApplicationQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<ModifierApplication>())
	;
	class_<QVector<ModifierApplication*>, boost::noncopyable>("ModifierApplicationQVector", no_init)
		.def(QVector_OO_readonly_indexing_suite<ModifierApplication>())
	;

	class_<ObjectNode, bases<SceneNode>, OORef<ObjectNode>, boost::noncopyable>("ObjectNode", init<DataSet*>())
		.add_property("sceneObject", make_function(&ObjectNode::sceneObject, return_value_policy<ovito_object_reference>()), &ObjectNode::setSceneObject)
		.add_property("displayObjects", make_function(&ObjectNode::displayObjects, return_internal_reference<>()))
		.def("evalPipeline", make_function(&ObjectNode::evalPipeline, return_value_policy<copy_const_reference>()))
		.def("applyModifier", &ObjectNode::applyModifier)
	;

	class_<SceneRoot, bases<SceneNode>, OORef<SceneRoot>, boost::noncopyable>("SceneRoot", init<DataSet*>())
		.def("getNodeByName", make_function(&SceneRoot::getNodeByName, return_value_policy<ovito_object_reference>()))
		.def("makeNameUnique", &SceneRoot::makeNameUnique)
	;

	class_<GroupNode, bases<SceneNode>, OORef<GroupNode>, boost::noncopyable>("GroupNode", init<DataSet*>())
		.add_property("open", &GroupNode::isGroupOpen, &GroupNode::setGroupOpen)
	;

	class_<SelectionSet, bases<RefTarget>, OORef<SelectionSet>, boost::noncopyable>("SelectionSet", init<DataSet*>())
		.add_property("count", &SelectionSet::count)
		.add_property("empty", &SelectionSet::empty)
		.add_property("firstNode", make_function(&SelectionSet::firstNode, return_value_policy<ovito_object_reference>()))
		.add_property("nodes", make_function(&SelectionSet::nodes, return_internal_reference<>()))
		.def("contains", &SelectionSet::contains)
		.def("add", &SelectionSet::add)
		.def("clear", &SelectionSet::clear)
		.def("remove", &SelectionSet::remove)
		.def("boundingBox", &SelectionSet::boundingBox)
		.def("setNode", &SelectionSet::setNode)
	;

	class_<DataSet, bases<RefTarget>, OORef<DataSet>, boost::noncopyable>("DataSet", no_init)
		.add_property("filePath", make_function(&DataSet::filePath, return_value_policy<copy_const_reference>()), &DataSet::setFilePath)
		.add_property("animationSettings", make_function(&DataSet::animationSettings, return_value_policy<ovito_object_reference>()))
		.add_property("viewportConfig", make_function(&DataSet::viewportConfig, return_value_policy<ovito_object_reference>()))
		.add_property("renderSettings", make_function(&DataSet::renderSettings, return_value_policy<ovito_object_reference>()))
		.add_property("selection", make_function(&DataSet::selection, return_value_policy<ovito_object_reference>()))
		.add_property("sceneRoot", make_function(&DataSet::sceneRoot, return_value_policy<ovito_object_reference>()))
		.add_property("container", make_function(&DataSet::container, return_value_policy<ovito_object_reference>()))
		.def("clearScene", &DataSet::clearScene)
		.def("rescaleTime", &DataSet::rescaleTime)
	;

	class_<DataSetContainer, bases<RefMaker>, boost::noncopyable>("DataSetContainer", no_init)
		.add_property("currentSet", make_function(&DataSetContainer::currentSet, return_value_policy<ovito_object_reference>()), &DataSetContainer::setCurrentSet)
		.def("fileNew", &DataSetContainer::fileNew)
		.def("fileLoad", &DataSetContainer::fileLoad)
		.def("fileSave", &DataSetContainer::fileSave)
		.def("fileSaveAs", &DataSetContainer::fileSaveAs)
		.def("askForSaveChanges", &DataSetContainer::askForSaveChanges)
	;
}

};
