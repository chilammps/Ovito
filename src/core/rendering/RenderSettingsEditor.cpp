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
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/dialogs/SaveImageFileDialog.h>
#include <core/gui/actions/ActionManager.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/RenderSettingsEditor.h>
#include <core/rendering/SceneRenderer.h>
#include <core/plugins/PluginManager.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Core, RenderSettingsEditor, PropertiesEditor);

// Predefined output image dimensions.
static const int imageSizePresets[][2] = {
		{ 320, 240 },
		{ 640, 480 },
		{ 800, 600 },
		{ 1024, 768 },
		{ 1600, 1200 },
		{ 600, 600 },
		{ 1000, 1000 }
};

/******************************************************************************
* Constructor that creates the UI controls for the editor.
******************************************************************************/
void RenderSettingsEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the rollout.
	QWidget* rollout = createRollout(tr("Render settings"), rolloutParams, "core.render_settings.html");
	
	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);

	// Rendering range
	{
		QGroupBox* groupBox = new QGroupBox(tr("Rendering range"));
		layout->addWidget(groupBox);

		QVBoxLayout* layout2 = new QVBoxLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);
		QGridLayout* layout2c = new QGridLayout();
		layout2c->setContentsMargins(0,0,0,0);
		layout2c->setSpacing(2);
		layout2->addLayout(layout2c);

		IntegerRadioButtonParameterUI* renderingRangeTypeUI = new IntegerRadioButtonParameterUI(this, PROPERTY_FIELD(RenderSettings::_renderingRangeType));

		QRadioButton* currentFrameButton = renderingRangeTypeUI->addRadioButton(RenderSettings::CURRENT_FRAME, tr("Single frame"));
		layout2c->addWidget(currentFrameButton, 0, 0, 1, 5);

		QRadioButton* animationIntervalButton = renderingRangeTypeUI->addRadioButton(RenderSettings::ANIMATION_INTERVAL, tr("Complete animation"));
		layout2c->addWidget(animationIntervalButton, 1, 0, 1, 5);

		QRadioButton* customIntervalButton = renderingRangeTypeUI->addRadioButton(RenderSettings::CUSTOM_INTERVAL, tr("Range:"));
		layout2c->addWidget(customIntervalButton, 2, 0, 1, 5);

		IntegerParameterUI* customRangeStartUI = new IntegerParameterUI(this, PROPERTY_FIELD(RenderSettings::_customRangeStart));
		customRangeStartUI->setEnabled(false);
		layout2c->addLayout(customRangeStartUI->createFieldLayout(), 3, 1);
		layout2c->addWidget(new QLabel(tr("to")), 3, 2);
		IntegerParameterUI* customRangeEndUI = new IntegerParameterUI(this, PROPERTY_FIELD(RenderSettings::_customRangeEnd));
		customRangeEndUI->setEnabled(false);
		layout2c->addLayout(customRangeEndUI->createFieldLayout(), 3, 3);
		layout2c->setColumnMinimumWidth(0, 30);
		layout2c->setColumnStretch(4, 1);
		connect(customIntervalButton, &QRadioButton::toggled, customRangeStartUI, &IntegerParameterUI::setEnabled);
		connect(customIntervalButton, &QRadioButton::toggled, customRangeEndUI, &IntegerParameterUI::setEnabled);

		QGridLayout* layout2a = new QGridLayout();
		layout2a->setContentsMargins(0,6,0,0);
		layout2a->setSpacing(2);
		layout2->addLayout(layout2a);
		IntegerParameterUI* everyNthFrameUI = new IntegerParameterUI(this, PROPERTY_FIELD(RenderSettings::_everyNthFrame));
		layout2a->addWidget(everyNthFrameUI->label(), 0, 0);
		layout2a->addLayout(everyNthFrameUI->createFieldLayout(), 0, 1);
		everyNthFrameUI->setMinValue(1);
		IntegerParameterUI* fileNumberBaseUI = new IntegerParameterUI(this, PROPERTY_FIELD(RenderSettings::_fileNumberBase));
		layout2a->addWidget(fileNumberBaseUI->label(), 1, 0);
		layout2a->addLayout(fileNumberBaseUI->createFieldLayout(), 1, 1);
		layout2a->setColumnStretch(2, 1);
		connect(currentFrameButton, &QRadioButton::toggled, everyNthFrameUI, &IntegerParameterUI::setDisabled);
		connect(currentFrameButton, &QRadioButton::toggled, fileNumberBaseUI, &IntegerParameterUI::setDisabled);
	}

	// Output size
	{
		QGroupBox* groupBox = new QGroupBox(tr("Output image size"));
		layout->addWidget(groupBox);
		QGridLayout* layout2 = new QGridLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);
		layout2->setColumnStretch(1, 1);

		// Image width parameter.
		IntegerParameterUI* imageWidthUI = new IntegerParameterUI(this, PROPERTY_FIELD(RenderSettings::_outputImageWidth));
		layout2->addWidget(imageWidthUI->label(), 0, 0);
		layout2->addLayout(imageWidthUI->createFieldLayout(), 0, 1);
		imageWidthUI->setMinValue(1);
	
		// Image height parameter.
		IntegerParameterUI* imageHeightUI = new IntegerParameterUI(this, PROPERTY_FIELD(RenderSettings::_outputImageHeight));
		layout2->addWidget(imageHeightUI->label(), 1, 0);
		layout2->addLayout(imageHeightUI->createFieldLayout(), 1, 1);
		imageHeightUI->setMinValue(1);

		sizePresetsBox = new QComboBox(groupBox);
		sizePresetsBox->addItem(tr("Presets..."));
		sizePresetsBox->insertSeparator(1);
		for(int i = 0; i < sizeof(imageSizePresets)/sizeof(imageSizePresets[0]); i++)
			sizePresetsBox->addItem(tr("%1 x %2").arg(imageSizePresets[i][0]).arg(imageSizePresets[i][1]));
		connect(sizePresetsBox, (void (QComboBox::*)(int))&QComboBox::activated, this, &RenderSettingsEditor::onSizePresetActivated);
		layout2->addWidget(sizePresetsBox, 0, 2);
	}

	// Render output
	{
		QGroupBox* groupBox = new QGroupBox(tr("Render output"));
		layout->addWidget(groupBox);
		QGridLayout* layout2 = new QGridLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);
		layout2->setColumnStretch(0, 1);

		BooleanParameterUI* saveFileUI = new BooleanParameterUI(this, PROPERTY_FIELD(RenderSettings::_saveToFile));
		layout2->addWidget(saveFileUI->checkBox(), 0, 0);

		QPushButton* chooseFilenameBtn = new QPushButton(tr("Choose..."), rollout);
		connect(chooseFilenameBtn, &QPushButton::clicked, this, &RenderSettingsEditor::onChooseImageFilename);
		layout2->addWidget(chooseFilenameBtn, 0, 1);

		// Output filename parameter.
		StringParameterUI* imageFilenameUI = new StringParameterUI(this, "imageFilename");
		imageFilenameUI->setEnabled(false);
		layout2->addWidget(imageFilenameUI->textBox(), 1, 0, 1, 2);

		//BooleanParameterUI* skipExistingImagesUI = new BooleanParameterUI(this, PROPERTY_FIELD(RenderSettings::_skipExistingImages));
		//layout2->addWidget(skipExistingImagesUI->checkBox(), 2, 0, 1, 2);
		//connect(saveFileUI->checkBox(), &QCheckBox::toggled, skipExistingImagesUI, &BooleanParameterUI::setEnabled);
	}

	// Options
	{
		QGroupBox* groupBox = new QGroupBox(tr("Options"));
		layout->addWidget(groupBox);
		QGridLayout* layout2 = new QGridLayout(groupBox);
		layout2->setContentsMargins(4,4,4,4);
		layout2->setSpacing(2);

		// Background color parameter.
		layout2->addWidget(new QLabel(tr("Background:")), 0, 0, 1, 3);

		ColorParameterUI* backgroundColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(RenderSettings::_backgroundColor));
		layout2->addWidget(backgroundColorPUI->colorPicker(), 1, 1, 1, 2);

		// Alpha channel.
		BooleanRadioButtonParameterUI* generateAlphaUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(RenderSettings::_generateAlphaChannel));
		layout2->addWidget(generateAlphaUI->buttonFalse(), 1, 0, 1, 1);
		layout2->addWidget(generateAlphaUI->buttonTrue(), 2, 0, 1, 3);
		generateAlphaUI->buttonFalse()->setText(tr("Color:"));
		generateAlphaUI->buttonTrue()->setText(tr("Transparent"));

		// Create 'Change renderer' button.
		QPushButton* changeRendererButton = new QPushButton(tr("Change renderer..."), groupBox);
		connect(changeRendererButton, &QPushButton::clicked, this, &RenderSettingsEditor::onChangeRenderer);
		layout2->setRowMinimumHeight(3, 8);
		layout2->addWidget(changeRendererButton, 4, 0, 1, 3);
	}

	// Open a sub-editor for the renderer.
	new SubObjectParameterUI(this, PROPERTY_FIELD(RenderSettings::_renderer), rolloutParams.after(rollout));
}

