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

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
	# This will create a GUI application on Windows platform.
	SET(OVITO_WIN32_EXECUTABLE "WIN32")
ELSE()
	# This will create a console application on Windows platform.
	SET(OVITO_WIN32_EXECUTABLE "")
ENDIF()

# Builds the main executable of the application
ADD_EXECUTABLE(${PROJECT_NAME} ${OVITO_WIN32_EXECUTABLE} MACOSX_BUNDLE Main.cpp ${WINDOWS_RESOURCES})

# The executable depends on the core module of course.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Core)

# Put the executable into the right directory.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OVITO_BINARY_DIRECTORY}")

# Set name of executable.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ovito")

# Link Qt5.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt5::Core Qt5::Gui)

IF(APPLE)
    # Enable the use of @rpath on OSX.
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES MACOSX_RPATH TRUE)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "@executable_path/")
ELSEIF(UNIX)
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib/ovito/")
ENDIF()

# This executable will be part of the installation package.
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

# Build all plugins first before building the main executable.
FOREACH(plugin ${OVITO_PLUGIN_LIST})
    ADD_DEPENDENCIES(${PROJECT_NAME} ${plugin})
ENDFOREACH()

# Install fonts required by 'minimal' platform plugin on Linux
IF(UNIX AND NOT APPLE)
    INSTALL(DIRECTORY "resources/fonts" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/")
ENDIF()

IF(UNIX AND NOT APPLE AND OVITO_MONOLITHIC_BUILD)
	# Install copies of the Qt libraries.
	FOREACH(component IN ITEMS Core Gui Widgets OpenGL Concurrent Network PrintSupport)
		GET_TARGET_PROPERTY(lib Qt5::${component} LOCATION)
		GET_TARGET_PROPERTY(lib_soname Qt5::${component} IMPORTED_SONAME_RELEASE)
		CONFIGURE_FILE("${lib}" "${OVITO_LIBRARY_DIRECTORY}" COPYONLY)		
		GET_FILENAME_COMPONENT(lib_realname "${lib}" NAME)
		EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${lib_realname} "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/${lib_soname}")
		INSTALL(FILES "${lib}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
		INSTALL(FILES "${OVITO_LIBRARY_DIRECTORY}/${lib_soname}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
		GET_FILENAME_COMPONENT(QtBinaryPath ${lib} PATH)
	ENDFOREACH()

	# Create a qt.conf file.
	# This is required to let Qt search for its plugin modules in the program directory.
	INSTALL(CODE "
	    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_BINARY_DIRECTORY}/qt.conf\" \"[Paths]\\nPlugins = ../${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins_qt/\")
	    " COMPONENT Runtime)

	# Install Qt plugins.
	INSTALL(DIRECTORY "${QtBinaryPath}/../plugins/platforms" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins_qt/")
	INSTALL(DIRECTORY "${QtBinaryPath}/../plugins/bearer" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins_qt/")
	INSTALL(DIRECTORY "${QtBinaryPath}/../plugins/imageformats" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins_qt/")
	IF(Qt5Core_VERSION VERSION_LESS "5.4")
    	INSTALL(DIRECTORY "${QtBinaryPath}/../plugins/accessible" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/plugins_qt/")
    ENDIF()

	# Make the executable look in lib/ovito/ directory for shared libraries.
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/ovito")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH "1")
	FIND_LIBRARY(OVITO_LIBSTDCXX NAMES libstdc++.so.6 PATHS /usr/lib NO_DEFAULT_PATH)
	IF(NOT OVITO_LIBSTDCXX)
		MESSAGE(FATAL_ERROR "Please specify the location of the C++ standard library with the OVITO_LIBSTDCXX option.") 
	ENDIF()
	GET_FILENAME_COMPONENT(OVITO_LIBSTDCXX_REAL ${OVITO_LIBSTDCXX} REALPATH)
	INSTALL(FILES "${OVITO_LIBSTDCXX}" "${OVITO_LIBSTDCXX_REAL}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
	
	# Install Boost libraries.
	FOREACH(lib ${Boost_LIBRARIES})
		FILE(GLOB lib_versions "${lib}*")
		FOREACH(lib_version ${lib_versions})
			INSTALL(FILES "${lib_version}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
		ENDFOREACH()
	ENDFOREACH()

ENDIF()

# On Windows, the third-party library DLLs need to be installed in the OVITO directory.
IF(WIN32)
	# Gather Qt dynamic link libraries.
	FOREACH(component IN ITEMS Core Gui Widgets OpenGL Concurrent Network PrintSupport)
		GET_TARGET_PROPERTY(dll Qt5::${component} LOCATION_${CMAKE_BUILD_TYPE})
		OVITO_INSTALL_DLL("${dll}")
		IF(${component} MATCHES "Core")
			GET_FILENAME_COMPONENT(QtBinaryPath ${dll} PATH)
			OVITO_INSTALL_DLL("${QtBinaryPath}/icudt53.dll")		
			OVITO_INSTALL_DLL("${QtBinaryPath}/icuin53.dll")		
			OVITO_INSTALL_DLL("${QtBinaryPath}/icuuc53.dll")
		ENDIF()
	ENDFOREACH()
	
	# Install Qt plugins.
	IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
		INSTALL(FILES "${QtBinaryPath}/../plugins/platforms/qwindowsd.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/platforms/")
		INSTALL(FILES "${QtBinaryPath}/../plugins/imageformats/qjpegd.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/imageformats/")
    	IF(Qt5Core_VERSION VERSION_LESS "5.4")
	    	INSTALL(FILES "${QtBinaryPath}/../plugins/accessible/qtaccessiblewidgetsd.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/accessible/")
	    ENDIF()
	ELSE()
		INSTALL(FILES "${QtBinaryPath}/../plugins/platforms/qwindows.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/platforms/")
		INSTALL(FILES "${QtBinaryPath}/../plugins/imageformats/qjpeg.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/imageformats/")
    	IF(Qt5Core_VERSION VERSION_LESS "5.4")
    		INSTALL(FILES "${QtBinaryPath}/../plugins/accessible/qtaccessiblewidgets.dll" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}/plugins/accessible/")
    	ENDIF()
	ENDIF()
	
	# Create a qt.conf file.
	# This is required to let Qt search for its plugin modules in the program directory.
	INSTALL(CODE "
	    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_BINARY_DIRECTORY}/qt.conf\" \"[Paths]\\nPlugins = plugins/\")
	    " COMPONENT Runtime)
		
	# Install zlib DLL.
	LIST(GET ZLIB_LIBRARIES 0 ZLIB_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(ZLIB_LIBRARY_PATH "${ZLIB_FIRST_LIBRARY}" PATH)
	OVITO_INSTALL_DLL("${ZLIB_LIBRARY_PATH}/../bin/zlib.dll")

	# Install Boost libraries.
	FOREACH(lib ${Boost_LIBRARIES})
		IF(lib STREQUAL "optimized")
			SET(is_debug_libs FALSE)
		ELSEIF(lib STREQUAL "debug")
			SET(is_debug_libs TRUE)
		ELSEIF((NOT is_debug_libs AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug") OR (is_debug_libs AND CMAKE_BUILD_TYPE STREQUAL "Debug"))
			GET_FILENAME_COMPONENT(base_name "${lib}" NAME_WE)
			GET_FILENAME_COMPONENT(lib_path "${lib}" PATH)
			OVITO_INSTALL_DLL("${lib_path}/${base_name}.dll")
		ENDIF()
	ENDFOREACH()

ENDIF()

# Fix Mac app bundle.
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/FixupMacBundle.cmake)

