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

SET(OVITO_BASE_SOURCE_FILES 
	utilities/Exception.cpp
	linalg/AffineDecomposition.cpp
	io/SaveStream.cpp
	io/LoadStream.cpp
	io/gzdevice/qtiocompressor.cpp
)

# This will let the source code export symbols when building a DLL.
ADD_DEFINITIONS(-DOVITO_BASE_LIBRARY)

# Build library
ADD_LIBRARY(Base ${OVITO_BASE_SOURCE_FILES})

# Link to OpenGL library.
TARGET_LINK_LIBRARIES(Base ${OPENGL_gl_LIBRARY})

# Link to ZLib library.
TARGET_LINK_LIBRARIES(Base ${ZLIB_LIBRARIES})

# Link Qt5.
QT5_USE_MODULES(Base ${OVITO_REQUIRED_QT_MODULES})

# This library is part of the installation package.
IF(APPLE OR WIN32)
	INSTALL(TARGETS Base 
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()