/******************************************************************************
* Lets the user choose a filename for the output image.
******************************************************************************/
void RenderSettingsEditor::onChooseImageFilename()
{
	RenderSettings* settings = static_object_cast<RenderSettings>(editObject());
	if(!settings) return;

	SaveImageFileDialog fileDialog(container(), tr("Output image file"), true, settings->imageInfo());
	if(fileDialog.exec()) {
		undoableTransaction(tr("Change output file"), [settings, &fileDialog]() {
			settings->setImageInfo(fileDialog.imageInfo());
			settings->setSaveToFile(true);
		});
	}
}

/******************************************************************************
* Is called when the user selects an output size preset from the drop-down list.
******************************************************************************/
void RenderSettingsEditor::onSizePresetActivated(int index)
{
	RenderSettings* settings = static_object_cast<RenderSettings>(editObject());
	if(settings && index >= 2 && index < 2+sizeof(imageSizePresets)/sizeof(imageSizePresets[0])) {
		undoableTransaction(tr("Change output dimensions"), [settings, index]() {
			settings->setOutputImageWidth(imageSizePresets[index-2][0]);
			settings->setOutputImageHeight(imageSizePresets[index-2][1]);
		});
	}
	sizePresetsBox->setCurrentIndex(0);
}

/******************************************************************************
* Lets the user choose a different plug-in rendering engine.
******************************************************************************/
void RenderSettingsEditor::onChangeRenderer()
{
	RenderSettings* settings = static_object_cast<RenderSettings>(editObject());
	if(!settings) return;

	QStringList itemList;
	QVector<OvitoObjectType*> rendererClasses = PluginManager::instance().listClasses(SceneRenderer::OOType);
	Q_FOREACH(OvitoObjectType* clazz, rendererClasses)
		itemList << clazz->displayName();

	int currentIndex = 0;
	if(settings->renderer())
		currentIndex = itemList.indexOf(settings->renderer()->getOOType().displayName());

	bool ok;
	QString selectedClass = QInputDialog::getItem(NULL, tr("Choose renderer"), tr("Select the new rendering engine:"), itemList, currentIndex, false, &ok);
	if(!ok) return;

	int newIndex = itemList.indexOf(selectedClass);
	if(newIndex != currentIndex && newIndex >= 0) {
		undoableTransaction(tr("Change renderer"), [settings, newIndex, &rendererClasses]() {
			OORef<SceneRenderer> renderer = static_object_cast<SceneRenderer>(rendererClasses[newIndex]->createInstance(settings->dataset()));
			renderer->loadUserDefaults();
			settings->setRenderer(renderer);
		});
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
