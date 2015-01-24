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

#include <core/Core.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include "StandardSceneRenderer.h"
#include "StandardSceneRendererEditor.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Core, StandardSceneRendererEditor, PropertiesEditor);

/******************************************************************************
* Constructor that creates the UI controls for the editor.
******************************************************************************/
void StandardSceneRendererEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the rollout.
	QWidget* rollout = createRollout(tr("OpenGL renderer settings"), rolloutParams, "rendering.opengl_renderer.html");
	
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
#ifndef Q_OS_MACX
	layout->setSpacing(2);
#endif
	layout->setColumnStretch(1, 1);
	
	// Antialiasing level	
	IntegerParameterUI* antialiasingLevelUI = new IntegerParameterUI(this, PROPERTY_FIELD(StandardSceneRenderer::_antialiasingLevel));
	layout->addWidget(antialiasingLevelUI->label(), 0, 0);
	layout->addLayout(antialiasingLevelUI->createFieldLayout(), 0, 1);
	antialiasingLevelUI->setMinValue(1);
	antialiasingLevelUI->setMaxValue(6);	
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
