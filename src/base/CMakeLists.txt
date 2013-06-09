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
)

# Build library
ADD_LIBRARY(Base ${OVITO_BASE_SOURCE_FILES})

# Link to OpenGL library.
TARGET_LINK_LIBRARIES(Base ${OPENGL_gl_LIBRARY})

# Link Qt5.
QT5_USE_MODULES(Base Widgets Xml)
