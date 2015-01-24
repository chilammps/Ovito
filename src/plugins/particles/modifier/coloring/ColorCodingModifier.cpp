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

#include <plugins/particles/Particles.h>
#include <core/viewport/Viewport.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/controller/Controller.h>
#include <core/reference/CloneHelper.h>
#include <core/gui/mainwin/MainWindow.h>
#include <core/gui/dialogs/LoadImageFileDialog.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/CustomParameterUI.h>
#include <core/plugins/PluginManager.h>
#include <core/gui/dialogs/SaveImageFileDialog.h>
#include <core/rendering/SceneRenderer.h>
#include <core/viewport/ViewportConfiguration.h>
#include <plugins/particles/util/ParticlePropertyParameterUI.h>
#include "ColorCodingModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(ColorCodingModifier, ColorCodingModifierEditor);
DEFINE_REFERENCE_FIELD(ColorCodingModifier, _startValueCtrl, "StartValue", Controller);
DEFINE_REFERENCE_FIELD(ColorCodingModifier, _endValueCtrl, "EndValue", Controller);
DEFINE_REFERENCE_FIELD(ColorCodingModifier, _colorGradient, "ColorGradient", ColorCodingGradient);
DEFINE_PROPERTY_FIELD(ColorCodingModifier, _colorOnlySelected, "SelectedOnly");
DEFINE_PROPERTY_FIELD(ColorCodingModifier, _keepSelection, "KeepSelection");
DEFINE_PROPERTY_FIELD(ColorCodingModifier, _sourceProperty, "SourceProperty");
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _startValueCtrl, "Start value");
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _endValueCtrl, "End value");
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _colorGradient, "Color gradient");
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _colorOnlySelected, "Color only selected particles");
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _keepSelection, "Keep particles selected");
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _sourceProperty, "Source property");

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingGradient, RefTarget);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingHSVGradient, ColorCodingGradient);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingGrayscaleGradient, ColorCodingGradient);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingHotGradient, ColorCodingGradient);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingJetGradient, ColorCodingGradient);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ColorCodingImageGradient, ColorCodingGradient);
DEFINE_PROPERTY_FIELD(ColorCodingImageGradient, _image, "Image");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ColorCodingModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ColorCodingModifier::ColorCodingModifier(DataSet* dataset) : ParticleModifier(dataset),
	_colorOnlySelected(false), _keepSelection(false)
{
	INIT_PROPERTY_FIELD(ColorCodingModifier::_startValueCtrl);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_endValueCtrl);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_colorGradient);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_colorOnlySelected);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_keepSelection);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_sourceProperty);

	_colorGradient = new ColorCodingHSVGradient(dataset);
	_startValueCtrl = ControllerManager::instance().createFloatController(dataset);
	_endValueCtrl = ControllerManager::instance().createFloatController(dataset);
}

