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

# Automatically link to qtmain.lib on Windows.
IF(WIN32)
	CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

GET_FILENAME_COMPONENT(OVITO_SOURCE_BASE_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${OVITO_SOURCE_BASE_DIR}/cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Version.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Plugins.cmake)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/CompileQtResource.cmake)

# Add options that control the building of plugins.
OPTION(OVITO_BUILD_PLUGIN_PARTICLES "Build the Particles plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_MESH "Build the Mesh plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_TACHYON "Build the Tachyon plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS "Build the CrystalAnalysis plugin." "OFF")
OPTION(OVITO_BUILD_PLUGIN_SCRIPTING "Build the Scripting plugin." "ON")
OPTION(OVITO_BUILD_PLUGIN_NETCDF "Build the NetCDF plugin." "OFF")

# Tell CMake to run Qt moc when necessary:
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
  
# Find and setup Qt5 libraries.
SET(OVITO_REQUIRED_QT_MODULES Core Gui Widgets Xml OpenGL Concurrent Network PrintSupport)
IF(OVITO_BUILD_PLUGIN_SCRIPTING)
	LIST(APPEND OVITO_REQUIRED_QT_MODULES Script)
ENDIF()
FOREACH(COMPONENT ${OVITO_REQUIRED_QT_MODULES})
	FIND_PACKAGE(Qt5${COMPONENT} REQUIRED)
ENDFOREACH()

# Use the Q_SIGNALS and Q_SLOTS macros to avoid name conflicts.
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

# Find OpenGL library.
FIND_PACKAGE(OpenGL REQUIRED)

# Find the ZLib library.
FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})

# Find the CGAL library.
IF(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS)
	FIND_PACKAGE(CGAL REQUIRED)
	IF(NOT CGAL_FOUND)
		MESSAGE(FATAL_ERROR "CGAL library, which is required by the CrystalAnalysis plugin, not found.")
	ENDIF()
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

# This is the base directory for all included header files.
INCLUDE_DIRECTORIES("${OVITO_SOURCE_BASE_DIR}/src")

# Static or shared (and extensible) version of Ovito?
OPTION(OVITO_MONOLITHIC_BUILD "Create one big executable that contains all plugins and third-party libraries." "OFF")

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

# Add the plugins directory to the library path. Some plugins may link against other plugins.
LINK_DIRECTORIES(${OVITO_PLUGINS_DIRECTORY})

# Controls compilation of the video encoding module.
OPTION(OVITO_VIDEO_OUTPUT_SUPPORT "Build the video encoding module." "OFF")
IF(OVITO_VIDEO_OUTPUT_SUPPORT)
	ADD_DEFINITIONS(-DOVITO_VIDEO_OUTPUT_SUPPORT)
ENDIF()

# Build program.
ADD_SUBDIRECTORY(src)

