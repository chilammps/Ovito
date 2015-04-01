###############################################################################
# 
#  Copyright (2013) Alexander Stukowski
#
#  This file is part of OVITO (Open Visualization Tool).
#
#  OVITO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  OVITO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

# Find Libav libraries for video encoding support.
FIND_PACKAGE(Libav)

SET(SourceFiles 
	utilities/Exception.cpp
	utilities/linalg/AffineDecomposition.cpp
	utilities/io/SaveStream.cpp
	utilities/io/LoadStream.cpp
	utilities/io/gzdevice/qtiocompressor.cpp
	object/OvitoObject.cpp
	object/OvitoObjectType.cpp
	object/NativeOvitoObjectType.cpp
	plugins/Plugin.cpp
	plugins/NativePlugin.cpp
	plugins/PluginManager.cpp
	plugins/autostart/AutoStartObject.cpp
	plugins/utility/UtilityApplet.cpp
	reference/CloneHelper.cpp
	reference/PropertyFieldDescriptor.cpp
	reference/PropertyField.cpp
	reference/RefMaker.cpp
	reference/RefTarget.cpp
	reference/RefTargetListener.cpp
	animation/AnimationSettings.cpp
	animation/controller/Controller.cpp
	animation/controller/ConstantControllers.cpp
	animation/controller/KeyframeController.cpp
	animation/controller/LinearInterpolationControllers.cpp
	animation/controller/SplineInterpolationControllers.cpp
	animation/controller/AnimationKeys.cpp
	animation/controller/LookAtController.cpp
	animation/controller/PRSTransformationController.cpp
	dataset/DataSet.cpp
	dataset/DataSetContainer.cpp
	dataset/UndoStack.cpp
	dataset/importexport/FileImporter.cpp
	dataset/importexport/FileSourceImporter.cpp
	dataset/importexport/FileSource.cpp
	dataset/importexport/FileSourceEditor.cpp
	scene/SceneNode.cpp
	scene/ObjectNode.cpp
	scene/SceneRoot.cpp
	scene/SelectionSet.cpp
	scene/objects/DisplayObject.cpp
	scene/objects/AsynchronousDisplayObject.cpp
	scene/objects/DataObject.cpp
	scene/objects/CompoundObject.cpp
	scene/objects/geometry/TriMeshObject.cpp
	scene/objects/geometry/TriMeshDisplay.cpp
	scene/objects/camera/CameraObject.cpp
	scene/objects/helpers/TargetObject.cpp
	scene/pipeline/PipelineFlowState.cpp
	scene/pipeline/PipelineObject.cpp
	scene/pipeline/Modifier.cpp
	scene/pipeline/ModifierApplication.cpp
	gui/app/Application.cpp
	gui/mainwin/MainWindow.cpp
	gui/mainwin/ViewportsPanel.cpp
	gui/mainwin/cmdpanel/CommandPanel.cpp
	gui/mainwin/cmdpanel/RenderCommandPage.cpp
	gui/mainwin/cmdpanel/ModifyCommandPage.cpp
	gui/mainwin/cmdpanel/ModificationListModel.cpp
	gui/mainwin/cmdpanel/ModificationListItem.cpp
	gui/mainwin/cmdpanel/ModifierListBox.cpp
	gui/mainwin/cmdpanel/UtilityCommandPage.cpp
	gui/mainwin/cmdpanel/OverlayCommandPage.cpp
	gui/dialogs/AdjustCameraDialog.cpp
	gui/dialogs/AnimationSettingsDialog.cpp
	gui/dialogs/ApplicationSettingsDialog.cpp
	gui/dialogs/GeneralSettingsPage.cpp
	gui/dialogs/ViewportSettingsPage.cpp
	gui/dialogs/HistoryFileDialog.cpp
	gui/dialogs/ImportFileDialog.cpp
	gui/dialogs/ImportRemoteFileDialog.cpp
	gui/dialogs/SaveImageFileDialog.cpp
	gui/dialogs/LoadImageFileDialog.cpp
	gui/dialogs/RemoteAuthenticationDialog.cpp
	gui/actions/ActionManager.cpp
	gui/actions/FileActions.cpp
	gui/actions/ViewportActions.cpp
	gui/actions/AnimationActions.cpp
	gui/actions/RenderActions.cpp
	gui/actions/ViewportModeAction.cpp
	gui/widgets/general/RolloutContainer.cpp
	gui/widgets/general/SpinnerWidget.cpp
	gui/widgets/general/ColorPickerWidget.cpp
	gui/widgets/general/ElidedTextLabel.cpp
	gui/widgets/general/AutocompleteLineEdit.cpp
	gui/widgets/animation/AnimationTimeSpinner.cpp
	gui/widgets/animation/AnimationFramesToolButton.cpp
	gui/widgets/animation/AnimationTimeSlider.cpp
	gui/widgets/animation/AnimationTrackBar.cpp
	gui/widgets/rendering/FrameBufferWidget.cpp
	gui/widgets/rendering/FrameBufferWindow.cpp
	gui/widgets/selection/SceneNodesListModel.cpp
	gui/widgets/selection/SceneNodeSelectionBox.cpp
	gui/widgets/display/CoordinateDisplayWidget.cpp
	gui/widgets/display/StatusWidget.cpp
	gui/properties/PropertiesEditor.cpp
	gui/properties/PropertiesPanel.cpp
	gui/properties/ParameterUI.cpp
	gui/properties/NumericalParameterUI.cpp
	gui/properties/AffineTransformationParameterUI.cpp
	gui/properties/BooleanActionParameterUI.cpp
	gui/properties/BooleanGroupBoxParameterUI.cpp
	gui/properties/BooleanParameterUI.cpp
	gui/properties/BooleanRadioButtonParameterUI.cpp
	gui/properties/ColorParameterUI.cpp
	gui/properties/CustomParameterUI.cpp
	gui/properties/FilenameParameterUI.cpp
	gui/properties/FloatParameterUI.cpp
	gui/properties/FontParameterUI.cpp
	gui/properties/IntegerParameterUI.cpp
	gui/properties/IntegerRadioButtonParameterUI.cpp
	gui/properties/RefTargetListParameterUI.cpp
	gui/properties/StringParameterUI.cpp
	gui/properties/SubObjectParameterUI.cpp
	gui/properties/VariantComboBoxParameterUI.cpp
	gui/properties/Vector3ParameterUI.cpp
	utilities/units/UnitsManager.cpp
	utilities/io/ObjectSaveStream.cpp
	utilities/io/ObjectLoadStream.cpp
	utilities/io/FileManager.cpp
	utilities/io/SftpJob.cpp
	utilities/io/CompressedTextReader.cpp
	utilities/io/CompressedTextWriter.cpp
	utilities/concurrent/Future.cpp
	utilities/concurrent/TaskManager.cpp
	utilities/mesh/TriMesh.cpp
	utilities/mesh/HalfEdgeMesh.cpp
	rendering/SceneRenderer.cpp
	rendering/viewport/ViewportSceneRenderer.cpp
	rendering/viewport/OpenGLLinePrimitive.cpp
	rendering/viewport/OpenGLParticlePrimitive.cpp
	rendering/viewport/OpenGLTextPrimitive.cpp
	rendering/viewport/OpenGLImagePrimitive.cpp
	rendering/viewport/OpenGLArrowPrimitive.cpp
	rendering/viewport/OpenGLMeshPrimitive.cpp
	rendering/viewport/OpenGLSharedResource.cpp
	rendering/standard/StandardSceneRenderer.cpp
	rendering/standard/StandardSceneRendererEditor.cpp
	rendering/noninteractive/NonInteractiveSceneRenderer.cpp
	rendering/noninteractive/DefaultLinePrimitive.cpp
	rendering/noninteractive/DefaultParticlePrimitive.cpp
	rendering/noninteractive/DefaultTextPrimitive.cpp
	rendering/noninteractive/DefaultImagePrimitive.cpp
	rendering/noninteractive/DefaultArrowPrimitive.cpp
	rendering/noninteractive/DefaultMeshPrimitive.cpp
	rendering/RenderSettings.cpp
	rendering/RenderSettingsEditor.cpp
	rendering/FrameBuffer.cpp
	viewport/Viewport.cpp
	viewport/ViewportWindow.cpp
	viewport/ViewportMenu.cpp
	viewport/ViewportConfiguration.cpp
	viewport/ViewportSettings.cpp
	viewport/input/ViewportInputManager.cpp
	viewport/input/ViewportInputMode.cpp
	viewport/input/NavigationModes.cpp
	viewport/input/XFormModes.cpp
	viewport/picking/PickingSceneRenderer.cpp
	viewport/overlay/ViewportOverlay.cpp
	viewport/overlay/CoordinateTripodOverlay.cpp
)

