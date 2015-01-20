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

PROJECT(Ovito)

# Make sure we have a recent version of CMake.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.8.12)
CMAKE_POLICY(SET CMP0023 OLD)
IF(NOT CMAKE_VERSION VERSION_LESS "3")
	CMAKE_POLICY(SET CMP0026 OLD)	# Suppress warning when reading the LOCATION target property.
	CMAKE_POLICY(SET CMP0043 OLD)	# Suppress warning because COMPILE_DEFINITIONS_* target property is used.
ENDIF()

IF(WIN32)
	CMAKE_POLICY(SET CMP0020 NEW) 	# Automatically link to qtmain.lib on Windows.

	IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "18")
			MESSAGE(FATAL_ERROR "OVITO requires Visual C++ 2013 or newer.")
		ENDIF()
	ENDIF()
ENDIF()

SET(OVITO_SOURCE_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
LIST(APPEND CMAKE_MODULE_PATH ${OVITO_SOURCE_BASE_DIR}/cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Version.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Plugins.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/PrecompiledHeader.cmake)

# Define user options that control the building of OVITO's standard plugins.
OPTION(OVITO_BUILD_PLUGIN_PARTICLES "Build the Particles plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_MESH "Build the Mesh plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_TACHYON "Build the Tachyon renderer plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS "Build the CrystalAnalysis plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_PYSCRIPT "Build the Python scripting plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_NETCDFPLUGIN "Build the NetCDF plugin." "ON")

# This is a global list of plugin targets that will be built.
# It will get populated by the OVITO_PLUGIN macro.
SET(OVITO_PLUGINS_LIST "")

# Enable software tests.
ENABLE_TESTING()

# Enable C++11 standard.
IF(NOT MSVC)
	IF(CMAKE_COMPILER_IS_GNUCXX)
	    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	ELSE()
    	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	ENDIF()
ENDIF()

IF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-invalid-offsetof")
ENDIF()

# Enable additional optimizations for release builds.
#IF(CMAKE_COMPILER_IS_GNUCXX)
#	# Use -flto flag to enable GCC's link-time optimization.
#	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto -fuse-linker-plugin")
#	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -fuse-linker-plugin")
#	SET(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -flto -fuse-linker-plugin")
#ENDIF()

IF(MSVC)
	# Suppress warning related to QVarLengthArray implementation.
	ADD_COMPILE_OPTIONS("/wd4996")
	# Suppress warning on conversion from size_t to int, possible loss of data.
	ADD_COMPILE_OPTIONS("/wd4267")
ENDIF()

IF(UNIX)
	IF(APPLE)
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
	# The directory where the main executable goes to.
	SET(OVITO_RELATIVE_BINARY_DIRECTORY ".")
	# The directory where the main libraries go to.
	SET(OVITO_RELATIVE_LIBRARY_DIRECTORY ".")
	# The directory where the auxiliary files go to.
	SET(OVITO_RELATIVE_SHARE_DIRECTORY ".")
	# The directory where the compiled plugins go to.
	SET(OVITO_RELATIVE_PLUGINS_DIRECTORY "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins")
ENDIF(UNIX)

# Controls whether a static or a shared OVITO executable is built.
# In a static build, all plugins (and most third-party libraries) are linked into one big executable,
# which makes distribution easier. In a shared build, plugins are built as shared objects (dynamic link libraries)
# and it becomes possible to add or remove plugins after distribution.
IF(UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCXX)
	OPTION(OVITO_MONOLITHIC_BUILD "Create one big executable that contains all plugins and third-party libraries." "OFF")
ENDIF()
IF(NOT OVITO_MONOLITHIC_BUILD)
    SET(OVITO_DEFAULT_LIBRARY_TYPE SHARED)
    
	# Add the automatically determined parts of the RPATH,
	# which point to directories outside the build tree to the install RPATH
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	    
ELSE()
    SET(OVITO_DEFAULT_LIBRARY_TYPE STATIC)
ENDIF()

# The directory where the main executable goes to.
SET(OVITO_BINARY_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_BINARY_DIRECTORY}")
# The directory where the main libraries go to.
SET(OVITO_LIBRARY_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
# The directory where the compiled plugins go to.
SET(OVITO_PLUGINS_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
# The directory where the auxiliary files go to.
SET(OVITO_SHARE_DIRECTORY "${${PROJECT_NAME}_BINARY_DIR}/${OVITO_RELATIVE_SHARE_DIRECTORY}")

# This is the name suffix used for generating plugin libraries.
SET(OVITO_PLUGIN_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
IF(APPLE)
	# On Mac OS, we use the .so extension instead of the standard .dylib to be compatible
	# with the Python interpreter, which only finds modules having a .so suffix.
	SET(OVITO_PLUGIN_LIBRARY_SUFFIX ".so")
ENDIF()

# This macro installs a required third-party DLL in the OVITO directory
# so that it can be distributed together with the program.
MACRO(OVITO_INSTALL_DLL dll)
	MESSAGE("Deploying DLL ${dll}.")
	EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" "-E" "copy_if_different" "${dll}" "${OVITO_BINARY_DIRECTORY}" RESULT_VARIABLE error_var)
	IF(error_var)
		MESSAGE(FATAL_ERROR "Failed to copy DLL into build directory: ${dll}")
	ENDIF()
	INSTALL(FILES "${dll}" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")
ENDMACRO()

# Tell CMake to run Qt moc whenever necessary.
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there.
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
  
# Find and set up Qt5 libraries.
FOREACH(component IN ITEMS Core Gui Widgets OpenGL Concurrent Network PrintSupport)
	FIND_PACKAGE(Qt5${component} REQUIRED)
ENDFOREACH()

# Find OpenGL library.
FIND_PACKAGE(OpenGL REQUIRED)
ADD_LIBRARY(OpenGLLib SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET OpenGLLib PROPERTY IMPORTED_LOCATION "${OPENGL_gl_LIBRARY}")
SET_PROPERTY(TARGET OpenGLLib APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}")

# Find the zlib library. It is required for reading/writing compressed simulation files.
FIND_PACKAGE(ZLIB REQUIRED)

# Find the Boost library.
IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	LIST(APPEND Boost_REQUIRED_COMPONENTS python)
ENDIF()
IF(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS)
	LIST(APPEND Boost_REQUIRED_COMPONENTS system thread)
ENDIF()
FIND_PACKAGE(Boost COMPONENTS ${Boost_REQUIRED_COMPONENTS} REQUIRED)
IF(NOT Boost_FOUND)
	MESSAGE(FATAL_ERROR "The Boost library could not be found on your system. ${Boost_ERROR_REASON}")
ENDIF()

# Generate build targets.
ADD_SUBDIRECTORY(src)

################# OpenGL test program ####################
OPTION(OVITO_BUILD_OPENGLTEST "Build the OpenGL test program." "OFF")
IF(OVITO_BUILD_OPENGLTEST)
	ADD_SUBDIRECTORY(tests/opengltest)
ENDIF()

################## Build documentation #####################
INCLUDE(cmake/Documentation.cmake)

##################### Set up CPack #########################
INCLUDE(cmake/Packaging.cmake)

############################################################

# Export our targets so that external plugins can use them.
CONFIGURE_FILE(cmake/OVITOConfig.cmake "${${PROJECT_NAME}_BINARY_DIR}/" @ONLY)
IF(NOT CMAKE_VERSION VERSION_LESS "3")
	EXPORT(EXPORT OVITO NAMESPACE "Ovito::" FILE OVITOTargets.cmake)
ENDIF()
