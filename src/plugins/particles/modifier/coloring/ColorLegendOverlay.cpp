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

#include <plugins/particles/Particles.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/RenderSettings.h>
#include <core/gui/properties/BooleanGroupBoxParameterUI.h>
#include <core/gui/properties/StringParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/FontParameterUI.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/gui/properties/CustomParameterUI.h>
#include <core/dataset/DataSet.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/pipeline/PipelineObject.h>
#include "ColorLegendOverlay.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorLegendOverlay, ViewportOverlay);
SET_OVITO_OBJECT_EDITOR(ColorLegendOverlay, ColorLegendOverlayEditor);
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _alignment, "Alignment", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _orientation, "Orientation", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _legendSize, "Size", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _font, "Font", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _fontSize, "FontSize", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ColorLegendOverlay, _offsetX, "OffsetX");
DEFINE_PROPERTY_FIELD(ColorLegendOverlay, _offsetY, "OffsetY");
DEFINE_PROPERTY_FIELD(ColorLegendOverlay, _title, "Title");
DEFINE_PROPERTY_FIELD(ColorLegendOverlay, _label1, "Label1");
DEFINE_PROPERTY_FIELD(ColorLegendOverlay, _label2, "Label2");
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _aspectRatio, "AspectRatio", PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(ColorLegendOverlay, _valueFormatString, "ValueFormatString");
DEFINE_FLAGS_REFERENCE_FIELD(ColorLegendOverlay, _modifier, "Modifier", ColorCodingModifier, PROPERTY_FIELD_NO_SUB_ANIM);
DEFINE_FLAGS_PROPERTY_FIELD(ColorLegendOverlay, _textColor, "TextColor", PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _alignment, "Position");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _orientation, "Orientation");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _legendSize, "Size factor");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _font, "Font");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _fontSize, "Font size");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _offsetX, "Offset X");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _offsetY, "Offset Y");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _aspectRatio, "Aspect ratio");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _textColor, "Font color");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _title, "Title");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _label1, "Label 1");
SET_PROPERTY_FIELD_LABEL(ColorLegendOverlay, _label2, "Label 2");
SET_PROPERTY_FIELD_UNITS(ColorLegendOverlay, _offsetX, PercentParameterUnit);
SET_PROPERTY_FIELD_UNITS(ColorLegendOverlay, _offsetY, PercentParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ColorLegendOverlayEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructor.
******************************************************************************/
ColorLegendOverlay::ColorLegendOverlay(DataSet* dataset) : ViewportOverlay(dataset),
		_alignment(Qt::AlignHCenter | Qt::AlignBottom), _orientation(Qt::Horizontal),
		_legendSize(0.3), _offsetX(0), _offsetY(0),
		_fontSize(0.1), _valueFormatString("%g"), _aspectRatio(8.0),
		_textColor(0,0,0)
{
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_alignment);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_orientation);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_legendSize);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_offsetX);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_offsetY);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_aspectRatio);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_font);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_fontSize);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_title);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_label1);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_label2);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_valueFormatString);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_modifier);
	INIT_PROPERTY_FIELD(ColorLegendOverlay::_textColor);

	// Find a ColorCodingModifiers in the scene that we can connect to.
	dataset->sceneRoot()->visitObjectNodes([this](ObjectNode* node) {
		DataObject* obj = node->dataProvider();
		while(obj) {
			if(PipelineObject* pipeline = dynamic_object_cast<PipelineObject>(obj)) {
				for(ModifierApplication* modApp : pipeline->modifierApplications()) {
					if(ColorCodingModifier* mod = dynamic_object_cast<ColorCodingModifier>(modApp->modifier())) {
						setModifier(mod);
						if(mod->isEnabled())
							return false;	// Stop search.
					}
				}
				obj = pipeline->sourceObject();
			}
			else break;
		}
		return true;
	});
}