/******************************************************************************
* Loads the user-defined default values of this object's parameter fields from the
* application's settings store.
******************************************************************************/
void ColorCodingModifier::loadUserDefaults()
{
	ParticleModifier::loadUserDefaults();

	// Load the default gradient type set by the user.
	QSettings settings;
	settings.beginGroup(ColorCodingModifier::OOType.plugin()->pluginId());
	settings.beginGroup(ColorCodingModifier::OOType.name());
	QString typeString = settings.value(PROPERTY_FIELD(ColorCodingModifier::_colorGradient).identifier()).toString();
	if(!typeString.isEmpty()) {
		try {
			OvitoObjectType* gradientType = OvitoObjectType::decodeFromString(typeString);
			if(!colorGradient() || colorGradient()->getOOType() != *gradientType) {
				OORef<ColorCodingGradient> gradient = dynamic_object_cast<ColorCodingGradient>(gradientType->createInstance(dataset()));
				if(gradient) setColorGradient(gradient);
			}
		}
		catch(...) {}
	}
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval ColorCodingModifier::modifierValidity(TimePoint time)
{
	TimeInterval interval = ParticleModifier::modifierValidity(time);
	if(_startValueCtrl) interval.intersect(_startValueCtrl->validityInterval(time));
	if(_endValueCtrl) interval.intersect(_endValueCtrl->validityInterval(time));
	return interval;
}

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a pipeline.
******************************************************************************/
void ColorCodingModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);
	if(sourceProperty().isNull()) {
		// Select the first available particle property from the input.
		PipelineFlowState input = pipeline->evaluatePipeline(dataset()->animationSettings()->time(), modApp, false);
		ParticlePropertyReference bestProperty;
		for(DataObject* o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o);
			if(property && (property->dataType() == qMetaTypeId<int>() || property->dataType() == qMetaTypeId<FloatType>())) {
				bestProperty = ParticlePropertyReference(property, (property->componentCount() > 1) ? 0 : -1);
			}
		}
		if(!bestProperty.isNull())
			setSourceProperty(bestProperty);
	}

	// Automatically adjust value range.
	if(startValue() == 0 && endValue() == 0)
		adjustRange();
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
PipelineStatus ColorCodingModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	// Get the source property.
	if(sourceProperty().isNull())
		throw Exception(tr("Select a particle property first."));
	ParticlePropertyObject* property = sourceProperty().findInState(input());
	if(!property)
		throw Exception(tr("The particle property with the name '%1' does not exist.").arg(sourceProperty().name()));
	if(sourceProperty().vectorComponent() >= (int)property->componentCount())
		throw Exception(tr("The vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(sourceProperty().name()).arg(property->componentCount()));

	int vecComponent = std::max(0, sourceProperty().vectorComponent());
	int stride = property->stride() / property->dataTypeSize();

	if(!_colorGradient)
		throw Exception(tr("No color gradient has been selected."));

	// Get modifier's parameter values.
	FloatType startValue = 0, endValue = 0;
	if(_startValueCtrl) startValue = _startValueCtrl->getFloatValue(time, validityInterval);
	if(_endValueCtrl) endValue = _endValueCtrl->getFloatValue(time, validityInterval);

	// Get the particle selection property if enabled by the user.
	ParticlePropertyObject* selProperty = nullptr;
	const int* sel = nullptr;
	std::vector<Color> existingColors;
	if(colorOnlySelected()) {
		selProperty = inputStandardProperty(ParticleProperty::SelectionProperty);
		if(selProperty) {
			sel = selProperty->constDataInt();
			existingColors = inputParticleColors(time, validityInterval);
		}
	}

	// Create the color output property.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);
	OVITO_ASSERT(colorProperty->size() == property->size());

	Color* c_begin = colorProperty->dataColor();
	Color* c_end = c_begin + colorProperty->size();
	Color* c = c_begin;

	if(property->dataType() == qMetaTypeId<FloatType>()) {
		const FloatType* v = property->constDataFloat() + vecComponent;
		for(; c != c_end; ++c, v += stride) {

			// If the "only selected" option is enabled, and the particle is not selected, use the existing particle color.
			if(sel && !(*sel++)) {
				*c = existingColors[c - c_begin];
				continue;
			}

			// Compute linear interpolation.
			FloatType t;
			if(startValue == endValue) {
				if((*v) == startValue) t = 0.5;
				else if((*v) > startValue) t = 1.0;
				else t = 0.0;
			}
			else t = ((*v) - startValue) / (endValue - startValue);

			// Clamp values.
			if(t < 0) t = 0;
			else if(t > 1) t = 1;

			*c = _colorGradient->valueToColor(t);
		}
	}
	else if(property->dataType() == qMetaTypeId<int>()) {
		const int* v = property->constDataInt() + vecComponent;
		for(; c != c_end; ++c, v += stride) {

			// If the "only selected" option is enabled, and the particle is not selected, use the existing particle color.
			if(sel && !(*sel++)) {
				*c = existingColors[c - c_begin];
				continue;
			}

			// Compute linear interpolation.
			FloatType t;
			if(startValue == endValue) {
				if((*v) == startValue) t = 0.5;
				else if((*v) > startValue) t = 1.0;
				else t = 0.0;
			}
			else t = ((*v) - startValue) / (endValue - startValue);

			// Clamp values.
			if(t < 0) t = 0;
			else if(t > 1) t = 1;

			*c = _colorGradient->valueToColor(t);
		}
	}
	else
		throw Exception(tr("The particle property '%1' has an invalid or non-numeric data type.").arg(property->name()));

	// Clear particle selection if requested.
	if(selProperty && !keepSelection())
		output().removeObject(selProperty);

	colorProperty->changed();
	return PipelineStatus::Success;
}

/******************************************************************************
* Sets the start and end value to the minimum and maximum value
* in the selected particle property.
******************************************************************************/
bool ColorCodingModifier::adjustRange()
{
	// Determine the minimum and maximum values of the selected particle property.

	// Get the value data channel from the input object.
	PipelineFlowState inputState = getModifierInput();
	ParticlePropertyObject* property = sourceProperty().findInState(inputState);
	if(!property)
		return false;

	if(sourceProperty().vectorComponent() >= (int)property->componentCount())
		return false;
	int vecComponent = std::max(0, sourceProperty().vectorComponent());
	int stride = property->stride() / property->dataTypeSize();

	// Iterate over all atoms.
	FloatType maxValue = FLOATTYPE_MIN;
	FloatType minValue = FLOATTYPE_MAX;
	if(property->dataType() == qMetaTypeId<FloatType>()) {
		const FloatType* v = property->constDataFloat() + vecComponent;
		const FloatType* vend = v + (property->size() * stride);
		for(; v != vend; v += stride) {
			if(*v > maxValue) maxValue = *v;
			if(*v < minValue) minValue = *v;
		}
	}
	else if(property->dataType() == qMetaTypeId<int>()) {
		const int* v = property->constDataInt() + vecComponent;
		const int* vend = v + (property->size() * stride);
		for(; v != vend; v += stride) {
			if(*v > maxValue) maxValue = *v;
			if(*v < minValue) minValue = *v;
		}
	}
	if(minValue == +FLOATTYPE_MAX)
		return false;

	if(startValueController())
		startValueController()->setCurrentFloatValue(minValue);
	if(endValueController())
		endValueController()->setCurrentFloatValue(maxValue);

	return true;
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ColorCodingModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);

	stream.beginChunk(0x02);
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ColorCodingModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);

	int version = stream.expectChunkRange(0, 0x02);
	if(version == 0x01) {
		ParticlePropertyReference pref;
		stream >> pref;
		setSourceProperty(pref);
	}
	stream.closeChunk();
}

