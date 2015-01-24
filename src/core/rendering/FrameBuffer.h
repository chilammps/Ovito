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

#ifndef __OVITO_FRAME_BUFFER_H
#define __OVITO_FRAME_BUFFER_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering)

/******************************************************************************
* Stores information about the image in a FrameBuffer.
******************************************************************************/
class OVITO_CORE_EXPORT ImageInfo
{
public:

	/// Default constructor.
	ImageInfo() : _imageWidth(0), _imageHeight(0) {}

	/// Comparison operator.
	bool operator==(const ImageInfo& other) const {
		if(this->_imageWidth != other._imageWidth) return false;
		if(this->_imageHeight != other._imageHeight) return false;
		if(this->_filename != other._filename) return false;
		if(this->_format != other._format) return false;
		return true;
	}
	
	/// Returns the width of the image in pixels.
	int imageWidth() const { return _imageWidth; }

	/// Sets the width of the image in pixels.
	void setImageWidth(int width) { OVITO_ASSERT(width >= 0); _imageWidth = width; }
	
	/// Returns the height of the image in pixels.
	int imageHeight() const { return _imageHeight; }

	/// Sets the height of the image to be rendered in pixels.
	void setImageHeight(int height) { OVITO_ASSERT(height >= 0); _imageHeight = height; }

	/// Returns the filename of the image on disk.
	const QString& filename() const { return _filename; }

	/// Sets the filename of the image on disk.
	void setFilename(const QString& filename) {
		_filename = filename;
		guessFormatFromFilename();
	}

	/// Returns the format of the image on disk.
	const QByteArray& format() const { return _format; }

	/// Sets the format of the image on disk.
	void setFormat(const QByteArray& format) { _format = format; }
	
	/// Detects the file format based on the filename suffix.
	bool guessFormatFromFilename();

	/// Returns whether the selected file format is a video format.
	bool isMovie() const;

private:

	/// The width of the image in pixels.
	int _imageWidth;

	/// The height of the image in pixels.
	int _imageHeight;
	
	/// The filename of the image on disk.
	QString _filename;

	/// The format of the image on disk.
	QByteArray _format;
	
	friend SaveStream& operator<<(SaveStream& stream, const ImageInfo& i);
	friend LoadStream& operator>>(LoadStream& stream, ImageInfo& i);
}; 

/// Writes an ImageInfo to an output stream.
/// \relates ImageInfo
SaveStream& operator<<(SaveStream& stream, const ImageInfo& i);

/// Reads an ImageInfo from an input stream.
/// \relates ImageInfo
LoadStream& operator>>(LoadStream& stream, ImageInfo& i);

/******************************************************************************
* A frame buffer is used by a renderer to store the rendered image.
******************************************************************************/
class OVITO_CORE_EXPORT FrameBuffer : public QObject
{
public:

	/// Constructor.
	FrameBuffer(QObject* parent = nullptr) : QObject(parent) {}

	/// Constructor.
	FrameBuffer(int width, int height, QObject* parent = nullptr) : QObject(parent), _image(width, height, QImage::Format_ARGB32) {
		_info.setImageWidth(width);
		_info.setImageHeight(height);
		clear();
	}

	/// Returns the internal QImage that is used to store the pixel data.
	QImage& image() { return _image; }

	/// Returns the internal QImage that is used to store the pixel data.
	const QImage& image() const { return _image; }

	/// Returns the width of the image.
	int width() const { return _image.width(); }

	/// Returns the height of the image.
	int height() const { return _image.height(); }
	
	/// Returns the size of the image.
	QSize size() const { return _image.size(); }

	/// Sets the size of the frame buffer image.
	void setSize(const QSize& newSize) {
		if(newSize == size())
			return;
		_info.setImageWidth(newSize.width());
		_info.setImageHeight(newSize.height());
		_image = _image.copy(0, 0, newSize.width(), newSize.height());
		update();
	}

	/// Returns the descriptor of the image.
	const ImageInfo& info() const { return _info; }
	
	/// Clears the framebuffer.
	void clear() { _image.fill(0); }

	/// This method must be called each time the contents of the frame buffer have been modified.
	/// Fires the contentReset() signal.
	void update() {
		contentReset();
	}

	/// This method must be called each time the contents of the frame buffer have been modified.
	/// Fires the contentChanged() signal.
	void update(const QRect& changedRegion) {
		contentChanged(changedRegion);
	}

Q_SIGNALS:

	/// This signal is emitted by the framebuffer when a part of its content has changed.
	void contentChanged(QRect changedRegion);

	/// This signal is emitted by the framebuffer when its content has been replaced.
	void contentReset();

private:

	/// The internal image that stores the pixel data.
	QImage _image;
	
	/// The descriptor of the image.
	ImageInfo _info;

private:

	Q_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FRAME_BUFFER_H
