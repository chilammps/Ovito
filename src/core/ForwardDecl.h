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

/**
 * \file
 * \brief Contains forward declarations of OVITO's classes and namespaces.
 */

#ifndef __OVITO_FORWARD_DECL_H
#define __OVITO_FORWARD_DECL_H

namespace Ovito {

	inline namespace Util {
		inline namespace IO {
			class ObjectSaveStream;
			class ObjectLoadStream;
			inline namespace Internal {
				class VideoEncoder;
				class SftpDownloadJob;
			}
		}
		inline namespace Concurrency {
		}
		inline namespace Mesh {
			class TriMesh;
			class HalfEdgeMesh;
		}
		inline namespace Math {
		}
	}
	inline namespace Anim {
		class AnimationSettings;
		class LookAtController;
	}
	inline namespace PluginSystem {
		class Plugin;
		class PluginManager;
		class AutoStartObject;
		class UtilityApplet;
		inline namespace Internal {
			class NativePlugin;
		}
	}
	inline namespace ObjectSystem {
		class OvitoObject;
		class OvitoObjectType;
		class CloneHelper;
		class RefMaker;
		class RefTarget;
		class PropertyFieldDescriptor;
		class DataSet;
		class DataSetContainer;
		inline namespace Units {
			class ParameterUnit;
		}
		inline namespace Undo {
			class UndoStack;
			class UndoableOperation;
		}
		inline namespace Scene {
			class SceneNode;
			class CompoundObject;
			class DataObject;
			class SceneRoot;
			class SelectionSet;
			class Modifier;
			class ModifierApplication;
			class ObjectNode;
			class PipelineFlowState;
			class PipelineObject;
			class DisplayObject;
			inline namespace StdObj {
			}
		}
		inline namespace Internal {
			class NativeOvitoObjectType;
		}
	}
	inline namespace Rendering {
		class SceneRenderer;
		class ObjectPickInfo;
		class RenderSettings;
		class FrameBuffer;
		class ViewportSceneRenderer;
		inline namespace Internal {
		}
	}
	inline namespace View {
		class Viewport;
		class ViewportConfiguration;
		class ViewportSettings;
		struct ViewProjectionParameters;
		class ViewportOverlay;
		inline namespace Internal {
			class PickingSceneRenderer;
		}
	}
	inline namespace DataIO {
		class FileImporter;
		class FileExporter;
		class FileSource;
		class FileSourceImporter;
	}
	inline namespace Gui {
		class MainWindow;
		class Application;
		class ActionManager;
		inline namespace Widgets {
			class PropertiesPanel;
			class SpinnerWidget;
			class ColorPickerWidget;
			class RolloutContainer;
			class FrameBufferWindow;
			class FrameBufferWidget;
		}
		inline namespace Params {
			class PropertiesEditor;
		}
		inline namespace ViewportInput {
			class ViewportInputManager;
			class ViewportInputMode;
			class ViewportModeAction;
		}
		inline namespace Dialogs {
		}
		inline namespace Internal {
			class CoordinateDisplayWidget;
			class CommandPanel;
			class ModifyCommandPage;
			class RenderCommandPage;
			class OverlayCommandPage;
			class UtilityCommandPage;
			class ViewportMenu;
			class ViewportWindow;
		}
	}

	// This should only be visible to Doxygen:
#ifdef DOXYGEN_SHOULD_SKIP_THIS
	using namespace Util;
	using namespace Util::IO;
	using namespace Util::Math;
	using namespace Util::Mesh;
	using namespace Util::Concurrency;
	using namespace Rendering;
	using namespace View;
	using namespace DataIO;
	using namespace Anim;
	using namespace Gui;
	using namespace Gui::Params;
	using namespace Gui::Dialogs;
	using namespace Gui::ViewportInput;
	using namespace Gui::Widgets;
	using namespace PluginSystem;
	using namespace ObjectSystem;
	using namespace ObjectSystem::Units;
	using namespace ObjectSystem::Undo;
	using namespace ObjectSystem::Scene;
	using namespace ObjectSystem::Scene::StdObj;
#endif
}

#endif // __OVITO_FORWARD_DECL_H