/******************************************************************************
* Loads the given image file from disk.
******************************************************************************/
void ColorCodingImageGradient::loadImage(const QString& filename)
{
	QImage image(filename);
	if(image.isNull())
		throw Exception(tr("Could not load image file '%1'.").arg(filename));
	setImage(image);
}

/******************************************************************************
* Converts a scalar value to a color value.
******************************************************************************/
Color ColorCodingImageGradient::valueToColor(FloatType t)
{
	if(image().isNull()) return Color(0,0,0);
	QPoint p;
	if(image().width() > image().height())
		p = QPoint(std::min((int)(t * image().width()), image().width()-1), 0);
	else
		p = QPoint(0, std::min((int)(t * image().height()), image().height()-1));
	return Color(image().pixel(p));
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ColorCodingModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Color coding"), rolloutParams, "particles.modifiers.color_coding.html");

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(2);

	ParticlePropertyParameterUI* sourcePropertyUI = new ParticlePropertyParameterUI(this, PROPERTY_FIELD(ColorCodingModifier::_sourceProperty));
	layout1->addWidget(new QLabel(tr("Property:"), rollout));
	layout1->addWidget(sourcePropertyUI->comboBox());

	colorGradientList = new QComboBox(rollout);
	layout1->addWidget(new QLabel(tr("Color gradient:"), rollout));
	layout1->addWidget(colorGradientList);
	colorGradientList->setIconSize(QSize(48,16));
	connect(colorGradientList, (void (QComboBox::*)(int))&QComboBox::activated, this, &ColorCodingModifierEditor::onColorGradientSelected);
	for(const OvitoObjectType* clazz : PluginManager::instance().listClasses(ColorCodingGradient::OOType)) {
		if(clazz == &ColorCodingImageGradient::OOType)
			continue;
		colorGradientList->addItem(iconFromColorMapClass(clazz), clazz->displayName(), QVariant::fromValue(clazz));
	}
	colorGradientList->insertSeparator(colorGradientList->count());
	colorGradientList->addItem(tr("Load custom color map..."));
	_gradientListContainCustomItem = false;

	// Update color legend if another modifier has been loaded into the editor.
	connect(this, &ColorCodingModifierEditor::contentsReplaced, this, &ColorCodingModifierEditor::updateColorGradient);

	layout1->addSpacing(10);

	QGridLayout* layout2 = new QGridLayout();
	layout2->setContentsMargins(0,0,0,0);
	layout2->setColumnStretch(1, 1);
	layout1->addLayout(layout2);

	// End value parameter.
	FloatParameterUI* endValuePUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorCodingModifier::_endValueCtrl));
	layout2->addWidget(endValuePUI->label(), 0, 0);
	layout2->addLayout(endValuePUI->createFieldLayout(), 0, 1);

	// Insert color legend display.
	colorLegendLabel = new QLabel(rollout);
	colorLegendLabel->setScaledContents(true);
	layout2->addWidget(colorLegendLabel, 1, 1);

	// Start value parameter.
	FloatParameterUI* startValuePUI = new FloatParameterUI(this, PROPERTY_FIELD(ColorCodingModifier::_startValueCtrl));
	layout2->addWidget(startValuePUI->label(), 2, 0);
	layout2->addLayout(startValuePUI->createFieldLayout(), 2, 1);

	// Export color scale button.
	QToolButton* exportBtn = new QToolButton(rollout);
	exportBtn->setIcon(QIcon(":/particles/icons/export_color_scale.png"));
	exportBtn->setToolTip("Export color map to image file");
	exportBtn->setAutoRaise(true);
	exportBtn->setIconSize(QSize(42,22));
	connect(exportBtn, &QPushButton::clicked, this, &ColorCodingModifierEditor::onExportColorScale);
	layout2->addWidget(exportBtn, 1, 0, Qt::AlignCenter);

	layout1->addSpacing(8);
	QPushButton* adjustBtn = new QPushButton(tr("Adjust range"), rollout);
	connect(adjustBtn, &QPushButton::clicked, this, &ColorCodingModifierEditor::onAdjustRange);
	layout1->addWidget(adjustBtn);
	layout1->addSpacing(4);
	QPushButton* reverseBtn = new QPushButton(tr("Reverse range"), rollout);
	connect(reverseBtn, &QPushButton::clicked, this, &ColorCodingModifierEditor::onReverseRange);
	layout1->addWidget(reverseBtn);

	layout1->addSpacing(8);

	// Only selected particles.
	BooleanParameterUI* onlySelectedPUI = new BooleanParameterUI(this, PROPERTY_FIELD(ColorCodingModifier::_colorOnlySelected));
	layout1->addWidget(onlySelectedPUI->checkBox());

	// Keep selection
	BooleanParameterUI* keepSelectionPUI = new BooleanParameterUI(this, PROPERTY_FIELD(ColorCodingModifier::_keepSelection));
	layout1->addWidget(keepSelectionPUI->checkBox());
	connect(onlySelectedPUI->checkBox(), &QCheckBox::toggled, keepSelectionPUI, &BooleanParameterUI::setEnabled);
	keepSelectionPUI->setEnabled(false);
}

