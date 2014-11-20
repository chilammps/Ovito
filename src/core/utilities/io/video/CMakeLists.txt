###############################################################################
# 
#  Copyright (2013) Alexander Stukowski
#
#  This file is part of OVITO (Open Visualization Tool).
#
#  OVITO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  OVITO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

# Find FFmpeg library.
FIND_PACKAGE(FFmpeg REQUIRED)
IF(NOT FFMPEG_FOUND)
	MESSAGE(FATAL_ERROR "FFmpeg library not found.")
ENDIF()

# Build library.
ADD_LIBRARY(Video VideoEncoder.cpp)
TARGET_LINK_LIBRARIES(Video Base)

# This will let the source code export symbols when building a DLL.
TARGET_COMPILE_DEFINITIONS(Video PRIVATE OVITO_VIDEO_LIBRARY)

# Link to FFmpeg library.
TARGET_LINK_LIBRARIES(Video ${FFMPEG_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(Video PRIVATE ${FFMPEG_INCLUDE_DIR})

# Link to ZLib library.
TARGET_LINK_LIBRARIES(Video ${ZLIB_LIBRARIES})

# Link Qt5.
QT5_USE_MODULES(Video ${OVITO_REQUIRED_QT_MODULES})

# This library is part of the installation package.
IF(NOT OVITO_MONOLITHIC_BUILD)
	INSTALL(TARGETS Video 
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()

OVITO_FIXUP_BUILD_OBJECT(Video)