# On Window the third-party library DLLs are usually not part of the system. 
# We need to copy them to the OVITO directory.
IF(WIN32)
	# Gather Qt dynamic link libraries.
	FOREACH(COMPONENT ${OVITO_REQUIRED_QT_MODULES})
		GET_TARGET_PROPERTY(dll Qt5::${COMPONENT} LOCATION_${CMAKE_BUILD_TYPE})
		LIST(APPEND REQUIRED_DLLS "${dll}")
		IF(${COMPONENT} MATCHES "Core")
			GET_FILENAME_COMPONENT(QtBinaryPath ${dll} PATH)
			IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/icudt52.dll")		
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/icuin52.dll")		
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/icuuc52.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libharfbuzz-0.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libfreetype-6.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libpcre16-0.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/zlib1.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libbz2-1.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libglib-2.0-0.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libintl-8.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libpng16-16.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libiconv-2.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libjpeg-8.dll")
			ELSE()
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/icudt51.dll")		
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/icuin51.dll")		
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/icuuc51.dll")
			ENDIF()
			IF(MINGW)
				# 32-bit and 64-bit MinGW use different exception models and must link different
				# libgcc_s variants.
				IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
					LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libgcc_s_seh-1.dll")
				ELSE()
					LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libgcc_s_dw2-1.dll")
				ENDIF()
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libstdc++-6.dll")
				LIST(APPEND REQUIRED_DLLS "${QtBinaryPath}/libwinpthread-1.dll")
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
	
	# Install a qt.conf file.
	INSTALL(CODE "
	    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_BINARY_DIRECTORY}/qt.conf\" \"[Paths]\\nPlugins = plugins/\")
	    " COMPONENT Runtime)
		
	# Install zlib DLL.
	GET_FILENAME_COMPONENT(ZLIB_LIBRARY_PATH ${ZLIB_LIBRARIES} PATH)
	LIST(APPEND REQUIRED_DLLS "${ZLIB_LIBRARY_PATH}/../bin/libzlib.dll")

	# Install libav DLLS.
	IF(OVITO_VIDEO_OUTPUT_SUPPORT)
		SET(LIBAV_DLLS avcodec-55.dll avdevice-54.dll avformat-55.dll avutil-52.dll libmp3lame-0.dll
						libogg-0.dll librtmp.dll libvo-aacenc-0.dll libvorbis-0.dll libvorbisenc-2.dll 
						libvorbisfile-3.dll libx264-118.dll libx264-125.dll swscale-2.dll)
		FOREACH(dll ${LIBAV_DLLS})
			LIST(APPEND REQUIRED_DLLS "${FFMPEG_LIBRARY_DIR}/../bin/${dll}")
		ENDFOREACH()
	ENDIF()
	
	# Install CGAL, GMP, MPFR, and Boost DLLs.
	IF(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS)
		LIST(APPEND REQUIRED_DLLS ${CGAL_LIBRARY})
		IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
			LIST(APPEND REQUIRED_DLLS ${GMP_LIBRARIES})
			LIST(APPEND REQUIRED_DLLS ${MPFR_LIBRARIES})
		ENDIF()
		LIST(APPEND REQUIRED_DLLS "${CGAL_3RD_PARTY_LIBRARIES_DIRS}/libboost_thread-mt.dll")
		LIST(APPEND REQUIRED_DLLS "${CGAL_3RD_PARTY_LIBRARIES_DIRS}/libboost_system-mt.dll")
	ENDIF()

	# Install NetCDF and HDF5 dlls.
	IF(OVITO_BUILD_PLUGIN_NETCDF)
		GET_FILENAME_COMPONENT(NETCDF_LIBRARY_PATH "${NETCDF_LIBRARY}" PATH)
		GET_FILENAME_COMPONENT(HD5_LIBRARY_PATH "${HDF5_C_LIBRARY}" PATH)
		LIST(APPEND REQUIRED_DLLS "${NETCDF_LIBRARY_PATH}/../bin/libnetcdf.dll")
		LIST(APPEND REQUIRED_DLLS "${HD5_LIBRARY_PATH}/../bin/hdf5.dll")
		LIST(APPEND REQUIRED_DLLS "${HD5_LIBRARY_PATH}/../bin/hdf5_hl.dll")
	ENDIF()
	
	# Copy library DLLs to output directory.
	FOREACH(dll ${REQUIRED_DLLS})
		MESSAGE("Installing DLL library ${dll} in output directory.")
		CONFIGURE_FILE("${dll}" "${OVITO_BINARY_DIRECTORY}" COPYONLY)
		INSTALL(FILES "${dll}" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")
	ENDFOREACH()
ENDIF(WIN32)


################## Build documentation #####################

# Controls the generation of the documentation files.
OPTION(OVITO_BUILD_DOCUMENTATION "Build the documentation" "OFF")

IF(OVITO_BUILD_DOCUMENTATION)
	INCLUDE(cmake/Documentation.cmake)
	ADD_DEPENDENCIES(${PROJECT_NAME} documentation)
ENDIF(OVITO_BUILD_DOCUMENTATION)

######################### Setup CPack #######################
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Packaging.cmake)
