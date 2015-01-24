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

#ifndef __OVITO_RENDER_SETTINGS_H
#define __OVITO_RENDER_SETTINGS_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>
#include "FrameBuffer.h"
#include "SceneRenderer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/**
 * Stores general settings for rendering pictures and movies.
 */
class OVITO_CORE_EXPORT RenderSettings : public RefTarget
{
public:

	/// This enumeration specifies the animation range that should be rendered.
	enum RenderingRangeType {
		CURRENT_FRAME,		///< Render only the current animation.
		ANIMATION_INTERVAL,	///< Render the complete animation interval.
		CUSTOM_INTERVAL,	///< Render time interval defined by the user.
	};
	Q_ENUMS(RenderingRangeType);
	
public:

	/// Constructor.
	/// Creates an instance of the default renderer class which can be accessed via the renderer() method.
	Q_INVOKABLE RenderSettings(DataSet* dataset);
	
	/// Returns the active renderer.
	SceneRenderer* renderer() const { return _renderer; }
	/// Sets the active renderer.
	void setRenderer(SceneRenderer* renderer) { OVITO_ASSERT(renderer == nullptr || renderer->dataset() == this->dataset()); _renderer = renderer; }
	
	/// Returns whether only the current frame or the whole animation will be rendered.
	RenderingRangeType renderingRangeType() const { return _renderingRangeType; }
	/// Specifies whether only the current frame or the whole animation should be rendered.
	void setRenderingRangeType(RenderingRangeType mode) { _renderingRangeType = mode; }
	
	/// Returns the width of the image to be rendered in pixels.
	int outputImageWidth() const { return std::max((int)_outputImageWidth, 1); }
	/// Sets the width of the image to be rendered in pixels.
	void setOutputImageWidth(int width) { _outputImageWidth = width; }
	
	/// Returns the height of the image to be rendered in pixels.
	int outputImageHeight() const { return std::max((int)_outputImageHeight, 1); }
	/// Sets the height of the image to be rendered in pixels.
	void setOutputImageHeight(int height) { _outputImageHeight = height; }

	/// Returns the aspect ratio (height/width) of the rendered image.
	FloatType outputImageAspectRatio() const { return (FloatType)outputImageHeight() / (FloatType)outputImageWidth(); }

	/// Returns the output filename of the rendered image.
	const QString& imageFilename() const { return _imageInfo.filename(); }
	/// Sets the output filename of the rendered image.
	void setImageFilename(const QString& filename);

	/// Returns the output image info of the rendered image.
	const ImageInfo& imageInfo() const { return _imageInfo; }
	/// Sets the output image info for the rendered image.
	void setImageInfo(const ImageInfo& imageInfo);

	/// Returns the background color of the rendered image.
	Color backgroundColor() const { return _backgroundColor ? _backgroundColor->currentColorValue() : Color(0,0,0); }
	/// Sets the background color of the rendered image.
	void setBackgroundColor(const Color& color) { if(_backgroundColor) _backgroundColor->setCurrentColorValue(color); }
	/// Returns the controller for the background color of the rendered image.
	Controller* backgroundColorController() const { return _backgroundColor; }
	/// Sets the controller for the background color of the rendered image.
	void setBackgroundColorController(Controller* colorController) { _backgroundColor = colorController; }
	
	/// Returns whether the alpha channel will be generated.
	bool generateAlphaChannel() const { return _generateAlphaChannel; }
	/// Sets whether the alpha channel will be generated.
	void setGenerateAlphaChannel(bool enable) { _generateAlphaChannel = enable; }

	/// Returns whether the rendered image is saved to an output file.
	bool saveToFile() const { return _saveToFile; }
	/// Sets whether the rendered image is saved to an output file.
	void setSaveToFile(bool enable) { _saveToFile = enable; }

	/// Returns whether existing animation frames are skipped during render.
	bool skipExistingImages() const { return _skipExistingImages; }
	/// Sets whether existing animation frames are skipped during render.
	void setSkipExistingImages(bool enable) { _skipExistingImages = enable; }

