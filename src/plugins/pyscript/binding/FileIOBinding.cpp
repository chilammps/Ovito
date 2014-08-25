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
#include <core/dataset/importexport/ImportExportManager.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/dataset/importexport/FileExporter.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/utilities/io/FileManager.h>
#include "PythonBinding.h"

namespace PyScript {

using namespace boost::python;
using namespace Ovito;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(FileExporter_exportToFile_overloads, exportToFile, 2, 3);

BOOST_PYTHON_MODULE(PyScriptFileIO)
{
	docstring_options docoptions(true, false);

	class_<QUrl>("QUrl", init<>())
		.def(init<const QString&>())
		.def("clear", &QUrl::clear)
		.add_property("errorString", &QUrl::errorString)
		.add_property("isEmpty", &QUrl::isEmpty)
		.add_property("isLocalFile", &QUrl::isLocalFile)
		.add_property("isValid", &QUrl::isValid)
		.def("__str__", (QString (*)(const QUrl&))([](const QUrl& url) { return url.toString(QUrl::PreferLocalFile); }))
		.def(self == other<QUrl>())
		.def(self != other<QUrl>())
	;

	// Install automatic Python string to QUrl conversion.
	auto convertible_QUrl = [](PyObject* obj_ptr) -> void* {
		// Check if Python object can be converted to target type.
		if(!PyString_Check(obj_ptr)) return nullptr;
		return obj_ptr;
	};
	auto construct_QUrl = [](PyObject* obj_ptr, converter::rvalue_from_python_stage1_data* data) {
		QString value = extract<QString>(obj_ptr);
		void* storage = ((converter::rvalue_from_python_storage<QUrl>*)data)->storage.bytes;
		new (storage) QUrl(FileManager::instance().urlFromUserInput(value));
		data->convertible = storage;
	};
	converter::registry::push_back(convertible_QUrl, construct_QUrl, type_id<QUrl>());

	ovito_abstract_class<FileImporter, RefTarget>()
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

	ovito_abstract_class<LinkedFileImporter, FileImporter>()
		.def("requestReload", &LinkedFileImporter::requestReload)
		.def("requestFramesUpdate", &LinkedFileImporter::requestFramesUpdate)
	;

	ovito_abstract_class<FileExporter, RefTarget>()
		.add_property("fileFilter", &FileExporter::fileFilter)
		.add_property("fileFilterDescription", &FileExporter::fileFilterDescription)
		.def("exportToFile", &FileExporter::exportToFile, FileExporter_exportToFile_overloads())
	;

	ovito_class<LinkedFileObject, SceneObject>(
			"This object serves as data source for a modification pipeline. It reads data from one or more external files "
			"and makes it available as input for modifiers."
			"\n\n"
			"You normally do not create an instance of this class yourself. "
			"The :py:func:`ovito.io.import_file` function automatically assigns a :py:class:`!FileSourceObject` to the :py:attr:`~ovito.scene.ObjectNode.source` "
			"attribute of the returned :py:class:`~ovito.scene.ObjectNode`. "
			"The file source object is responsible for managing the reference to the external file and for loading data upon request, which "
			"is then fed into the modification pipeline."
			"\n\n"
			"The :py:meth:`FileSourceObject.load` method allows you to hook a different external file into an existing modification pipeline::"
			"\n\n"
			"    # This creates a new node with an empty modification pipeline:\n"
			"    node = import_file(\"first_file.dump\")\n"
			"    \n"
			"    # Populate the pipeline with a modifier:\n"
			"    node.modifiers.append(ColorCodingModifier(source=\"Potential Energy\"))\n"
			"    \n"
			"    # This will replace the input data with a new file \n"
			"    # but keeps the modification pipeline:\n"
			"    node.source.load(\"second_file.dump\")\n"
			"\n"
			"File source objects are also used by certain modifiers to load a reference configuration."
//			"Note the actual parsing of the external file is not performed by the :py:class:`!FileSourceObject` itself. For this, a dedicated "
//			"file importer object is responsible, which is accessible through the :py:attr:`.importer` attribute. When loading a new file, "
//			"the :py:class:`!FileSourceObject`` automatically creates the right file importer depending on the file's format (which is auto-detected). "
//			"Note that the importer might have additional parameter attributes, which further control the loading of specific file formats."
			"\n\n"
			"**Example**"
			"\n\n"
			"The following script accepts a list of data files on the command line. It loads them one by one and performs a common neighbor analysis "
			"to determine the number of FCC atoms in each structure::"
			"\n\n"
			"    import sys\n"
			"    from ovito.io import *\n"
			"    from ovito.modifiers import *\n"
			"    \n"
			"    node = None\n"
			"    for file in sys.argv[1:]:\n\n"
			"        if not node:\n"
			"            # Import the first file using import_file().\n"
			"            # This creates the ObjectNode and sets up the modification pipeline.\n"
			"            node = import_file(file)\n"
			"            # Insert a modifier into the pipeline.\n"
			"            cna = CommonNeighborAnalysisModifier(adaptive_mode=True)\n"
			"            node.modifiers.append(cna)\n"
			"        else:\n"
			"            # To load subsequent files, call the load() function of the FileSourceObject.\n"
			"            node.source.load(file)\n\n"
			"        # Wait until the results of the analysis modifier are available.\n"
			"        node.wait()\n"
			"        print \"Structure %s contains %i FCC atoms.\" % (file, cna.counts[\"FCC\"])\n",
			// The Python class name:
			"FileSourceObject")
		.add_property("importer", make_function(&LinkedFileObject::importer, return_value_policy<ovito_object_reference>()))
		.add_property("source_path", make_function(&LinkedFileObject::sourceUrl, return_value_policy<copy_const_reference>()))
		.add_property("status", &LinkedFileObject::status)
		.add_property("num_frames", &LinkedFileObject::numberOfFrames,
				"The number of frames the loaded file or file sequence contains (read-only).")
		.add_property("loaded_frame", &LinkedFileObject::loadedFrame,
				"The zero-based index of the frame that is currently loaded (read-only)	.")
		.add_property("adjust_animation_interval", &LinkedFileObject::adjustAnimationIntervalEnabled, &LinkedFileObject::setAdjustAnimationIntervalEnabled,
				"A flag that controls whether the animation length in OVITO is automatically adjusted to match the number of frames in the "
				"loaded file or file sequence."
				"\n\n"
				"The current length of the animation in OVITO is stored in the :py:class:`~ovito.anim.AnimationSettings` object. The number of frames in the external file "
				"or file sequence is indicated by the :py:attr:`.num_frames` attribute of this :py:class:`!FileSourceObject`. If :py:attr:`.adjust_animation_interval` "
				"is ``True``, then animation length will be automatically adjusted to match the number of frames in the file input. "
				"\n\n"
				"In some situations it makes sense to turn this option off, for example, if you import several data files into "
				"OVITO simultaneously, but their frame counts do not match. "
				"\n\n"
				"Default: ``True``\n")
		.add_property("sceneObjects", make_function(&LinkedFileObject::sceneObjects, return_internal_reference<>()))
		.def("refreshFromSource", &LinkedFileObject::refreshFromSource)
		.def("updateFrames", &LinkedFileObject::updateFrames)
		.def("animationTimeToInputFrame", &LinkedFileObject::animationTimeToInputFrame)
		.def("inputFrameToAnimationTime", &LinkedFileObject::inputFrameToAnimationTime)
		.def("adjustAnimationInterval", &LinkedFileObject::adjustAnimationInterval)
		.def("addSceneObject", &LinkedFileObject::addSceneObject)
		.def("setSource", (bool (LinkedFileObject::*)(const QUrl&, const FileImporterDescription*))&LinkedFileObject::setSource)
		.def("setSource", (bool (LinkedFileObject::*)(QUrl, LinkedFileImporter*, bool))&LinkedFileObject::setSource)
	;
}

OVITO_REGISTER_PLUGIN_PYTHON_INTERFACE(PyScriptFileIO);

};
