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
	scene/pipeline/PipelineFlowState.cpp
	scene/pipeline/PipelineObject.cpp
	scene/pipeline/Modifier.cpp
	scene/pipeline/ModifierApplication.cpp
	scene/display/DisplayObject.cpp
	gui/app/Application.cpp
	gui/mainwin/MainWindow.cpp
	gui/mainwin/AnimationTimeSlider.cpp
	gui/mainwin/ViewportsPanel.cpp
	gui/mainwin/cmdpanel/CommandPanel.cpp
	gui/mainwin/cmdpanel/RenderCommandPage.cpp
	gui/mainwin/cmdpanel/ModifyCommandPage.cpp
	gui/mainwin/cmdpanel/ModifierStack.cpp
	gui/mainwin/cmdpanel/UtilityCommandPage.cpp
	gui/mainwin/cmdpanel/UtilityApplet.cpp
	gui/dialogs/ApplicationSettingsDialog.cpp
	gui/dialogs/ViewportSettingsPage.cpp
	gui/dialogs/HistoryFileDialog.cpp
	gui/dialogs/ImportFileDialog.cpp
	gui/dialogs/SaveImageFileDialog.cpp
	gui/actions/ActionManager.cpp
	gui/actions/FileActions.cpp
	gui/actions/ViewportActions.cpp
	gui/actions/AnimationActions.cpp
	gui/actions/ViewportModeAction.cpp
	gui/undo/UndoManager.cpp
	gui/widgets/RolloutContainer.cpp
	gui/widgets/SpinnerWidget.cpp
	gui/widgets/ColorPickerWidget.cpp
	gui/widgets/AnimationTimeSpinner.cpp
	gui/widgets/ElidedTextLabel.cpp
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
	utilities/io/gzstream/gzstream.cpp
	utilities/io/gzdevice/qtiocompressor.cpp
	utilities/concurrent/Future.cpp
	utilities/concurrent/ProgressManager.cpp
	rendering/SceneRenderer.cpp
	rendering/viewport/ViewportSceneRenderer.cpp
	rendering/viewport/ViewportSceneRendererEditor.cpp
	rendering/viewport/ViewportLineGeometryBuffer.cpp
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
)

# Compile resource files.
QT5_ADD_RESOURCES(OVITO_CORE_RESOURCE_FILES resources/core.qrc)

# Find the ZLib library.
FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})

# Build library
ADD_LIBRARY(Core ${OVITO_CORE_SOURCE_FILES} ${OVITO_CORE_RESOURCE_FILES})

TARGET_LINK_LIBRARIES(Core Base)

# Link to OpenGL library.
TARGET_LINK_LIBRARIES(Core ${OPENGL_gl_LIBRARY})

# Link to ZLib library.
TARGET_LINK_LIBRARIES(Core ${ZLIB_LIBRARIES})

# Link Qt5.
QT5_USE_MODULES(Core Widgets Xml Network)