	/// Returns the first frame to render when the rendering range is set to CUSTOM_INTERVAL.
	int customRangeStart() const { return _customRangeStart; }
	/// Sets the first frame to render when the rendering range is set to CUSTOM_INTERVAL.
	void setCustomRangeStart(int frame) { _customRangeStart = frame; }

	/// Returns the last frame to render when the rendering range is set to CUSTOM_INTERVAL.
	int customRangeEnd() const { return _customRangeEnd; }
	/// Sets the last frame to render when the rendering range is set to CUSTOM_INTERVAL.
	void setCustomRangeEnd(int frame) { _customRangeEnd = frame; }

	/// Returns the frame interval to render when rendering an animation.
	int everyNthFrame() const { return _everyNthFrame; }
	/// Sets the frame interval to render when rendering an animation.
	void setEveryNthFrame(int n) { _everyNthFrame = n; }

	/// Returns the base number for filename generation when rendering an animation.
	int fileNumberBase() const { return _fileNumberBase; }
	/// Sets the base number for filename generation when rendering an animation.
	void setFileNumberBase(int n) { _fileNumberBase = n; }

public:

	Q_PROPERTY(QString imageFilename READ imageFilename WRITE setImageFilename);

protected:

	/// Saves the class' contents to the given stream. 
	virtual void saveToStream(ObjectSaveStream& stream) override;
	/// Loads the class' contents from the given stream. 
	virtual void loadFromStream(ObjectLoadStream& stream) override;
	/// Creates a copy of this object. 
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;
	
private:

	/// Contains the output filename and format of the image to be rendered.
	ImageInfo _imageInfo;

	/// The instance of the plugin renderer class. 
	ReferenceField<SceneRenderer> _renderer;

	/// Controls the background color of the rendered image.
	ReferenceField<Controller> _backgroundColor;
	
	/// The width of the output image in pixels.
	PropertyField<int> _outputImageWidth;

	/// The height of the output image in pixels.
	PropertyField<int> _outputImageHeight;

	/// Controls whether the alpha channel will be included in the output image.
	PropertyField<bool> _generateAlphaChannel;

	/// Controls whether the rendered image is saved to the output file.
	PropertyField<bool> _saveToFile;

	/// Controls whether already rendered frames are skipped.
	PropertyField<bool> _skipExistingImages;

	/// Specifies which part of the animation should be rendered.
    PropertyField<RenderingRangeType, int> _renderingRangeType;

	/// The first frame to render when rendering range is set to CUSTOM_INTERVAL.
	PropertyField<int> _customRangeStart;

	/// The last frame to render when rendering range is set to CUSTOM_INTERVAL.
	PropertyField<int> _customRangeEnd;

	/// Specifies the number of frames to skip when rendering an animation.
	PropertyField<int> _everyNthFrame;

	/// Specifies the base number for filename generation when rendering an animation.
	PropertyField<int> _fileNumberBase;

private:
    
    friend class RenderSettingsEditor;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_renderer);
	DECLARE_REFERENCE_FIELD(_backgroundColor);
	DECLARE_PROPERTY_FIELD(_outputImageWidth);
	DECLARE_PROPERTY_FIELD(_outputImageHeight);
	DECLARE_PROPERTY_FIELD(_generateAlphaChannel);
	DECLARE_PROPERTY_FIELD(_saveToFile);
	DECLARE_PROPERTY_FIELD(_skipExistingImages);
	DECLARE_PROPERTY_FIELD(_renderingRangeType);
	DECLARE_PROPERTY_FIELD(_customRangeStart);
	DECLARE_PROPERTY_FIELD(_customRangeEnd);
	DECLARE_PROPERTY_FIELD(_everyNthFrame);
	DECLARE_PROPERTY_FIELD(_fileNumberBase);
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::RenderSettings::RenderingRangeType);
Q_DECLARE_TYPEINFO(Ovito::RenderSettings::RenderingRangeType, Q_PRIMITIVE_TYPE);

#endif // __OVITO_RENDER_SETTINGS_H
