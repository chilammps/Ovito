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

#ifndef __OVITO_COLOR_CODING_MODIFIER_H
#define __OVITO_COLOR_CODING_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>
#include <core/rendering/ImagePrimitive.h>
#include <core/rendering/TextPrimitive.h>
#include <core/viewport/Viewport.h>
#include "../ParticleModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Coloring)

/**
 * \brief Abstract base class for color gradients that can be used with a ColorCodingModifier.
 *
 * Implementations of this class convert a scalar value in the range [0,1] to a color value.
 */
class OVITO_PARTICLES_EXPORT ColorCodingGradient : public RefTarget
{
protected:

	/// Constructor.
	ColorCodingGradient(DataSet* dataset) : RefTarget(dataset) {}

public:

	/// \brief Converts a scalar value to a color value.
	/// \param t A value between 0 and 1.
	/// \return The color that visualizes the given scalar value.
	virtual Color valueToColor(FloatType t) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * \brief Converts a scalar value to a color using the HSV color system.
 */
class ColorCodingHSVGradient : public ColorCodingGradient
{
public:

	/// Constructor.
	Q_INVOKABLE ColorCodingHSVGradient(DataSet* dataset) : ColorCodingGradient(dataset) {}

	/// \brief Converts a scalar value to a color value.
	/// \param t A value between 0 and 1.
	/// \return The color that visualizes the given scalar value.
	virtual Color valueToColor(FloatType t) override { return Color::fromHSV((FloatType(1) - t) * FloatType(0.7), 1, 1); }

private:

	Q_OBJECT
	OVITO_OBJECT
	Q_CLASSINFO("DisplayName", "Rainbow");
};

/**
 * \brief Converts a scalar value to a color using a gray-scale ramp.
 */
class ColorCodingGrayscaleGradient : public ColorCodingGradient
{
public:

	/// Constructor.
	Q_INVOKABLE ColorCodingGrayscaleGradient(DataSet* dataset) : ColorCodingGradient(dataset) {}

	/// \brief Converts a scalar value to a color value.
	/// \param t A value between 0 and 1.
	/// \return The color that visualizes the given scalar value.
	virtual Color valueToColor(FloatType t) override { return Color(t, t, t); }

private:

	Q_OBJECT
	OVITO_OBJECT
	Q_CLASSINFO("DisplayName", "Grayscale");
};

/**
 * \brief Converts a scalar value to a color.
 */
class ColorCodingHotGradient : public ColorCodingGradient
{
public:

	/// Constructor.
	Q_INVOKABLE ColorCodingHotGradient(DataSet* dataset) : ColorCodingGradient(dataset) {}

	/// \brief Converts a scalar value to a color value.
	/// \param t A value between 0 and 1.
	/// \return The color that visualizes the given scalar value.
	virtual Color valueToColor(FloatType t) override {
		// Interpolation black->red->yellow->white.
		OVITO_ASSERT(t >= 0.0f && t <= 1.0f);
		return Color(std::min(t / 0.375f, FloatType(1)), std::max(FloatType(0), std::min((t-0.375f)/0.375f, FloatType(1))), std::max(FloatType(0), t*4.0f - 3.0f));
	}

private:

	Q_OBJECT
	OVITO_OBJECT
	Q_CLASSINFO("DisplayName", "Hot");
};

/**
 * \brief Converts a scalar value to a color.
 */
class ColorCodingJetGradient : public ColorCodingGradient
{
public:

	/// Constructor.
	Q_INVOKABLE ColorCodingJetGradient(DataSet* dataset) : ColorCodingGradient(dataset) {}

	/// \brief Converts a scalar value to a color value.
	/// \param t A value between 0 and 1.
	/// \return The color that visualizes the given scalar value.
	virtual Color valueToColor(FloatType t) override {
	    if(t < 0.125f) return Color(0, 0, 0.5f + 0.5f * t / 0.125f);
	    else if(t < 0.125f + 0.25f) return Color(0, (t - 0.125f) / 0.25f, 1);
	    else if(t < 0.125f + 0.25f + 0.25f) return Color((t - 0.375f) / 0.25f, 1, 1.0f - (t - 0.375f) / 0.25f);
	    else if(t < 0.125f + 0.25f + 0.25f + 0.25f) return Color(1, 1.0f - (t - 0.625f) / 0.25f, 0);
	    else return Color(1.0f - 0.5f * (t - 0.875f) / 0.125f, 0, 0);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
	Q_CLASSINFO("DisplayName", "Jet");
};

/**
 * \brief Converts a scalar value to a color based on a user-defined image.
 */
class ColorCodingImageGradient : public ColorCodingGradient
{
public:

	/// Constructor.
	Q_INVOKABLE ColorCodingImageGradient(DataSet* dataset) : ColorCodingGradient(dataset) {
		INIT_PROPERTY_FIELD(ColorCodingImageGradient::_image);
	}

	/// \brief Converts a scalar value to a color value.
	/// \param t A value between 0 and 1.
	/// \return The color that visualizes the given scalar value.
	virtual Color valueToColor(FloatType t) override;

	/// Changes the image to be used for mapping values to colors.
	void setImage(const QImage& image) { _image = image; }

	/// Loads the given image file from disk.
	void loadImage(const QString& filename);

	/// Returns the image being used for mapping values to colors.
	const QImage& image() const { return _image; }

private:

	/// The user-defined color map image.
	PropertyField<QImage> _image;

	Q_OBJECT
	OVITO_OBJECT
	Q_CLASSINFO("DisplayName", "User image");

