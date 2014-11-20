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

	class CloneHelper;
	class DataSet;
	class DataSetContainer;
	class DisplayObject;
	class GroupNode;
	class LinkedFileObject;
	class LinkedFileImporter;
	class Modifier;
	class ModifierApplication;
	class ObjectNode;
	class ParameterUnit;
	class PipelineFlowState;
	class PipelineObject;
	class Plugin;
	class RefMaker;
	class RefTarget;
	class SceneNode;
	class SceneObject;
	class SceneRoot;
	class SelectionSet;
	class TriMesh;
	class VideoEncoder;
	class AutoStartObject;

	namespace Util {
		namespace IO {
			class ObjectSaveStream;
			class ObjectLoadStream;
		}
	}
	namespace Math {
		using namespace Ovito::Util::IO;
	}
	namespace Anim {
		class AnimationSettings;
		class LookAtController;
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
	namespace Gui {
		class MainWindow;
		class Application;
		class ActionManager;
		class FrameBufferWindow;
		class ViewportModeAction;
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
		}
		namespace Internal {
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
		using namespace Gui::Internal;
	}

	// Pull all sub-namespaces into the root Ovito namespace.
	using namespace Util;
	using namespace Util::IO;
	using namespace Math;
	using namespace Rendering;
	using namespace Anim;
	using namespace View;
	using namespace Gui;
	using namespace Gui::Widgets;
	using namespace Gui::Params;
	using namespace Gui::Internal;

}

#endif // __OVITO_FORWARD_DECL_H
