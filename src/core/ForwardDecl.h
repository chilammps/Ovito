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
 * \brief Contains forward declarations of OVITO's classes.
 */

#ifndef __OVITO_FORWARD_DECL_H
#define __OVITO_FORWARD_DECL_H

namespace Ovito {

	namespace Util {
		namespace IO {
			class ObjectSaveStream;
			class ObjectLoadStream;
			namespace Internal {
				class VideoEncoder;
				class SftpDownloadJob;
			}
		}
		namespace Concurrency {
		}
		namespace Mesh {
			class TriMesh;
			class HalfEdgeMesh;
		}
	}
	namespace Math {
		using namespace Ovito::Util::IO;
	}
	namespace Anim {
		class AnimationSettings;
		class LookAtController;
	}
	namespace PluginSystem {
		class Plugin;
		class PluginManager;
		class AutoStartObject;
		class UtilityApplet;
		namespace Internal {
			class NativePlugin;
		}
		using namespace Internal;
	}
	namespace ObjectSystem {
		class OvitoObject;
		class OvitoObjectType;
		class CloneHelper;
		class RefMaker;
		class RefTarget;
		class PropertyFieldDescriptor;
		class DataSet;
		class DataSetContainer;
		namespace Units {
			class ParameterUnit;
		}
		namespace Undo {
			class UndoStack;
			class UndoableOperation;
		}
		namespace Scene {
			class SceneNode;
			class SceneObject;
			class SceneRoot;
			class SelectionSet;
			class Modifier;
			class ModifierApplication;
			class ObjectNode;
			class PipelineFlowState;
			class PipelineObject;
			class DisplayObject;
			namespace StdObj {
			}
			using namespace StdObj;
		}
		namespace Internal {
			class NativeOvitoObjectType;
		}
		using namespace Units;
		using namespace Undo;
		using namespace Scene;
		using namespace Internal;
	}
	namespace Rendering {
		class SceneRenderer;
		class ObjectPickInfo;
		class RenderSettings;
		class FrameBuffer;
		class ViewportSceneRenderer;
		namespace Internal {
		}
		using namespace Internal;
	}
	namespace View {
		class Viewport;
		class ViewportConfiguration;
		class ViewportSettings;
		struct ViewProjectionParameters;
		class ViewportOverlay;
		namespace Internal {
			class PickingSceneRenderer;
		}
		using namespace Internal;
	}
	namespace DataIO {
		class LinkedFileObject;
		class LinkedFileImporter;
	}
	namespace Gui {
		class MainWindow;
		class Application;
		class ActionManager;
		namespace Widgets {
			class PropertiesPanel;
			class SpinnerWidget;
			class ColorPickerWidget;
			class RolloutContainer;
		}
		namespace Params {
			class PropertiesEditor;
		}
		namespace View {
			class ViewportInputManager;
			class ViewportInputMode;
			class ViewportModeAction;
		}
		namespace Dialogs {
		}
		namespace Internal {
			class FrameBufferWindow;
			class FrameBufferWidget;
			class CoordinateDisplayWidget;
			class CommandPanel;
			class ModifyCommandPage;
			class RenderCommandPage;
			class OverlayCommandPage;
			class UtilityCommandPage;
			class ViewportMenu;
			class ViewportWindow;
		}
		using namespace Gui::Widgets;
		using namespace Gui::Params;
		using namespace Gui::View;
		using namespace Gui::Dialogs;
		using namespace Gui::Internal;
	}

	// Pull all sub-namespaces into the root Ovito namespace.
	using namespace Util;
	using namespace Util::IO;
	using namespace Util::Concurrency;
	using namespace Util::Mesh;
	using namespace Math;
	using namespace Rendering;
	using namespace Anim;
	using namespace View;
	using namespace ObjectSystem;
	using namespace ObjectSystem::Undo;
	using namespace ObjectSystem::Scene;
	using namespace ObjectSystem::Scene::StdObj;
	using namespace PluginSystem;
	using namespace DataIO;
	using namespace Gui;
	using namespace Gui::Widgets;
	using namespace Gui::Params;
	using namespace Gui::Internal;

}

#endif // __OVITO_FORWARD_DECL_H
