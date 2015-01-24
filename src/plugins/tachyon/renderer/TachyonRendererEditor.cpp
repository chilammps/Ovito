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
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include "TachyonRenderer.h"
#include "TachyonRendererEditor.h"

namespace Ovito { namespace Tachyon { OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Tachyon, TachyonRendererEditor, PropertiesEditor);

/******************************************************************************
* Creates the UI controls for the editor.
******************************************************************************/
void TachyonRendererEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the rollout.
	QWidget* rollout = createRollout(tr("Tachyon renderer settings"), rolloutParams, "rendering.tachyon_renderer.html");

	QVBoxLayout* mainLayout = new QVBoxLayout(rollout);
	mainLayout->setContentsMargins(4,4,4,4);

	// Antialiasing
	BooleanGroupBoxParameterUI* enableAntialiasingUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_antialiasingEnabled));
	QGroupBox* aaGroupBox = enableAntialiasingUI->groupBox();
	mainLayout->addWidget(aaGroupBox);

	QGridLayout* layout = new QGridLayout(enableAntialiasingUI->childContainer());
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	IntegerParameterUI* aaSamplesUI = new IntegerParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_antialiasingSamples));
	layout->addWidget(aaSamplesUI->label(), 0, 0);
	layout->addLayout(aaSamplesUI->createFieldLayout(), 0, 1);
	aaSamplesUI->setMinValue(1);
	aaSamplesUI->setMaxValue(100);

	BooleanGroupBoxParameterUI* enableDirectLightUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_directLightSourceEnabled));
	QGroupBox* lightsGroupBox = enableDirectLightUI->groupBox();
	mainLayout->addWidget(lightsGroupBox);

	layout = new QGridLayout(enableDirectLightUI->childContainer());
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Default light brightness.
	FloatParameterUI* defaultLightIntensityUI = new FloatParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_defaultLightSourceIntensity));
	defaultLightIntensityUI->label()->setText(tr("Brightness:"));
	layout->addWidget(defaultLightIntensityUI->label(), 0, 0);
	layout->addLayout(defaultLightIntensityUI->createFieldLayout(), 0, 1);
	defaultLightIntensityUI->setMinValue(0);

	// Shadows.
	BooleanParameterUI* enableShadowsUI = new BooleanParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_shadowsEnabled));
	layout->addWidget(enableShadowsUI->checkBox(), 1, 0, 1, 2);

	// Ambient occlusion.
	BooleanGroupBoxParameterUI* enableAmbientOcclusionUI = new BooleanGroupBoxParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionEnabled));
	QGroupBox* aoGroupBox = enableAmbientOcclusionUI->groupBox();
	mainLayout->addWidget(aoGroupBox);

	layout = new QGridLayout(enableAmbientOcclusionUI->childContainer());
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);

	// Ambient occlusion brightness.
	FloatParameterUI* aoBrightnessUI = new FloatParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionBrightness));
	aoBrightnessUI->label()->setText(tr("Brightness:"));
	layout->addWidget(aoBrightnessUI->label(), 0, 0);
	layout->addLayout(aoBrightnessUI->createFieldLayout(), 0, 1);
	aoBrightnessUI->setMinValue(0);

	// Ambient occlusion samples.
	IntegerParameterUI* aoSamplesUI = new IntegerParameterUI(this, PROPERTY_FIELD(TachyonRenderer::_ambientOcclusionSamples));
	aoSamplesUI->label()->setText(tr("Sample count:"));
	layout->addWidget(aoSamplesUI->label(), 1, 0);
	layout->addLayout(aoSamplesUI->createFieldLayout(), 1, 1);
	aoSamplesUI->setMinValue(1);
	aoSamplesUI->setMaxValue(100);

	// Copyright notice
	QWidget* copyrightRollout = createRollout(tr("About"), rolloutParams.collapse().after(rollout));
	mainLayout = new QVBoxLayout(copyrightRollout);
	mainLayout->setContentsMargins(4,4,4,4);
	QLabel* label = new QLabel(tr("This rendering plugin is based on:<br>Tachyon Parallel / Multiprocessor Ray Tracing System<br>Copyright 1994-2013 John E. Stone<br><a href=\"http://jedi.ks.uiuc.edu/~johns/raytracer\">See Tachyon website for more information</a>"));
	label->setWordWrap(true);
	label->setOpenExternalLinks(true);
	mainLayout->addWidget(label);
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