/******************************************************************************
* Updates the display for the color gradient.
******************************************************************************/
void ColorCodingModifierEditor::updateColorGradient()
{
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	if(!mod) return;

	// Create the color legend image.
	int legendHeight = 128;
	QImage image(1, legendHeight, QImage::Format_RGB32);
	for(int y = 0; y < legendHeight; y++) {
		FloatType t = (FloatType)y / (legendHeight - 1);
		Color color = mod->colorGradient()->valueToColor(1.0 - t);
		image.setPixel(0, y, QColor(color).rgb());
	}
	colorLegendLabel->setPixmap(QPixmap::fromImage(image));

	// Select the right entry in the color gradient selector.
	bool isCustomMap = false;
	if(mod->colorGradient()) {
		int index = colorGradientList->findData(QVariant::fromValue(&mod->colorGradient()->getOOType()));
		if(index >= 0)
			colorGradientList->setCurrentIndex(index);
		else
			isCustomMap = true;
	}
	else colorGradientList->setCurrentIndex(-1);

	if(isCustomMap) {
		if(!_gradientListContainCustomItem) {
			_gradientListContainCustomItem = true;
			colorGradientList->insertItem(colorGradientList->count() - 2, iconFromColorMap(mod->colorGradient()), tr("Custom color map"));
			colorGradientList->insertSeparator(colorGradientList->count() - 3);
		}
		else {
			colorGradientList->setItemIcon(colorGradientList->count() - 3, iconFromColorMap(mod->colorGradient()));
		}
		colorGradientList->setCurrentIndex(colorGradientList->count() - 3);
	}
	else if(_gradientListContainCustomItem) {
		_gradientListContainCustomItem = false;
		colorGradientList->removeItem(colorGradientList->count() - 3);
		colorGradientList->removeItem(colorGradientList->count() - 3);
	}
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool ColorCodingModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::ReferenceChanged &&
			static_cast<ReferenceFieldEvent*>(event)->field() == PROPERTY_FIELD(ColorCodingModifier::_colorGradient)) {
		updateColorGradient();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the user selects a color gradient in the list box.
******************************************************************************/
void ColorCodingModifierEditor::onColorGradientSelected(int index)
{
	if(index < 0) return;
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	OVITO_CHECK_OBJECT_POINTER(mod);

	const OvitoObjectType* descriptor = colorGradientList->itemData(index).value<const OvitoObjectType*>();
	if(descriptor) {
		undoableTransaction(tr("Change color gradient"), [descriptor, mod]() {
			OORef<ColorCodingGradient> gradient = static_object_cast<ColorCodingGradient>(descriptor->createInstance(mod->dataset()));
			if(gradient) {
				mod->setColorGradient(gradient);

				QSettings settings;
				settings.beginGroup(ColorCodingModifier::OOType.plugin()->pluginId());
				settings.beginGroup(ColorCodingModifier::OOType.name());
				settings.setValue(PROPERTY_FIELD(ColorCodingModifier::_colorGradient).identifier(),
						QVariant::fromValue(OvitoObjectType::encodeAsString(descriptor)));
			}
		});
	}
	else if(index == colorGradientList->count() - 1) {
		undoableTransaction(tr("Change color gradient"), [this, mod]() {
			LoadImageFileDialog fileDialog(container(), tr("Pick color map image"));
			if(fileDialog.exec()) {
				OORef<ColorCodingImageGradient> gradient(new ColorCodingImageGradient(mod->dataset()));
				gradient->loadImage(fileDialog.imageInfo().filename());
				mod->setColorGradient(gradient);
			}
		});
	}
}

/******************************************************************************
* Is called when the user presses the "Adjust Range" button.
******************************************************************************/
void ColorCodingModifierEditor::onAdjustRange()
{
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	OVITO_CHECK_OBJECT_POINTER(mod);

	undoableTransaction(tr("Adjust range"), [mod]() {
		mod->adjustRange();
	});
}

/******************************************************************************
* Is called when the user presses the "Reverse Range" button.
******************************************************************************/
void ColorCodingModifierEditor::onReverseRange()
{
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());

	if(mod->startValueController() && mod->endValueController()) {
		undoableTransaction(tr("Reverse range"), [mod]() {

			// Swap controllers for start and end value.
			OORef<Controller> oldStartValue = mod->startValueController();
			mod->setStartValueController(mod->endValueController());
			mod->setEndValueController(oldStartValue);

		});
	}
}

