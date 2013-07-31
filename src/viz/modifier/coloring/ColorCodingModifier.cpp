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
#include <core/viewport/Viewport.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/animation/controller/StandardControllers.h>
#include <core/reference/CloneHelper.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/gui/properties/ColorParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/plugins/PluginManager.h>
#include <core/gui/dialogs/SaveImageFileDialog.h>
#include <core/rendering/SceneRenderer.h>
#include "ColorCodingModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ColorCodingModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, ColorCodingModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(ColorCodingModifier, ColorCodingModifierEditor)
DEFINE_REFERENCE_FIELD(ColorCodingModifier, _startValueCtrl, "StartValue", FloatController)
DEFINE_REFERENCE_FIELD(ColorCodingModifier, _endValueCtrl, "EndValue", FloatController)
DEFINE_REFERENCE_FIELD(ColorCodingModifier, _colorGradient, "ColorGradient", ColorCodingGradient)
DEFINE_PROPERTY_FIELD(ColorCodingModifier, _renderLegend, "RenderLegend")
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _startValueCtrl, "Start value")
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _endValueCtrl, "End value")
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _colorGradient, "Color gradient")
SET_PROPERTY_FIELD_LABEL(ColorCodingModifier, _renderLegend, "Display color legend (experimental)")