# Include video encoder interface code.
IF(LIBAV_FOUND)
	LIST(APPEND SourceFiles utilities/io/video/VideoEncoder.cpp)
ENDIF()

# Compile resources.
QT5_ADD_RESOURCES(ResourceFiles resources/core.qrc)

# Build library
ADD_LIBRARY(Core SHARED Core.cpp ${SourceFiles} ${ResourceFiles})

# Set default include directory.
TARGET_INCLUDE_DIRECTORIES(Core PUBLIC "$<BUILD_INTERFACE:${OVITO_SOURCE_BASE_DIR}/src>")

# Required by built-in SSH client.
TARGET_LINK_LIBRARIES(Core PRIVATE QSsh)

# Link to OpenGL library.
TARGET_LINK_LIBRARIES(Core PRIVATE ${OPENGL_gl_LIBRARY})

# Link to ZLib library.
TARGET_LINK_LIBRARIES(Core PRIVATE ${ZLIB_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(Core PRIVATE "${ZLIB_INCLUDE_DIRS}")

# Add Boost include directories.
TARGET_INCLUDE_DIRECTORIES(Core PUBLIC ${Boost_INCLUDE_DIRS})

# Link Qt5.
TARGET_LINK_LIBRARIES(Core PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::Concurrent)
TARGET_LINK_LIBRARIES(Core PRIVATE Qt5::Network)

# Link video encoding lib.
IF(LIBAV_FOUND)
	TARGET_COMPILE_DEFINITIONS(Core PUBLIC OVITO_VIDEO_OUTPUT_SUPPORT)
	TARGET_LINK_LIBRARIES(Core PRIVATE ${Libav_LIBRARIES})
	TARGET_INCLUDE_DIRECTORIES(Core PRIVATE ${Libav_INCLUDE_DIRS})
ENDIF()

# Define the OVITO_DEBUG macro in debug builds.
TARGET_COMPILE_DEFINITIONS(Core PUBLIC "$<$<CONFIG:Debug>:OVITO_DEBUG>")

# Speed up compilation by using precompiled headers.
ADD_PRECOMPILED_HEADER(Core core/Core.h)

IF(APPLE)
    # Activate @rpath on MacOS.
    SET_TARGET_PROPERTIES(Core PROPERTIES MACOSX_RPATH TRUE)
	# This is required to avoid error by install_name_tool on MacOS.
	SET_TARGET_PROPERTIES(Core PROPERTIES LINK_FLAGS "-headerpad_max_install_names")
ELSEIF(UNIX)
    SET_TARGET_PROPERTIES(Core PROPERTIES INSTALL_RPATH "$ORIGIN")
ENDIF()
    
# This library is part of the installation package.
INSTALL(TARGETS Core EXPORT OVITO
	RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
	LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")

# Export target to make it accessible for external plugins.
IF(CMAKE_VERSION VERSION_LESS "3")
	EXPORT(TARGETS Core NAMESPACE "Ovito::" APPEND FILE "${${PROJECT_NAME}_BINARY_DIR}/OVITOTargets.cmake")
ENDIF()

# Install Libav video encoding libraries.
IF(LIBAV_FOUND)	
	FOREACH(lib ${Libav_LIBRARIES})
		GET_FILENAME_COMPONENT(base_name "${lib}" NAME_WE)
		GET_FILENAME_COMPONENT(lib_path "${lib}" PATH)
		IF(WIN32)
			FILE(GLOB lib_dll "${lib_path}/${base_name}-*.dll")
			IF(NOT lib_dll)
				MESSAGE(FATAL_ERROR "Could not find corresponding DLL for Libav library '${lib}' in same directory.")
			ENDIF()
			FOREACH(dll ${lib_dll})
				OVITO_INSTALL_DLL("${dll}")
			ENDFOREACH()
		ELSEIF(UNIX AND NOT APPLE AND OVITO_MONOLITHIC_BUILD)
			FILE(GLOB lib_versions "${lib}*")
			FOREACH(lib_version ${lib_versions})
				INSTALL(FILES "${lib_version}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
			ENDFOREACH()
		ENDIF()
	ENDFOREACH()
ENDIF()