	DECLARE_PROPERTY_FIELD(_image);
};


/**
 * \brief This modifier assigns a colors to the particles based on the value of a particle property.
 */
class ColorCodingModifier : public ParticleModifier
{
public:

	/// Constructor.
	Q_INVOKABLE ColorCodingModifier(DataSet* dataset);

	/// Loads the user-defined default values of this object's parameter fields from the
	/// application's settings store.
	virtual void loadUserDefaults() override;

	/// Asks the modifier for its validity interval at the given time.
	virtual TimeInterval modifierValidity(TimePoint time) override;

	/// Sets the source particle property that is used for coloring of particles.
	void setSourceProperty(const ParticlePropertyReference& prop) { _sourceProperty = prop; }

	/// Returns the source particle property that is used for coloring of particles.
	const ParticlePropertyReference& sourceProperty() const { return _sourceProperty; }

	/// Returns the range start value.
	FloatType startValue() const { return _startValueCtrl ? _startValueCtrl->currentFloatValue() : 0; }

	/// Sets the range start value.
	void setStartValue(FloatType value) { if(_startValueCtrl) _startValueCtrl->setCurrentFloatValue(value); }

	/// Returns the controller for the range start value.
	Controller* startValueController() const { return _startValueCtrl; }

	/// Sets the controller for the range start value.
	void setStartValueController(Controller* ctrl) { _startValueCtrl = ctrl; }

	/// Returns the range end value.
	FloatType endValue() const { return _endValueCtrl ? _endValueCtrl->currentFloatValue() : 0; }

	/// Sets the range end value.
	void setEndValue(FloatType value) { if(_endValueCtrl) _endValueCtrl->setCurrentFloatValue(value); }

	/// Returns the controller for the range end value.
	Controller* endValueController() const { return _endValueCtrl; }

	/// Sets the controller for the range end value.
	void setEndValueController(Controller* ctrl) { _endValueCtrl = ctrl; }

	/// Returns the color gradient used by the modifier to convert scalar atom properties to colors.
	ColorCodingGradient* colorGradient() const { return _colorGradient; }

	/// Sets the color gradient for the modifier to convert scalar atom properties to colors.
	void setColorGradient(ColorCodingGradient* gradient) { _colorGradient = gradient; }

	/// Returns whether the modifier assigns a color only to selected particles.
	bool colorOnlySelected() const { return _colorOnlySelected; }

	/// Sets whether the modifier should assign a color only to selected particles.
	void setColorOnlySelected(bool colorOnlySelected) { _colorOnlySelected = colorOnlySelected; }

	/// Returns whether the input particle selection is preserved by the modifier.
	bool keepSelection() const { return _keepSelection; }

	/// Sets whether the input particle selection should be preserved by the modifier.
	void setKeepSelection(bool keepSel) { _keepSelection = keepSel; }

public Q_SLOTS:

	/// Sets the start and end value to the minimum and maximum value in the selected data channel.
	bool adjustRange();

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// This virtual method is called by the system when the modifier has been inserted into a PipelineObject.
	virtual void initializeModifier(PipelineObject* pipelineObject, ModifierApplication* modApp) override;

	/// Modifies the particles.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This controller stores the start value of the color scale.
	ReferenceField<Controller> _startValueCtrl;

	/// This controller stores the end value of the color scale.
	ReferenceField<Controller> _endValueCtrl;

	/// This object converts scalar atom properties to colors.
	ReferenceField<ColorCodingGradient> _colorGradient;

	/// The particle type property that is used as source for the coloring.
	PropertyField<ParticlePropertyReference> _sourceProperty;

	/// Controls whether the modifier assigns a color only to selected particles.
	PropertyField<bool> _colorOnlySelected;

	/// Controls whether the input particle selection is preserved.
	/// If false, the selection is cleared by the modifier.
	PropertyField<bool> _keepSelection;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("DisplayName", "Color coding");
	Q_CLASSINFO("ModifierCategory", "Coloring");

	DECLARE_REFERENCE_FIELD(_startValueCtrl);
	DECLARE_REFERENCE_FIELD(_endValueCtrl);
	DECLARE_REFERENCE_FIELD(_colorGradient);
	DECLARE_PROPERTY_FIELD(_colorOnlySelected);
	DECLARE_PROPERTY_FIELD(_keepSelection);
	DECLARE_PROPERTY_FIELD(_sourceProperty);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * A properties editor for the ColorCodingModifier class.
 */
class ColorCodingModifierEditor : public ParticleModifierEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE ColorCodingModifierEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	/// Returns an icon representing the given color map class.
	QIcon iconFromColorMapClass(const OvitoObjectType* clazz);

	/// Returns an icon representing the given color map.
	QIcon iconFromColorMap(ColorCodingGradient* map);

	/// The list of available color gradients.
	QComboBox* colorGradientList;

	/// Indicates the combo box already contains an item for a custom color map.
	bool _gradientListContainCustomItem;

	/// Label that displays the color gradient picture.
	QLabel* colorLegendLabel;

protected Q_SLOTS:

	/// Updates the display for the color gradient.
	void updateColorGradient();

	/// Is called when the user selects a color gradient in the list box.
	void onColorGradientSelected(int index);

	/// Is called when the user presses the "Adjust Range" button.
	void onAdjustRange();

	/// Is called when the user presses the "Reverse Range" button.
	void onReverseRange();

	/// Is called when the user presses the "Export color scale" button.
	void onExportColorScale();

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_COLOR_CODING_MODIFIER_H
