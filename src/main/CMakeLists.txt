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

# Include the resource in the Win32 executable.
IF(WIN32)
	SET(WINDOWS_RESOURCES resources/ovito.rc)
ENDIF(WIN32)

# Put the executable into the right directory.
SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})

# Builds the main executable of the application
ADD_EXECUTABLE(${PROJECT_NAME} WIN32 MACOSX_BUNDLE Main.cpp ${WINDOWS_RESOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Base Core Viz)

# Set name of executable.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ovito")

# Link Qt5.
QT5_USE_MODULES(${PROJECT_NAME} Widgets Xml)

# This executable will be part of the installation package.
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

# This macro adds a static library target to an executable target and makes sure that
# the library is linked completely into the executable without removal of unreferenced
# objects.
MACRO(LINK_WHOLE_LIBRARY _targetName _libraryTarget)
	GET_TARGET_PROPERTY(EXE_LINKER_OPTIONS ${_targetName} LINK_FLAGS)
	IF(NOT EXE_LINKER_OPTIONS)
		SET(EXE_LINKER_OPTIONS "")
	ENDIF(NOT EXE_LINKER_OPTIONS)	
	GET_TARGET_PROPERTY(TARGET_LOCATION ${_libraryTarget} LOCATION)
	SET(EXE_LINKER_OPTIONS "${EXE_LINKER_OPTIONS} -Wl,-whole-archive ${TARGET_LOCATION} -Wl,-no-whole-archive ")
	SET_TARGET_PROPERTIES(${_targetName} PROPERTIES LINK_FLAGS ${EXE_LINKER_OPTIONS})
	TARGET_LINK_LIBRARIES(${_targetName} ${_libraryTarget})
ENDMACRO(LINK_WHOLE_LIBRARY)

LINK_WHOLE_LIBRARY(${PROJECT_NAME} Core)
LINK_WHOLE_LIBRARY(${PROJECT_NAME} Viz)

IF(APPLE)
	# Install the Info.plist file.
	CONFIGURE_FILE("resources/Info.plist" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")

	# Copy the application icon into the resource directory.
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/ovito.icns" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}")
ENDIF()
