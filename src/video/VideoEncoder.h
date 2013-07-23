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

/**
 * \file VideoEncoder.h
 * \brief Contains the definition of the Ovito::VideoEncoder class.
 */

#ifndef __OVITO_VIDEO_ENCODER_H
#define __OVITO_VIDEO_ENCODER_H

#include <base/Base.h>

namespace Ovito {

/**
 * \brief Wrapper class for the FFmpeg video encoding library.
 */
class VideoEncoder : public QObject
{
public:

	/// \brief Constructor.
	VideoEncoder(QObject* parent = nullptr);

private:

	Q_OBJECT
};

};	// End of namespace

#endif // __OVITO_VIDEO_ENCODER_H
