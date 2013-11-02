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

SET(OVITO_CORE_SOURCE_FILES 
	object/OvitoObject.cpp
	object/OvitoObjectType.cpp
	object/NativeOvitoObjectType.cpp
	plugins/Plugin.cpp
	plugins/NativePlugin.cpp
	plugins/PluginManager.cpp
	reference/CloneHelper.cpp
	reference/PropertyFieldDescriptor.cpp
	reference/PropertyField.cpp
	reference/RefMaker.cpp
	reference/RefTarget.cpp
	reference/RefTargetListener.cpp
	animation/AnimManager.cpp
	animation/AnimationSettings.cpp
	animation/controller/Controller.cpp
	animation/controller/StandardConstControllers.cpp
	animation/controller/StandardLinearControllers.cpp
	animation/controller/LookAtController.cpp
	animation/controller/TransformationController.cpp
	dataset/DataSet.cpp
	dataset/DataSetManager.cpp
	dataset/CurrentSelectionProxy.cpp
	dataset/importexport/ImportExportManager.cpp
	dataset/importexport/LinkedFileImporter.cpp
	dataset/importexport/LinkedFileObject.cpp
	dataset/importexport/LinkedFileObjectEditor.cpp
	scene/SceneNode.cpp
	scene/GroupNode.cpp
	scene/ObjectNode.cpp
	scene/SceneRoot.cpp
	scene/SelectionSet.cpp
	scene/objects/SceneObject.cpp
	scene/objects/geometry/TriMesh.cpp
	scene/objects/geometry/TriMeshObject.cpp
	scene/objects/camera/CameraObject.cpp
	scene/pipeline/PipelineFlowState.cpp
	scene/pipeline/PipelineObject.cpp
	scene/pipeline/Modifier.cpp
	scene/pipeline/ModifierApplication.cpp
	scene/display/DisplayObject.cpp
	scene/display/geometry/TriMeshDisplay.cpp
	scene/display/camera/CameraDisplayObject.cpp
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
	gui/mainwin/cmdpanel/UtilityApplet.cpp
	gui/dialogs/AdjustCameraDialog.cpp
	gui/dialogs/AnimationSettingsDialog.cpp
	gui/dialogs/ApplicationSettingsDialog.cpp
	gui/dialogs/ViewportSettingsPage.cpp
	gui/dialogs/HistoryFileDialog.cpp
	gui/dialogs/ImportFileDialog.cpp
	gui/dialogs/ImportRemoteFileDialog.cpp
	gui/dialogs/SaveImageFileDialog.cpp
	gui/dialogs/RemoteAuthenticationDialog.cpp
	gui/actions/ActionManager.cpp
	gui/actions/FileActions.cpp
	gui/actions/ViewportActions.cpp
	gui/actions/AnimationActions.cpp
	gui/actions/RenderActions.cpp
	gui/actions/ViewportModeAction.cpp
	gui/undo/UndoManager.cpp
	gui/widgets/general/RolloutContainer.cpp
	gui/widgets/general/SpinnerWidget.cpp
	gui/widgets/general/ColorPickerWidget.cpp
	gui/widgets/general/ElidedTextLabel.cpp
	gui/widgets/general/AutocompleteLineEdit.cpp
	gui/widgets/animation/AnimationTimeSpinner.cpp
	gui/widgets/animation/AnimationFramesToolButton.cpp
	gui/widgets/animation/AnimationTimeSlider.cpp
	gui/widgets/rendering/FrameBufferWidget.cpp
	gui/widgets/rendering/FrameBufferWindow.cpp
	gui/widgets/selection/SceneNodesListModel.cpp
	gui/widgets/selection/SceneNodeSelectionBox.cpp
	gui/widgets/display/ObjectStatusWidget.cpp
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
	gui/properties/FilenameParameterUI.cpp
	gui/properties/FloatParameterUI.cpp
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
	utilities/io/CompressedTextParserStream.cpp
	utilities/concurrent/Future.cpp
	utilities/concurrent/ProgressManager.cpp
	utilities/opengl/SharedOpenGLResource.cpp
	rendering/SceneRenderer.cpp
	rendering/viewport/ViewportSceneRenderer.cpp
	rendering/viewport/ViewportLineGeometryBuffer.cpp
	rendering/viewport/ViewportParticleGeometryBuffer.cpp
	rendering/viewport/ViewportTextGeometryBuffer.cpp
	rendering/viewport/ViewportImageGeometryBuffer.cpp
	rendering/viewport/ViewportArrowGeometryBuffer.cpp
	rendering/viewport/ViewportTriMeshGeometryBuffer.cpp
	rendering/standard/StandardSceneRenderer.cpp
	rendering/standard/StandardSceneRendererEditor.cpp
	rendering/noninteractive/NonInteractiveSceneRenderer.cpp
	rendering/noninteractive/DefaultLineGeometryBuffer.cpp
	rendering/noninteractive/DefaultParticleGeometryBuffer.cpp
	rendering/noninteractive/DefaultTextGeometryBuffer.cpp
	rendering/noninteractive/DefaultImageGeometryBuffer.cpp
	rendering/noninteractive/DefaultArrowGeometryBuffer.cpp
	rendering/noninteractive/DefaultTriMeshGeometryBuffer.cpp
	rendering/RenderSettings.cpp
	rendering/RenderSettingsEditor.cpp
	rendering/FrameBuffer.cpp
	viewport/Viewport.cpp
	viewport/ViewportWindow.cpp
	viewport/ViewportMenu.cpp
	viewport/ViewportConfiguration.cpp
	viewport/ViewportSettings.cpp
	viewport/ViewportManager.cpp
	viewport/input/ViewportInputManager.cpp
	viewport/input/ViewportInputHandler.cpp
	viewport/input/NavigationModes.cpp
	viewport/picking/PickingSceneRenderer.cpp
)

# Compile resource files.
QT5_ADD_RESOURCES(OVITO_CORE_RESOURCE_FILES resources/core.qrc)

# This will let the source code export symbols when building a DLL.
ADD_DEFINITIONS(-DOVITO_CORE_LIBRARY)

# Build library
ADD_LIBRARY(Core ${OVITO_CORE_SOURCE_FILES} ${OVITO_CORE_RESOURCE_FILES})
TARGET_LINK_LIBRARIES(Core Base QSsh)
IF(OVITO_VIDEO_OUTPUT_SUPPORT)
	TARGET_LINK_LIBRARIES(Core Video)
ENDIF()

# Link to OpenGL library.
TARGET_LINK_LIBRARIES(Core ${OPENGL_gl_LIBRARY})

# Link Qt5.
QT5_USE_MODULES(Core ${OVITO_REQUIRED_QT_MODULES})

# This is required to avoid error by install_name_tool on MacOS.
IF(APPLE)
	SET_TARGET_PROPERTIES(Core PROPERTIES LINK_FLAGS "-headerpad_max_install_names")
ENDIF(APPLE)

# This library is part of the installation package.
IF(APPLE OR WIN32)
	INSTALL(TARGETS Core 
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()
