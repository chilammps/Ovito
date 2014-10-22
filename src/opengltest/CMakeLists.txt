###############################################################################
# 
#  Copyright (2014) Alexander Stukowski
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

# Put the executable into the right directory.
SET(EXECUTABLE_OUTPUT_PATH "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS")

# Builds the OpenGL test executable.
ADD_EXECUTABLE(OpenGLTest Main.cpp TestWindow.cpp)

# The executable makes use of OVITO's core functions.
TARGET_LINK_LIBRARIES(OpenGLTest Base Core)

# Link Qt5.
QT5_USE_MODULES(OpenGLTest ${OVITO_REQUIRED_QT_MODULES})

# This executable will be part of the installation package.
INSTALL(TARGETS OpenGLTest DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

OVITO_FIXUP_BUILD_OBJECT(OpenGLTest)
