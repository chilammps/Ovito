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

PROJECT(Ovito)

# Make sure we have a recent version of CMake.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8 FATAL_ERROR)

# Include program revision information.
GET_FILENAME_COMPONENT(OVITO_SOURCE_BASE_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Version.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Plugins.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/CompileQtResource.cmake)

# Tell CMake to run Qt moc when necessary:
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
  
# Find and setup Qt5 library.
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5Xml REQUIRED)

# Use the Q_SIGNALS and Q_SLOTS macros to avoid name conflicts.
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

# Find OpenGL library.
FIND_PACKAGE(OpenGL REQUIRED)

# Define the OVITO_DEBUG macro in debug builds.
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DOVITO_DEBUG")
ENDIF()

# Enable C++11.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
IF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-invalid-offsetof")
ENDIF()

# This is the base directory for all included header files.
INCLUDE_DIRECTORIES("${OVITO_SOURCE_BASE_DIR}/src")

IF(UNIX)
	IF(APPLE)
		SET(OVITO_MONOLITHIC_BUILD "OFF")
		SET(MACOSX_BUNDLE_NAME "Ovito") 
		SET(MACOSX_BUNDLE_BUNDLE_NAME "${MACOSX_BUNDLE_NAME}") 
		
		# The directory where the main executable goes to.
		SET(OVITO_RELATIVE_BINARY_DIRECTORY ".")
		# The directory where the main libraries go to.
		SET(OVITO_RELATIVE_LIBRARY_DIRECTORY "${MACOSX_BUNDLE_NAME}.app/Contents/MacOS")
		# The directory where the auxiliary files go to.
		SET(OVITO_RELATIVE_SHARE_DIRECTORY "${MACOSX_BUNDLE_NAME}.app/Contents/Resources")
		# The directory where the compiled plugins go to.
		SET(OVITO_RELATIVE_PLUGINS_DIRECTORY "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins")	
	ELSE(APPLE)
		SET(OVITO_MONOLITHIC_BUILD "ON")
		# The directory where the main executable goes to.
		SET(OVITO_RELATIVE_BINARY_DIRECTORY "bin")
		# The directory where the main libraries go to.
		SET(OVITO_RELATIVE_LIBRARY_DIRECTORY "lib/ovito")
		# The directory where the auxiliary files go to.
		SET(OVITO_RELATIVE_SHARE_DIRECTORY "share/ovito")
		# The directory where the compiled plugins go to.
		SET(OVITO_RELATIVE_PLUGINS_DIRECTORY "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins")
	ENDIF(APPLE)
ELSE(UNIX)
	SET(OVITO_MONOLITHIC_BUILD "OFF")
	# The directory where the main executable goes to.
	SET(OVITO_RELATIVE_BINARY_DIRECTORY ".")
	# The directory where the main libraries go to.
	SET(OVITO_RELATIVE_LIBRARY_DIRECTORY ".")
	# The directory where the auxiliary files go to.
	SET(OVITO_RELATIVE_SHARE_DIRECTORY ".")
	# The directory where the compiled plugins go to.
	SET(OVITO_RELATIVE_PLUGINS_DIRECTORY "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins")
ENDIF(UNIX)

# If a monothlitic build is enabled, a single static executable is created that contains all plugins.
# Otherwise the plugin and core libraries are created as shared objects. This is the default.
IF(OVITO_MONOLITHIC_BUILD)
	
	# Creating a static executable has only been tested for Linux systems.
	IF(NOT UNIX OR APPLE OR NOT CMAKE_COMPILER_IS_GNUCXX)
		MESSAGE(FATAL_ERROR "Creating a static executable is only supported on Linux using the GCC compiler. Please set OVITO_MONOLITHIC_BUILD to OFF.")
	ENDIF(NOT UNIX OR APPLE OR NOT CMAKE_COMPILER_IS_GNUCXX)

	# Create .a libraries.
	SET(BUILD_SHARED_LIBS "OFF")

	# Let the source code know that we're building a monolithic executable without
	# plugins residing in shared libraries.
	ADD_DEFINITIONS(-DOVITO_MONOLITHIC_BUILD)

ELSE(OVITO_MONOLITHIC_BUILD)

	# Create .so libraries.
	SET(BUILD_SHARED_LIBS "ON")

	# Setup RPATH
	SET(CMAKE_SKIP_BUILD_RPATH FALSE)
	# When building, use the install RPATH already
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	# The RPATH to be used when building and installing
	SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib/ovito")
	# add the automatically determined parts of the RPATH
	# which point to directories outside the build tree to the install RPATH
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

ENDIF(OVITO_MONOLITHIC_BUILD)

# The directory where the main executable goes to.
SET(OVITO_BINARY_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_BINARY_DIRECTORY}")
# The directory where the main libraries go to.
SET(OVITO_LIBRARY_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
# The directory where the compiled plugins go to.
SET(OVITO_PLUGINS_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
# The directory where the auxiliary files go to.
SET(OVITO_SHARE_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_SHARE_DIRECTORY}")

# Our installation path.
SET(OVITO_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# Add the plugins directory to the library path. Maybe some plugins link against other plugins.
LINK_DIRECTORIES(${OVITO_PLUGINS_DIRECTORY})

# Build program.
ADD_SUBDIRECTORY(src)

######################### Setup CPack #######################
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Packaging.cmake)