/******************************************************************************
* This method asks the overlay to paint its contents over the given viewport.
******************************************************************************/
void ColorLegendOverlay::render(Viewport* viewport, QPainter& painter, const ViewProjectionParameters& projParams, RenderSettings* renderSettings)
{
	if(!modifier()) return;

	FloatType legendSize = _legendSize.value() * renderSettings->outputImageHeight();
	if(legendSize <= 0) return;

	FloatType colorBarWidth = legendSize;
	FloatType colorBarHeight = colorBarWidth / std::max(FloatType(0.01), _aspectRatio.value());
	bool vertical = (_orientation.value() == Qt::Vertical);
	if(vertical)
		std::swap(colorBarWidth, colorBarHeight);

	QPointF origin(_offsetX.value() * renderSettings->outputImageWidth(), -_offsetY.value() * renderSettings->outputImageHeight());
	FloatType hmargin = 0.01 * renderSettings->outputImageWidth();
	FloatType vmargin = 0.01 * renderSettings->outputImageHeight();

	if(_alignment.value() & Qt::AlignLeft) origin.rx() += hmargin;
	else if(_alignment.value() & Qt::AlignRight) origin.rx() += renderSettings->outputImageWidth() - hmargin - colorBarWidth;
	else if(_alignment.value() & Qt::AlignHCenter) origin.rx() += 0.5 * renderSettings->outputImageWidth() - 0.5 * colorBarWidth;

	if(_alignment.value() & Qt::AlignTop) origin.ry() += vmargin;
	else if(_alignment.value() & Qt::AlignBottom) origin.ry() += renderSettings->outputImageHeight() - vmargin - colorBarHeight;
	else if(_alignment.value() & Qt::AlignVCenter) origin.ry() += 0.5 * renderSettings->outputImageHeight() - 0.5 * colorBarHeight;

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

	// Create the color scale image.
	int imageSize = 256;
	QImage image(vertical ? 1 : imageSize, vertical ? imageSize : 1, QImage::Format_RGB32);
	for(int i = 0; i < imageSize; i++) {
		FloatType t = (FloatType)i / (FloatType)(imageSize - 1);
		Color color = modifier()->colorGradient()->valueToColor(vertical ? (FloatType(1) - t) : t);
		image.setPixel(vertical ? 0 : i, vertical ? i : 0, QColor(color).rgb());
	}
	painter.drawImage(QRectF(origin, QSizeF(colorBarWidth, colorBarHeight)), image);

	qreal fontSize = legendSize * std::max(0.0, (double)_fontSize.value());
	if(fontSize == 0) return;
	QFont font = _font.value();
	painter.setPen((QColor)_textColor.value());

	// Get modifier's parameters.
	FloatType startValue = modifier()->startValue();
	FloatType endValue = modifier()->endValue();

	QByteArray format = valueFormatString().toLatin1();
	if(format.contains("%s")) format.clear();

	QString titleLabel, topLabel, bottomLabel;
	if(label1().isEmpty())
		topLabel.sprintf(format.constData(), endValue);
	else
		topLabel = label1();
	if(label2().isEmpty())
		bottomLabel.sprintf(format.constData(), startValue);
	else
		bottomLabel = label2();
	if(title().isEmpty())
		titleLabel = modifier()->sourceProperty().nameWithComponent();
	else
		titleLabel = title();

	font.setPointSizeF(fontSize);
	painter.setFont(font);

	qreal textMargin = 0.2 * legendSize / std::max(FloatType(0.01), _aspectRatio.value());
	if(!vertical || (_alignment.value() & Qt::AlignHCenter)) {
		painter.drawText(QRectF(origin.x() + 0.5 * colorBarWidth, origin.y() - 0.5 * textMargin, 0, 0), Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip | Qt::TextSingleLine, titleLabel);
	}
	else {
		if(_alignment.value() & Qt::AlignLeft)
			painter.drawText(QRectF(origin.x(), origin.y() - textMargin, 0, 0), Qt::AlignLeft | Qt::AlignBottom | Qt::TextDontClip | Qt::TextSingleLine, titleLabel);
		else if(_alignment.value() & Qt::AlignRight)
			painter.drawText(QRectF(origin.x() + colorBarWidth, origin.y() - textMargin, 0, 0), Qt::AlignRight | Qt::AlignBottom | Qt::TextDontClip | Qt::TextSingleLine, titleLabel);
	}

	font.setPointSizeF(fontSize * 0.8);
	painter.setFont(font);

	if(!vertical) {
		painter.drawText(QRectF(origin.x() - textMargin, origin.y() + 0.5 * colorBarHeight, 0, 0), Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine, bottomLabel);
		painter.drawText(QRectF(origin.x() + colorBarWidth + textMargin, origin.y() + 0.5 * colorBarHeight, 0, 0), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine, topLabel);
	}
	else {
		if(_alignment.value() & Qt::AlignLeft) {
			painter.drawText(QRectF(origin.x() + colorBarWidth + textMargin, origin.y(), 0, 0), Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine, topLabel);
			painter.drawText(QRectF(origin.x() + colorBarWidth + textMargin, origin.y() + colorBarHeight, 0, 0), Qt::AlignLeft | Qt::AlignBottom | Qt::TextDontClip | Qt::TextSingleLine, bottomLabel);
		}
		else if(_alignment.value() & Qt::AlignRight) {
			painter.drawText(QRectF(origin.x() - textMargin, origin.y(), 0, 0), Qt::AlignRight | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine, topLabel);
			painter.drawText(QRectF(origin.x() - textMargin, origin.y() + colorBarHeight, 0, 0), Qt::AlignRight | Qt::AlignBottom | Qt::TextDontClip | Qt::TextSingleLine, bottomLabel);
		}
		else if(_alignment.value() & Qt::AlignHCenter) {
			painter.drawText(QRectF(origin.x() + colorBarWidth + textMargin, origin.y(), 0, 0), Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip | Qt::TextSingleLine, topLabel);
			painter.drawText(QRectF(origin.x() + colorBarWidth + textMargin, origin.y() + colorBarHeight, 0, 0), Qt::AlignLeft | Qt::AlignBottom | Qt::TextDontClip | Qt::TextSingleLine, bottomLabel);
		}
	}
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ColorLegendOverlayEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Color legend"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(4);
	layout->setColumnStretch(1, 1);
	int row = 0;

	// This widget displays the list of available ColorCodingModifiers in the current scene.
	class ModifierComboBox : public QComboBox {
	public:
		/// Initializes the widget.
		ModifierComboBox(QWidget* parent = nullptr) : QComboBox(parent), _overlay(nullptr) {}

		/// Sets the overlay being edited.
		void setOverlay(ColorLegendOverlay* overlay) { _overlay = overlay; }

		/// Is called just before the drop-down box is activated.
		virtual void showPopup() override {
			clear();
			if(_overlay) {
				// Find all ColorCodingModifiers in the scene. For this we have to visit all
				// object nodes and iterate over their modification pipelines.
				_overlay->dataset()->sceneRoot()->visitObjectNodes([this](ObjectNode* node) {
					DataObject* obj = node->dataProvider();
					while(obj) {
						if(PipelineObject* pipeline = dynamic_object_cast<PipelineObject>(obj)) {
							for(ModifierApplication* modApp : pipeline->modifierApplications()) {
								if(ColorCodingModifier* mod = dynamic_object_cast<ColorCodingModifier>(modApp->modifier())) {
									addItem(mod->sourceProperty().nameWithComponent(), QVariant::fromValue(mod));
								}
							}
							obj = pipeline->sourceObject();
						}
						else break;
					}
					return true;
				});
				setCurrentIndex(findData(QVariant::fromValue(_overlay->modifier())));
			}
			if(count() == 0) addItem(tr("<none>"));
			QComboBox::showPopup();
		}

	private:
		ColorLegendOverlay* _overlay;
	};

	ModifierComboBox* modifierComboBox = new ModifierComboBox();
	CustomParameterUI* modifierPUI = new CustomParameterUI(this, "modifier", modifierComboBox,
			[modifierComboBox](const QVariant& value) {
				modifierComboBox->clear();
				ColorCodingModifier* mod = dynamic_object_cast<ColorCodingModifier>(value.value<ColorCodingModifier*>());
				if(mod)
					modifierComboBox->addItem(mod->sourceProperty().nameWithComponent(), QVariant::fromValue(mod));
				else
					modifierComboBox->addItem(tr("<none>"));
				modifierComboBox->setCurrentIndex(0);
			},
			[modifierComboBox]() {
				return modifierComboBox->currentData();
			},
			[modifierComboBox](RefTarget* editObject) {
				modifierComboBox->setOverlay(dynamic_object_cast<ColorLegendOverlay>(editObject));
			});
	connect(modifierComboBox, (void (QComboBox::*)(int))&QComboBox::activated, modifierPUI, &CustomParameterUI::updatePropertyValue);
	layout->addWidget(new QLabel(tr("Source modifier:")), row, 0);
	layout->addWidget(modifierPUI->widget(), row++, 1);

	QGroupBox* positionBox = new QGroupBox(tr("Position"));
	layout->addWidget(positionBox, row++, 0, 1, 2);
	QGridLayout* sublayout = new QGridLayout(positionBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(1, 1);

	VariantComboBoxParameterUI* alignmentPUI = new VariantComboBoxParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_alignment));
	sublayout->addWidget(alignmentPUI->comboBox(), 0, 0);
	alignmentPUI->comboBox()->addItem(tr("Top"), QVariant::fromValue((int)(Qt::AlignTop | Qt::AlignHCenter)));
	alignmentPUI->comboBox()->addItem(tr("Top left"), QVariant::fromValue((int)(Qt::AlignTop | Qt::AlignLeft)));
	alignmentPUI->comboBox()->addItem(tr("Top right"), QVariant::fromValue((int)(Qt::AlignTop | Qt::AlignRight)));
	alignmentPUI->comboBox()->addItem(tr("Bottom"), QVariant::fromValue((int)(Qt::AlignBottom | Qt::AlignHCenter)));
	alignmentPUI->comboBox()->addItem(tr("Bottom left"), QVariant::fromValue((int)(Qt::AlignBottom | Qt::AlignLeft)));
	alignmentPUI->comboBox()->addItem(tr("Bottom right"), QVariant::fromValue((int)(Qt::AlignBottom | Qt::AlignRight)));
	alignmentPUI->comboBox()->addItem(tr("Left"), QVariant::fromValue((int)(Qt::AlignVCenter | Qt::AlignLeft)));
	alignmentPUI->comboBox()->addItem(tr("Right"), QVariant::fromValue((int)(Qt::AlignVCenter | Qt::AlignRight)));

	VariantComboBoxParameterUI* orientationPUI = new VariantComboBoxParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_orientation));
	sublayout->addWidget(orientationPUI->comboBox(), 0, 1);
	orientationPUI->comboBox()->addItem(tr("Vertical"), QVariant::fromValue((int)Qt::Vertical));
	orientationPUI->comboBox()->addItem(tr("Horizontal"), QVariant::fromValue((int)Qt::Horizontal));

	FloatParameterUI* offsetXPUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_offsetX));
	sublayout->addWidget(offsetXPUI->label(), 1, 0);
	sublayout->addLayout(offsetXPUI->createFieldLayout(), 1, 1);

	FloatParameterUI* offsetYPUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_offsetY));
	sublayout->addWidget(offsetYPUI->label(), 2, 0);
	sublayout->addLayout(offsetYPUI->createFieldLayout(), 2, 1);

	QGroupBox* sizeBox = new QGroupBox(tr("Size"));
	layout->addWidget(sizeBox, row++, 0, 1, 2);
	sublayout = new QGridLayout(sizeBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(1, 1);

	FloatParameterUI* sizePUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_legendSize));
	sublayout->addWidget(sizePUI->label(), 0, 0);
	sublayout->addLayout(sizePUI->createFieldLayout(), 0, 1);
	sizePUI->setMinValue(0);

	FloatParameterUI* aspectRatioPUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_aspectRatio));
	sublayout->addWidget(aspectRatioPUI->label(), 1, 0);
	sublayout->addLayout(aspectRatioPUI->createFieldLayout(), 1, 1);
	aspectRatioPUI->setMinValue(1.0);

	QGroupBox* labelBox = new QGroupBox(tr("Labels"));
	layout->addWidget(labelBox, row++, 0, 1, 2);
	sublayout = new QGridLayout(labelBox);
	sublayout->setContentsMargins(4,4,4,4);
	sublayout->setSpacing(4);
	sublayout->setColumnStretch(1, 3);
	sublayout->setColumnStretch(2, 1);

	StringParameterUI* titlePUI = new StringParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_title));
	sublayout->addWidget(new QLabel(tr("Custom title:")), 0, 0);
	sublayout->addWidget(titlePUI->textBox(), 0, 1, 1, 2);

	StringParameterUI* label1PUI = new StringParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_label1));
	sublayout->addWidget(new QLabel(tr("Custom label 1:")), 1, 0);
	sublayout->addWidget(label1PUI->textBox(), 1, 1, 1, 2);

	StringParameterUI* label2PUI = new StringParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_label2));
	sublayout->addWidget(new QLabel(tr("Custom label 2:")), 2, 0);
	sublayout->addWidget(label2PUI->textBox(), 2, 1, 1, 2);

	StringParameterUI* valueFormatStringPUI = new StringParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_valueFormatString));
	sublayout->addWidget(new QLabel(tr("Format string:")), 3, 0);
	sublayout->addWidget(valueFormatStringPUI->textBox(), 3, 1, 1, 2);

	FloatParameterUI* fontSizePUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_fontSize));
	sublayout->addWidget(new QLabel(tr("Text size/color:")), 4, 0);
	sublayout->addLayout(fontSizePUI->createFieldLayout(), 4, 1);
	fontSizePUI->setMinValue(0);

	ColorParameterUI* textColorPUI = new ColorParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_textColor));
	sublayout->addWidget(textColorPUI->colorPicker(), 4, 2);

	FontParameterUI* labelFontPUI = new FontParameterUI(this, PROPERTY_FIELD(ColorLegendOverlay::_font));
	sublayout->addWidget(labelFontPUI->label(), 5, 0);
	sublayout->addWidget(labelFontPUI->fontPicker(), 5, 1, 1, 2);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
