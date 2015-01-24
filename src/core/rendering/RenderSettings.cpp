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
#include <core/rendering/SceneRenderer.h>
#include <core/rendering/standard/StandardSceneRenderer.h>
#include <core/viewport/Viewport.h>
#include <core/gui/app/Application.h>
#include <core/plugins/PluginManager.h>
#include "RenderSettings.h"
#include "RenderSettingsEditor.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, RenderSettings, RefTarget);
SET_OVITO_OBJECT_EDITOR(RenderSettings, RenderSettingsEditor);
DEFINE_FLAGS_REFERENCE_FIELD(RenderSettings, _renderer, "Renderer", SceneRenderer, PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_REFERENCE_FIELD(RenderSettings, _backgroundColor, "BackgroundColor", Controller, PROPERTY_FIELD_MEMORIZE);
DEFINE_PROPERTY_FIELD(RenderSettings, _outputImageWidth, "OutputImageWidth");
DEFINE_PROPERTY_FIELD(RenderSettings, _outputImageHeight, "OutputImageHeight");
DEFINE_PROPERTY_FIELD(RenderSettings, _generateAlphaChannel, "GenerateAlphaChannel");
DEFINE_PROPERTY_FIELD(RenderSettings, _saveToFile, "SaveToFile");
DEFINE_PROPERTY_FIELD(RenderSettings, _skipExistingImages, "SkipExistingImages");
DEFINE_PROPERTY_FIELD(RenderSettings, _renderingRangeType, "RenderingRangeType");
DEFINE_PROPERTY_FIELD(RenderSettings, _customRangeStart, "CustomRangeStart");
DEFINE_PROPERTY_FIELD(RenderSettings, _customRangeEnd, "CustomRangeEnd");
DEFINE_PROPERTY_FIELD(RenderSettings, _everyNthFrame, "EveryNthFrame");
DEFINE_PROPERTY_FIELD(RenderSettings, _fileNumberBase, "FileNumberBase");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _renderer, "Renderer");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _backgroundColor, "Background color");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _outputImageWidth, "Width");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _outputImageHeight, "Height");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _generateAlphaChannel, "Transparent background");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _saveToFile, "Save to file");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _skipExistingImages, "Skip existing animation images");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _renderingRangeType, "Rendering range");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _customRangeStart, "Range start");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _customRangeEnd, "Range end");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _everyNthFrame, "Every Nth frame");
SET_PROPERTY_FIELD_LABEL(RenderSettings, _fileNumberBase, "File number base");

/******************************************************************************
* Constructor.
* Creates an instance of the default renderer class which can be 
* accessed via the renderer() method.
******************************************************************************/
RenderSettings::RenderSettings(DataSet* dataset) : RefTarget(dataset),
	_outputImageWidth(640), _outputImageHeight(480), _generateAlphaChannel(false),
	_saveToFile(Application::instance().consoleMode()), _skipExistingImages(false), _renderingRangeType(CURRENT_FRAME),
	_customRangeStart(0), _customRangeEnd(100), _everyNthFrame(1), _fileNumberBase(0)
{
	INIT_PROPERTY_FIELD(RenderSettings::_renderer);
	INIT_PROPERTY_FIELD(RenderSettings::_backgroundColor);
	INIT_PROPERTY_FIELD(RenderSettings::_outputImageWidth);
	INIT_PROPERTY_FIELD(RenderSettings::_outputImageHeight);
	INIT_PROPERTY_FIELD(RenderSettings::_generateAlphaChannel);
	INIT_PROPERTY_FIELD(RenderSettings::_saveToFile);
	INIT_PROPERTY_FIELD(RenderSettings::_skipExistingImages);
	INIT_PROPERTY_FIELD(RenderSettings::_renderingRangeType);
	INIT_PROPERTY_FIELD(RenderSettings::_customRangeStart);
	INIT_PROPERTY_FIELD(RenderSettings::_customRangeEnd);
	INIT_PROPERTY_FIELD(RenderSettings::_everyNthFrame);
	INIT_PROPERTY_FIELD(RenderSettings::_fileNumberBase);

	// Setup default background color.
	_backgroundColor = ControllerManager::instance().createColorController(dataset);
	setBackgroundColor(Color(1,1,1));

	// Create an instance of the default renderer class.
	OORef<SceneRenderer> renderer(new StandardSceneRenderer(dataset));
	setRenderer(renderer);
}

/******************************************************************************
* Sets the output filename of the rendered image. 
******************************************************************************/
void RenderSettings::setImageFilename(const QString& filename)
{
	if(filename == imageFilename()) return;
	_imageInfo.setFilename(filename);
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Sets the output image info of the rendered image.
******************************************************************************/
void RenderSettings::setImageInfo(const ImageInfo& imageInfo)
{
	if(imageInfo == _imageInfo) return;
	_imageInfo = imageInfo;
	notifyDependents(ReferenceEvent::TargetChanged);
}

#define RENDER_SETTINGS_FILE_FORMAT_VERSION		1

/******************************************************************************
* Saves the class' contents to the given stream. 
******************************************************************************/
void RenderSettings::saveToStream(ObjectSaveStream& stream)
{
	RefTarget::saveToStream(stream);

	stream.beginChunk(RENDER_SETTINGS_FILE_FORMAT_VERSION);
	stream << _imageInfo;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream. 
******************************************************************************/
void RenderSettings::loadFromStream(ObjectLoadStream& stream)
{
	RefTarget::loadFromStream(stream);

	int fileVersion = stream.expectChunkRange(0, RENDER_SETTINGS_FILE_FORMAT_VERSION);
	if(fileVersion == 0) {
		bool generateAlphaChannel;
		RenderingRangeType renderingRange;
		stream >> renderingRange;
		stream >> _imageInfo;
		stream >> generateAlphaChannel;
		_generateAlphaChannel = generateAlphaChannel;
		_renderingRangeType = renderingRange;
		_outputImageWidth = _imageInfo.imageWidth();
		_outputImageHeight = _imageInfo.imageHeight();
	}
	else {
		stream >> _imageInfo;
	}
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object. 
******************************************************************************/
OORef<RefTarget> RenderSettings::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<RenderSettings> clone = static_object_cast<RenderSettings>(RefTarget::clone(deepCopy, cloneHelper));
	
	/// Copy data values.
	clone->_imageInfo = this->_imageInfo;

	return clone;
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
