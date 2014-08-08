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
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12 FATAL_ERROR)
CMAKE_POLICY(SET CMP0023 OLD)

IF(WIN32)
	# Automatically link to qtmain.lib on Windows.
	CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

GET_FILENAME_COMPONENT(OVITO_SOURCE_BASE_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${OVITO_SOURCE_BASE_DIR}/cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Version.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Plugins.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/CompileQtResource.cmake)

# Define user options that control the building of OVITO's standard plugins.
OPTION(OVITO_BUILD_PLUGIN_PARTICLES "Build the Particles plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_MESH "Build the Mesh plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_TACHYON "Build the Tachyon plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS "Build the CrystalAnalysis plugin." "OFF")
OPTION(OVITO_BUILD_PLUGIN_SCRIPTING "Build the Qt Scripting plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_PYSCRIPT "Build the Python Scripting plugin." "OFF")
OPTION(OVITO_BUILD_PLUGIN_NETCDF "Build the NetCDF plugin." "OFF")

# Controls whether a static or a shared OVITO executable is built.
# In a static build, all plugins (and most third-party libraries) are linked into one big executable,
# which makes distribution easier. In a shared build, plugins are built as shared objects (dynamic link libraries)
# and it becomes possible to add or remove plugins after distribution.
IF(UNIX AND NOT APPLE AND CMAKE_COMPILER_IS_GNUCXX)
	OPTION(OVITO_MONOLITHIC_BUILD "Create one big executable that contains all plugins and third-party libraries." "OFF")
ENDIF()

# Define the OVITO_DEBUG macro in debug builds.
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DOVITO_DEBUG")

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

# Set the base directory for all header files included by the OVITO source code.
INCLUDE_DIRECTORIES("${OVITO_SOURCE_BASE_DIR}/src")

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

# If a static build is selected, a single executable is created that contains all plugins and most third-party libraries.
# Otherwise the plugin and core libraries are created as shared objects. This is the default.
IF(OVITO_MONOLITHIC_BUILD)

	# Create only static .a libraries on Linux.
	SET(BUILD_SHARED_LIBS "OFF")

	# Let the source code know that we're building a monolithic executable. 
	# This disables the code that is responsible for dynamic loading of plugins.
	ADD_DEFINITIONS(-DOVITO_MONOLITHIC_BUILD)

ELSE(OVITO_MONOLITHIC_BUILD)

	# Create dynamic libraries.
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

# This is the name suffix used for generating plugin libraries.
SET(OVITO_PLUGIN_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
IF(APPLE)
	# On Mac OS, we use the .so extension instead of the standard .dylib to be compatible
	# with the Python interpreter, which only finds modules having a .so suffix.
	SET(OVITO_PLUGIN_LIBRARY_SUFFIX ".so")
ENDIF()

# Controls compilation of the video encoding feature.
OPTION(OVITO_VIDEO_OUTPUT_SUPPORT "Build the video encoding module." "OFF")

# This macro installs a required third-party DLL in the OVITO directory
# so that it can be distributed together with the program.
MACRO(OVITO_INSTALL_DLL dll)
	MESSAGE("Installing DLL library ${dll} in output directory.")
	CONFIGURE_FILE("${dll}" "${OVITO_BINARY_DIRECTORY}" COPYONLY)
	INSTALL(FILES "${dll}" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")
ENDMACRO()

# Tell CMake to run Qt moc whenever necessary.
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there.
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
  
# Find and set up Qt5 libraries.
SET(OVITO_REQUIRED_QT_MODULES Core Gui Widgets Xml OpenGL Concurrent Network PrintSupport)
IF(OVITO_BUILD_PLUGIN_SCRIPTING)
	LIST(APPEND OVITO_REQUIRED_QT_MODULES Script)
ENDIF()
FOREACH(COMPONENT ${OVITO_REQUIRED_QT_MODULES})
	FIND_PACKAGE(Qt5${COMPONENT} REQUIRED)
ENDFOREACH()

# Find OpenGL library.
FIND_PACKAGE(OpenGL REQUIRED)
ADD_LIBRARY(OpenGLLib SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET OpenGLLib PROPERTY IMPORTED_LOCATION "${OPENGL_gl_LIBRARY}")
SET_PROPERTY(TARGET OpenGLLib APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${OPENGL_INCLUDE_DIR}")

# Find the zlib library. It is required for reading/writing compressed simulation files.
FIND_PACKAGE(ZLIB REQUIRED)

# Find the Boost.Python library, which is required by the scripting plugin.
IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	SET(Boost_USE_STATIC_LIBS ${OVITO_MONOLITHIC_BUILD})
	FIND_PACKAGE(Boost COMPONENTS python REQUIRED)
	IF(NOT Boost_FOUND)
		MESSAGE(FATAL_ERROR "The Boost library could not be found on your system. ${Boost_ERROR_REASON}")
	ENDIF()
ENDIF()

# Build program.
ADD_SUBDIRECTORY(src)

# On Windows, the third-party library DLLs need to be installed in the OVITO directory.
IF(WIN32)
	# Gather Qt dynamic link libraries.
	FOREACH(COMPONENT ${OVITO_REQUIRED_QT_MODULES})
		GET_TARGET_PROPERTY(dll Qt5::${COMPONENT} LOCATION_${CMAKE_BUILD_TYPE})
		OVITO_INSTALL_DLL("${dll}")
		IF(${COMPONENT} MATCHES "Core")
			GET_FILENAME_COMPONENT(QtBinaryPath ${dll} PATH)
			IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
				OVITO_INSTALL_DLL("${QtBinaryPath}/icudt52.dll")		
				OVITO_INSTALL_DLL("${QtBinaryPath}/icuin52.dll")		
				OVITO_INSTALL_DLL("${QtBinaryPath}/icuuc52.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libharfbuzz-0.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libfreetype-6.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libpcre16-0.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/zlib1.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libbz2-1.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libglib-2.0-0.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libintl-8.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libpng16-16.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libiconv-2.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libjpeg-8.dll")
			ELSE()
				OVITO_INSTALL_DLL("${QtBinaryPath}/icudt51.dll")		
				OVITO_INSTALL_DLL("${QtBinaryPath}/icuin51.dll")		
				OVITO_INSTALL_DLL("${QtBinaryPath}/icuuc51.dll")
			ENDIF()
			IF(MINGW)
				# 32-bit and 64-bit MinGW use different exception models and must link different
				# libgcc_s variants.
				IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
					OVITO_INSTALL_DLL("${QtBinaryPath}/libgcc_s_seh-1.dll")
				ELSE()
					OVITO_INSTALL_DLL("${QtBinaryPath}/libgcc_s_dw2-1.dll")
				ENDIF()
				OVITO_INSTALL_DLL("${QtBinaryPath}/libstdc++-6.dll")
				OVITO_INSTALL_DLL("${QtBinaryPath}/libwinpthread-1.dll")
			ENDIF()
		ENDIF()
	ENDFOREACH()
	
	# Install Qt plugins.
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		INSTALL(FILES "${QtBinaryPath}/../plugins/platforms/qwindowsd.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/platforms/")
		INSTALL(FILES "${QtBinaryPath}/../plugins/imageformats/qjpegd.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/imageformats/")
		INSTALL(FILES "${QtBinaryPath}/../plugins/accessible/qtaccessiblewidgetsd.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/accessible/")
	ELSE()
		INSTALL(FILES "${QtBinaryPath}/../plugins/platforms/qwindows.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/platforms/")
		INSTALL(FILES "${QtBinaryPath}/../plugins/imageformats/qjpeg.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/imageformats/")
		INSTALL(FILES "${QtBinaryPath}/../plugins/accessible/qtaccessiblewidgets.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/accessible/")
	ENDIF()
	
	# Create a qt.conf file.
	# This is required to let Qt search for its plugin modules in the program directory.
	INSTALL(CODE "
	    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_BINARY_DIRECTORY}/qt.conf\" \"[Paths]\\nPlugins = plugins/\")
	    " COMPONENT Runtime)
		
	# Install zlib DLL.
	LIST(GET ZLIB_LIBRARIES 0 ZLIB_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(ZLIB_LIBRARY_PATH "${ZLIB_FIRST_LIBRARY}" PATH)
	SET(ZLIB_DLL_PATH "${ZLIB_LIBRARY_PATH}/../bin" CACHE PATH "Directory where the ZLib dll is located.")
	MARK_AS_ADVANCED(ZLIB_DLL_PATH)
	OVITO_INSTALL_DLL("${ZLIB_DLL_PATH}/libzlib.dll")

	# Install libav DLLS.
	IF(OVITO_VIDEO_OUTPUT_SUPPORT)
		SET(LIBAV_DLLS avcodec-55.dll avdevice-54.dll avformat-55.dll avutil-52.dll libmp3lame-0.dll
						libogg-0.dll librtmp.dll libvo-aacenc-0.dll libvorbis-0.dll libvorbisenc-2.dll 
						libvorbisfile-3.dll libx264-118.dll libx264-125.dll swscale-2.dll)
		FOREACH(dll ${LIBAV_DLLS})
			OVITO_INSTALL_DLL("${FFMPEG_LIBRARY_DIR}/../bin/${dll}")
		ENDFOREACH()
	ENDIF()
ENDIF()

################## Build documentation #####################
INCLUDE(cmake/Documentation.cmake)

##################### Set up CPack #########################
INCLUDE(cmake/Packaging.cmake)
