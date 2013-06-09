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
#include <core/rendering/FrameBuffer.h>

namespace Ovito {

#define IMAGE_FORMAT_FILE_FORMAT_VERSION		1

/******************************************************************************
* Writes an ImageInfo to an output stream.
******************************************************************************/
SaveStream& operator<<(SaveStream& stream, const ImageInfo& i)
{
	stream.beginChunk(IMAGE_FORMAT_FILE_FORMAT_VERSION);
	stream << i._imageWidth;
	stream << i._imageHeight;
	stream << i._filename;
	stream << i._format;
	stream.endChunk();
	return stream;
}

/******************************************************************************
* Reads an ImageInfo from an input stream.
******************************************************************************/
LoadStream& operator>>(LoadStream& stream, ImageInfo& i)
{
	int fileVersion = stream.expectChunkRange(0, IMAGE_FORMAT_FILE_FORMAT_VERSION);
	stream >> i._imageWidth;
	stream >> i._imageHeight;
	stream >> i._filename;
	if(fileVersion >= 1) {
		stream >> i._format;
	}
	else i._format = QFileInfo(i._filename).suffix().toUpper().toLocal8Bit();
	stream.closeChunk();
	return stream;
}

};