IMPLEMENT_OVITO_OBJECT(Viz, ColorCodingGradient, RefTarget)
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ColorCodingHSVGradient, ColorCodingGradient)
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ColorCodingGrayscaleGradient, ColorCodingGradient)
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ColorCodingHotGradient, ColorCodingGradient)
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ColorCodingJetGradient, ColorCodingGradient)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ColorCodingModifier::ColorCodingModifier() : _renderLegend(false)
{
	INIT_PROPERTY_FIELD(ColorCodingModifier::_startValueCtrl);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_endValueCtrl);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_colorGradient);
	INIT_PROPERTY_FIELD(ColorCodingModifier::_renderLegend);

	_colorGradient = new ColorCodingHSVGradient();
	_startValueCtrl = ControllerManager::instance().createDefaultController<FloatController>();
	_endValueCtrl = ControllerManager::instance().createDefaultController<FloatController>();
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
* Sets the source particle property that is used for coloring of particles.
******************************************************************************/
void ColorCodingModifier::setSourceProperty(const ParticlePropertyReference& prop)
{
	if(_sourcePropertyRef == prop) return;

	// Make this change undoable.
	qRegisterMetaType<ParticlePropertyReference>();
	if(UndoManager::instance().isRecording())
		UndoManager::instance().push(new SimplePropertyChangeOperation(this, "sourceProperty"));

	_sourcePropertyRef = prop;
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Retrieves the selected input particle property from the given modifier input state.
******************************************************************************/
ParticlePropertyObject* ColorCodingModifier::lookupInputProperty(const PipelineFlowState& inputState) const
{
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* prop = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(prop) {
			if((sourceProperty().type() == ParticleProperty::UserProperty && prop->name() == sourceProperty().name()) ||
					(sourceProperty().type() != ParticleProperty::UserProperty && prop->type() == sourceProperty().type())) {
				return prop;
			}
		}
	}
	return nullptr;
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
		PipelineFlowState input = pipeline->evaluatePipeline(AnimManager::instance().time(), modApp, false);
		ParticlePropertyReference bestProperty;
		for(const auto& o : input.objects()) {
			ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
			if(property && (property->dataType() == qMetaTypeId<int>() || property->dataType() == qMetaTypeId<FloatType>())) {
				bestProperty = ParticlePropertyReference(property, (property->componentCount() > 1) ? 0 : -1);
			}
		}
		if(!bestProperty.isNull()) {
			setSourceProperty(bestProperty);
			adjustRange();
		}
	}
}

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus ColorCodingModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	if(sourceProperty().isNull())
		throw Exception(tr("Select a particle property first."));

	ParticlePropertyObject* property = lookupInputProperty(input());
	if(!property)
		throw Exception(tr("The selected particle property with the name '%1' does not exist.").arg(sourceProperty().name()));
	if(sourceProperty().vectorComponent() >= (int)property->componentCount())
		throw Exception(tr("The selected vector component is out of range. The particle property '%1' contains only %2 values per particle.").arg(sourceProperty().name()).arg(property->componentCount()));

	int vecComponent = sourceProperty().vectorComponent() >= 0 ? sourceProperty().vectorComponent() : 0;
	int vecComponentCount = property->componentCount();

	if(!_colorGradient)
		throw Exception(tr("No color gradient has been selected."));

	// Get modifier's parameter values.
	FloatType startValue = 0, endValue = 0;
	if(_startValueCtrl) _startValueCtrl->getValue(time, startValue, validityInterval);
	if(_endValueCtrl) _endValueCtrl->getValue(time, endValue, validityInterval);

	// Get the deep copy of the color output property.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);

	OVITO_ASSERT(colorProperty->size() == property->size());
	if(property->dataType() == qMetaTypeId<FloatType>()) {
		const FloatType* v = property->constDataFloat() + vecComponent;
		Color* c = colorProperty->dataColor();
		Color* c_end = c + colorProperty->size();
		for(; c != c_end; ++c, v += vecComponentCount) {

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
		Color* c = colorProperty->dataColor();
		Color* c_end = c + colorProperty->size();
		for(; c != c_end; ++c, v += vecComponentCount) {

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

	colorProperty->changed();
	return ObjectStatus::Success;
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
	ParticlePropertyObject* property = lookupInputProperty(inputState);
	if(!property)
		return false;

	if(sourceProperty().vectorComponent() >= (int)property->componentCount())
		return false;
	int vecComponent = sourceProperty().vectorComponent() >= 0 ? sourceProperty().vectorComponent() : 0;
	int vecComponentCount = property->componentCount();

	// Iterate over all atoms.
	FloatType maxValue = -FLOATTYPE_MAX;
	FloatType minValue = +FLOATTYPE_MAX;
	if(property->dataType() == qMetaTypeId<FloatType>()) {
		const FloatType* v = property->constDataFloat() + vecComponent;
		const FloatType* vend = v + (property->size() * vecComponentCount);
		for(; v != vend; v += vecComponentCount) {
			if(*v > maxValue) maxValue = *v;
			if(*v < minValue) minValue = *v;
		}
	}
	else if(property->dataType() == qMetaTypeId<int>()) {
		const int* v = property->constDataInt() + vecComponent;
		const int* vend = v + (property->size() * vecComponentCount);
		for(; v != vend; v += vecComponentCount) {
			if(*v > maxValue) maxValue = *v;
			if(*v < minValue) minValue = *v;
		}
	}
	if(minValue == +FLOATTYPE_MAX)
		return false;

	if(startValueController())
		startValueController()->setCurrentValue(minValue);
	if(endValueController())
		endValueController()->setCurrentValue(maxValue);

	return true;
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ColorCodingModifier::saveToStream(ObjectSaveStream& stream)
{
	ParticleModifier::saveToStream(stream);

	stream.beginChunk(0x01);
	stream << _sourcePropertyRef;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ColorCodingModifier::loadFromStream(ObjectLoadStream& stream)
{
	ParticleModifier::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream >> _sourcePropertyRef;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> ColorCodingModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<ColorCodingModifier> clone = static_object_cast<ColorCodingModifier>(ParticleModifier::clone(deepCopy, cloneHelper));
	clone->_sourcePropertyRef = this->_sourcePropertyRef;

	return clone;
}

/******************************************************************************
* Lets the modifier render itself into the viewport.
******************************************************************************/
void ColorCodingModifier::render(TimePoint time, ObjectNode* contextNode, ModifierApplication* modApp, SceneRenderer* renderer, bool renderOverlay)
{
	if(!renderOverlay || !isEnabled() || !_renderLegend)
		return;

	if(!colorGradient())
		return;

	// Get modifier's parameter values.
	TimeInterval validityInterval;
	FloatType startValue = 0, endValue = 0;
	if(_startValueCtrl) _startValueCtrl->getValue(time, startValue, validityInterval);
	if(_endValueCtrl) _endValueCtrl->getValue(time, endValue, validityInterval);

	QString topLabel = QString::number(endValue);
	QString bottomLabel = QString::number(startValue);
	QString titleLabel = _sourcePropertyRef.name();

	if(_renderBufferUpdateHelper.updateState(colorGradient())
			|| !_colorScaleImageBuffer
			|| !_colorScaleImageBuffer->isValid(renderer)) {

		// Create the color legend image.
		int imageHeight = 256;
		QImage image(1, imageHeight, QImage::Format_RGB32);
		for(int y = 0; y < image.height(); y++) {
			FloatType t = (FloatType)y / (FloatType)(imageHeight - 1);
			Color color = colorGradient()->valueToColor(1.0 - t);
			image.setPixel(0, y, QColor(color).rgb());
		}

		_colorScaleImageBuffer = renderer->createImageGeometryBuffer();
		_colorScaleImageBuffer->setImage(image);
	}

	FloatType legendSize = 0.4;
	FloatType topMargin = 0.1;
	FloatType rightMargin = 0.03;

	_colorScaleImageBuffer->renderViewport(renderer, Point2(1.0 - rightMargin - legendSize * 0.2, 1.0 - topMargin - legendSize), Vector2(legendSize * 0.2, legendSize));

	if(!_colorScaleTopLabel || !_colorScaleTopLabel->isValid(renderer))
		_colorScaleTopLabel = renderer->createTextGeometryBuffer();
	if(!_colorScaleBottomLabel || !_colorScaleBottomLabel->isValid(renderer))
		_colorScaleBottomLabel = renderer->createTextGeometryBuffer();
	if(!_colorScaleTitleLabel || !_colorScaleTitleLabel->isValid(renderer))
		_colorScaleTitleLabel = renderer->createTextGeometryBuffer();

	ColorA labelColor = renderer->isInteractive() ? ColorA(1,1,1) : ColorA(0,0,0);

	_colorScaleTopLabel->setText(topLabel);
	_colorScaleTopLabel->setColor(labelColor);
	_colorScaleBottomLabel->setText(bottomLabel);
	_colorScaleBottomLabel->setColor(labelColor);
	_colorScaleTitleLabel->setText(titleLabel);
	_colorScaleTitleLabel->setColor(labelColor);

	_colorScaleTitleLabel->renderViewport(renderer, Point2(1.0 - rightMargin, 1.0 - topMargin + 0.01), Qt::AlignRight | Qt::AlignBottom);
	_colorScaleTopLabel->renderViewport(renderer, Point2(1.0 - rightMargin - legendSize * 0.24, 1.0 - topMargin), Qt::AlignRight | Qt::AlignTop);
	_colorScaleBottomLabel->renderViewport(renderer, Point2(1.0 - rightMargin - legendSize * 0.24, 1.0 - topMargin - legendSize), Qt::AlignRight | Qt::AlignBottom);
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ColorCodingModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Color coding"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(2);

	propertyListBox = new ParticlePropertyComboBox();
	layout1->addWidget(new QLabel(tr("Property:"), rollout));
	layout1->addWidget(propertyListBox);
	connect(propertyListBox, SIGNAL(activated(int)), this, SLOT(onPropertySelected(int)));

	colorGradientList = new QComboBox(rollout);
	layout1->addWidget(new QLabel(tr("Color gradient:"), rollout));
	layout1->addWidget(colorGradientList);
	connect(colorGradientList, SIGNAL(activated(int)), this, SLOT(onColorGradientSelected(int)));
	for(OvitoObjectType* clazz : PluginManager::instance().listClasses(ColorCodingGradient::OOType)) {
		colorGradientList->addItem(clazz->displayName(), qVariantFromValue((void*)clazz));
	}

	// Update property list if another modifier has been loaded into the editor.
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(updatePropertyList()));
	// Do the same for the color legend.
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(updateColorGradient()));

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
	exportBtn->setIcon(QIcon(":/viz/icons/export_color_scale.png"));
	exportBtn->setToolTip("Export color map to file");
	exportBtn->setAutoRaise(true);
	exportBtn->setIconSize(QSize(42,22));
	connect(exportBtn, SIGNAL(clicked(bool)), this, SLOT(onExportColorScale()));
	layout2->addWidget(exportBtn, 1, 0, Qt::AlignHCenter | Qt::AlignVCenter);

	layout1->addSpacing(8);
	QPushButton* adjustBtn = new QPushButton(tr("Adjust range"), rollout);
	connect(adjustBtn, SIGNAL(clicked(bool)), this, SLOT(onAdjustRange()));
	layout1->addWidget(adjustBtn);
	layout1->addSpacing(4);
	QPushButton* reverseBtn = new QPushButton(tr("Reverse range"), rollout);
	connect(reverseBtn, SIGNAL(clicked(bool)), this, SLOT(onReverseRange()));
	layout1->addWidget(reverseBtn);

	layout1->addSpacing(8);

	// Render legend.
	BooleanParameterUI* renderLegendPUI = new BooleanParameterUI(this, PROPERTY_FIELD(ColorCodingModifier::_renderLegend));
	layout1->addWidget(renderLegendPUI->checkBox());

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());
}

/******************************************************************************
* Updates the contents of the combo box.
******************************************************************************/
void ColorCodingModifierEditor::updatePropertyList()
{
	propertyListBox->clear();

	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	if(!mod) {
		propertyListBox->setEnabled(false);
		return;
	}
	propertyListBox->setEnabled(true);

	// Obtain the particle property that serves as the input for the color coding modifier.
	PipelineFlowState inputState = mod->getModifierInput();

	// Populate property list from input object.
	int initialIndex = -1;
	for(const auto& o : inputState.objects()) {
		ParticlePropertyObject* property = dynamic_object_cast<ParticlePropertyObject>(o.get());
		if(!property) continue;

		// Properties with a non-numeric data type cannot be used as source for the color coding.
		if(property->dataType() != qMetaTypeId<int>() && property->dataType() != qMetaTypeId<FloatType>()) continue;

		if(property->componentNames().empty()) {
			// Scalar property:
			propertyListBox->addItem(property);
		}
		else {
			// Vector property:
			for(int vectorComponent = 0; vectorComponent < (int)property->componentCount(); vectorComponent++) {
				propertyListBox->addItem(property, vectorComponent);
			}
		}
	}

	// Select the right item in the list box.
	int selIndex = propertyListBox->propertyIndex(mod->sourceProperty());
	if(selIndex < 0 && !mod->sourceProperty().isNull()) {
		// Add a place-holder item if the selected property does not exist anymore.
		propertyListBox->addItem(mod->sourceProperty(), tr("%1 (no longer available)").arg(mod->sourceProperty().name()));
		selIndex = propertyListBox->count() - 1;
	}
	propertyListBox->setCurrentIndex(selIndex);
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
	if(mod->colorGradient())
		colorGradientList->setCurrentIndex(colorGradientList->findData(qVariantFromValue((void*)&mod->colorGradient()->getOOType())));
	else
		colorGradientList->setCurrentIndex(-1);
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool ColorCodingModifierEditor::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject() && event->type() == ReferenceEvent::TargetChanged) {
		ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
		propertyListBox->setCurrentProperty(mod->sourceProperty());
	}
	else if(source == editObject() && event->type() == ReferenceEvent::ReferenceChanged &&
			static_cast<ReferenceFieldEvent*>(event)->field() == PROPERTY_FIELD(ColorCodingModifier::_colorGradient)) {
		updateColorGradient();
	}
	return ParticleModifierEditor::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the user selects an input particle property.
******************************************************************************/
void ColorCodingModifierEditor::onPropertySelected(int index)
{
	OVITO_ASSERT(!UndoManager::instance().isRecording());

	if(index < 0) return;
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	OVITO_CHECK_OBJECT_POINTER(mod);

	UndoableTransaction::handleExceptions(tr("Select property"), [this, mod, index]() {
		mod->setSourceProperty(propertyListBox->property(index));
	});
}

/******************************************************************************
* Is called when the user selects a color gradient in the list box.
******************************************************************************/
void ColorCodingModifierEditor::onColorGradientSelected(int index)
{
	OVITO_ASSERT(!UndoManager::instance().isRecording());
	if(index < 0) return;
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	OVITO_CHECK_OBJECT_POINTER(mod);

	const OvitoObjectType* descriptor = static_cast<const OvitoObjectType*>(colorGradientList->itemData(index).value<void*>());
	if(!descriptor) return;

	UndoableTransaction::handleExceptions(tr("Change color gradient"), [descriptor, mod]() {
		// Create an instance of the selected color gradient class.
		OORef<ColorCodingGradient> gradient = static_object_cast<ColorCodingGradient>(descriptor->createInstance());
		if(gradient)
	        mod->setColorGradient(gradient);
	});
}

/******************************************************************************
* Is called when the user presses the "Adjust Range" button.
******************************************************************************/
void ColorCodingModifierEditor::onAdjustRange()
{
	OVITO_ASSERT(!UndoManager::instance().isRecording());
	ColorCodingModifier* mod = static_object_cast<ColorCodingModifier>(editObject());
	OVITO_CHECK_OBJECT_POINTER(mod);

	UndoableTransaction::handleExceptions(tr("Adjust range"), [mod]() {
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
		UndoableTransaction::handleExceptions(tr("Reverse range"), [mod]() {

			// Swap controllers for start and end value.
			OORef<FloatController> oldStartValue = mod->startValueController();
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

};	// End of namespace
