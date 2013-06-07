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

# Include the resource in the Win32 executable.
IF(WIN32)
	SET(WINDOWS_RESOURCES resources/executable/ovito.rc)
ENDIF(WIN32)

SET(OVITO_SOURCE_FILES 
	base/utilities/Exception.cpp
	base/linalg/AffineDecomposition.cpp
	base/io/SaveStream.cpp
	base/io/LoadStream.cpp
	core/object/OvitoObject.cpp
	core/object/OvitoObjectType.cpp
	core/object/NativeOvitoObjectType.cpp
	core/plugins/Plugin.cpp
	core/plugins/NativePlugin.cpp
	core/plugins/PluginManager.cpp
	core/reference/CloneHelper.cpp
	core/reference/PropertyFieldDescriptor.cpp
	core/reference/PropertyField.cpp
	core/reference/RefMaker.cpp
	core/reference/RefTarget.cpp
	core/io/ObjectSaveStream.cpp
	core/io/ObjectLoadStream.cpp
	core/animation/AnimManager.cpp
	core/animation/AnimationSettings.cpp
	core/animation/controller/Controller.cpp
	core/animation/controller/StandardConstControllers.cpp
	core/animation/controller/StandardLinearControllers.cpp
	core/animation/controller/LookAtController.cpp
	core/animation/controller/TransformationController.cpp
	core/dataset/DataSet.cpp
	core/dataset/DataSetManager.cpp
	core/dataset/CurrentSelectionProxy.cpp
	core/scene/SceneNode.cpp
	core/scene/GroupNode.cpp
	core/scene/ObjectNode.cpp
	core/scene/SceneRoot.cpp
	core/scene/SelectionSet.cpp
	core/scene/objects/SceneObject.cpp
	core/scene/pipeline/PipelineFlowState.cpp
	core/scene/pipeline/PipelineObject.cpp
	core/scene/pipeline/Modifier.cpp
	core/scene/pipeline/ModifierApplication.cpp
	core/scene/display/DisplayObject.cpp
	core/gui/app/Application.cpp
	core/gui/mainwin/MainWindow.cpp
	core/gui/mainwin/AnimationTimeSlider.cpp
	core/gui/mainwin/ViewportsPanel.cpp
	core/gui/mainwin/cmdpanel/CommandPanel.cpp
	core/gui/dialogs/ApplicationSettingsDialog.cpp
	core/gui/dialogs/ViewportSettingsPage.cpp
	core/gui/actions/ActionManager.cpp
	core/gui/actions/FileActions.cpp
	core/gui/actions/ViewportActions.cpp
	core/gui/actions/AnimationActions.cpp
	core/gui/actions/ViewportModeAction.cpp
	core/gui/undo/UndoManager.cpp
	core/gui/widgets/RolloutContainer.cpp
	core/gui/widgets/SpinnerWidget.cpp
	core/gui/widgets/ColorPickerWidget.cpp
	core/gui/widgets/AnimationTimeSpinner.cpp
	core/gui/properties/PropertiesEditor.cpp
	core/gui/properties/PropertiesPanel.cpp
	core/gui/properties/ParameterUI.cpp
	core/gui/properties/NumericalParameterUI.cpp
	core/gui/properties/AffineTransformationParameterUI.cpp
	core/gui/properties/BooleanActionParameterUI.cpp
	core/gui/properties/BooleanGroupBoxParameterUI.cpp
	core/gui/properties/BooleanParameterUI.cpp
	core/gui/properties/BooleanRadioButtonParameterUI.cpp
	core/gui/properties/ColorParameterUI.cpp
	core/gui/properties/FilenameParameterUI.cpp
	core/gui/properties/FloatParameterUI.cpp
	core/gui/properties/IntegerParameterUI.cpp
	core/gui/properties/IntegerRadioButtonParameterUI.cpp
	core/gui/properties/RefTargetListParameterUI.cpp
	core/gui/properties/StringParameterUI.cpp
	core/gui/properties/SubObjectParameterUI.cpp
	core/gui/properties/VariantComboBoxParameterUI.cpp
	core/gui/properties/Vector3ParameterUI.cpp
	core/utilities/units/UnitsManager.cpp
	core/rendering/SceneRenderer.cpp
	core/rendering/viewport/ViewportSceneRenderer.cpp
	core/rendering/viewport/ViewportLineGeometryBuffer.cpp
	core/viewport/Viewport.cpp
	core/viewport/ViewportWindow.cpp
	core/viewport/ViewportConfiguration.cpp
	core/viewport/ViewportSettings.cpp
	core/viewport/ViewportManager.cpp
	core/viewport/input/ViewportInputManager.cpp
	core/viewport/input/ViewportInputHandler.cpp
	core/viewport/input/NavigationModes.cpp
	viz/data/SimulationCell.cpp
	viz/data/SimulationCellDisplay.cpp
	main/Main.cpp
)

# Compile resource files.
QT5_ADD_RESOURCES(OVITO_RESOURCE_FILES core/resources/core.qrc)

# Put the executable into the right directory.
SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})

# Builds the main executable of the application
ADD_EXECUTABLE(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${OVITO_SOURCE_FILES} ${OVITO_RESOURCE_FILES} ${WINDOWS_RESOURCES})

# Link to OpenGL library.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})

# Set name of executable.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ovito")

# Link Qt5.
QT5_USE_MODULES(${PROJECT_NAME} Widgets Xml)

# This executable will be part of the installation package.
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

IF(APPLE)
	# Install the Info.plist file.
	CONFIGURE_FILE("resources/executable/Info.plist" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")

	# Copy the application icon into the resource directory.
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/executable/ovito.icns" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}")
ENDIF()
