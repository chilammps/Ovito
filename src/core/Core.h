///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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

#ifndef __OVITO_CORE_H
#define __OVITO_CORE_H

/******************************************************************************
* The Base module is required by the Core module.
******************************************************************************/
#include <base/Base.h>
#include <base/linalg/LinAlg.h>
#include <base/utilities/Color.h>

#ifdef OVITO_CORE_LIBRARY
#  define OVITO_CORE_EXPORT Q_DECL_EXPORT
#else
#  define OVITO_CORE_EXPORT Q_DECL_IMPORT
#endif

/******************************************************************************
* Forward declarations of often-used classes.
******************************************************************************/
namespace Ovito {

class AnimationSettings;		// defined in AnimationSettings.h
class CloneHelper;				// defined in CloneHelper.h
class DataSet;					// defined in DataSet.h
class DataSetContainer;			// defined in DataSetContainer.h
class DisplayObject;			// defined in DisplayObject.h
class FrameBuffer;				// defined in FrameBuffer.h
class FrameBufferWindow;		// defined in FrameBufferWindow.h
class GroupNode;				// defined in GroupNode.h
class LinkedFileObject;			// defined in LinkedFileObject.h
class LinkedFileImporter;		// defined in LinkedFileImporter.h
class LookAtController;			// defined in LookAtController.h
class MainWindow;				// defined in MainWindow.h
class Modifier;					// defined in Modifier.h
class ModifierApplication;		// defined in ModifierApplication.h
class ObjectNode;				// defined in ObjectNode.h
class ParameterUnit;			// defined in ParameterUnit.h
class PipelineFlowState;		// defined in PipelineFlowState.h
class PipelineObject;			// defined in PipelineObject.h
class Plugin;					// defined in Plugin.h
class PropertiesEditor;			// defined in PropertiesEditor.h
class PropertiesPanel;			// defined in PropertiesPanel.h
class RefMaker;					// defined in RefMaker.h
class RefTarget;				// defined in RefTarget.h
class RenderSettings;			// defined in RenderSettings.h
class SceneNode;				// defined in SceneNode.h
class SceneObject;				// defined in SceneObject.h
class SceneRenderer;			// defined in SceneRenderer.h
class SceneRoot;				// defined in SceneRoot.h
class SelectionSet;				// defined in SelectionSet.h
class TriMesh;					// defined in TriMesh.h
class Viewport;					// defined in Viewport.h
class ViewportConfiguration;	// defined in ViewportConfiguration.h
class ViewportInputManager;		// defined in ViewportInputManager.h
class ViewportInputMode;		// defined in ViewportInputMode.h
class ViewportSceneRenderer;	// defined in ViewportSceneRenderer.h
class ViewportSettings;			// defined in ViewportSettings.h
class VideoEncoder;				// defined in VideoEncoder.h

};

/******************************************************************************
* Include some basic headers.
******************************************************************************/
#include <core/object/OvitoObject.h>

#endif // __OVITO_CORE_H