/******************************************************************************
* Is called when the user presses the "Export color scale" button.
******************************************************************************/
void ColorCodingModifierEditor::onExportColorScale()
{
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	if(!mod || !mod->colorGradient()) return;

	SaveImageFileDialog fileDialog(colorLegendLabel, tr("Save color map"));
	if(fileDialog.exec()) {

		// Create the color legend image.
		int legendWidth = 32;
		int legendHeight = 256;
		QImage image(1, legendHeight, QImage::Format_RGB32);
		for(int y = 0; y < legendHeight; y++) {
			FloatType t = (FloatType)y / (FloatType)(legendHeight - 1);
			Color color = mod->colorGradient()->valueToColor(1.0 - t);
			image.setPixel(0, y, QColor(color).rgb());
		}

		QString imageFilename = fileDialog.imageInfo().filename();
		if(!image.scaled(legendWidth, legendHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation).save(imageFilename, fileDialog.imageInfo().format())) {
			Exception ex(tr("Failed to save image to file '%1'.").arg(imageFilename));
			ex.showError();
		}
	}
}

/******************************************************************************
* Returns an icon representing the given color map class.
******************************************************************************/
QIcon ColorCodingModifierEditor::iconFromColorMapClass(const OvitoObjectType* clazz)
{
	/// Cache icons for color map types.
	static std::map<const OvitoObjectType*, QIcon> iconCache;
	auto entry = iconCache.find(clazz);
	if(entry != iconCache.end())
		return entry->second;

	DataSet* dataset = mainWindow()->datasetContainer().currentSet();
	OVITO_ASSERT(dataset);
	if(dataset) {
		try {
			// Create a temporary instance of the color map class.
			OORef<ColorCodingGradient> map = static_object_cast<ColorCodingGradient>(clazz->createInstance(dataset));
			if(map) {
				QIcon icon = iconFromColorMap(map);
				iconCache.insert(std::make_pair(clazz, icon));
				return icon;
			}
		}
		catch(...) {}
	}
	return QIcon();
}

/******************************************************************************
* Returns an icon representing the given color map.
******************************************************************************/
QIcon ColorCodingModifierEditor::iconFromColorMap(ColorCodingGradient* map)
{
	const int sizex = 48;
	const int sizey = 16;
	QImage image(sizex, sizey, QImage::Format_RGB32);
	for(int x = 0; x < sizex; x++) {
		FloatType t = (FloatType)x / (sizex - 1);
		uint c = QColor(map->valueToColor(t)).rgb();
		for(int y = 0; y < sizey; y++)
			image.setPixel(x, y, c);
	}
	return QIcon(QPixmap::fromImage(image));
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
