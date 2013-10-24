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

######################### Setup CPack #######################

# Gather required system libraries and install them.
SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
INCLUDE(InstallRequiredSystemLibraries)
IF(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS AND WIN32)
	INSTALL_PROGRAMS("${OVITO_RELATIVE_BINARY_DIRECTORY}" ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
ENDIF()

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OVITO (The Open Visualization Tool) is a scientific visualization and analysis software for atomistic simulation data.")
SET(CPACK_PACKAGE_VENDOR "Alexander Stukowski")
SET(CPACK_PACKAGE_CONTACT "Alexander Stukowski <mail@ovito.org>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${OVITO_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${OVITO_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${OVITO_VERSION_REVISION})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OVITO ${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}")

IF(UNIX AND NOT APPLE)
	SET(CPACK_GENERATOR "TGZ")
	SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY "1")
	EXECUTE_PROCESS(COMMAND "uname" "-m" OUTPUT_VARIABLE MACHINE_HARDWARE_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
	SET(CPACK_PACKAGE_FILE_NAME "ovito-${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}.${OVITO_VERSION_REVISION}-${MACHINE_HARDWARE_NAME}")
ELSEIF(APPLE)
	SET(CPACK_GENERATOR "ZIP")
	SET(CPACK_PACKAGE_FILE_NAME "ovito-${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}.${OVITO_VERSION_REVISION}-macosx")
ELSEIF(WIN32)
	SET(CPACK_GENERATOR "ZIP")
	SET(CPACK_PACKAGE_FILE_NAME "ovito-${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}.${OVITO_VERSION_REVISION}-win32")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_debug")
ENDIF()

IF(NOT OVITO_MONOLITHIC_BUILD)
	IF(NOT WIN32)
		SET(CPACK_STRIP_FILES "${OVITO_RELATIVE_BINARY_DIRECTORY}/ovito")
		SET(CPACK_SOURCE_STRIP_FILES "")
	ENDIF()
ELSE()
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" DESTINATION "./")
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" DESTINATION "./")
ENDIF()

SET(CPACK_PACKAGE_EXECUTABLES "${OVITO_RELATIVE_BINARY_DIRECTORY}/ovito" "OVITO (The Open Visualization Tool)")

INCLUDE(CPack)

