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

IF(CMAKE_BUILD_TYPE STREQUAL "Release")
	# This will create a GUI application on Windows platform.
	SET(OVITO_WIN32_EXECUTABLE "WIN32")
ELSE()
	# This will create a console application on Windows platform.
	SET(OVITO_WIN32_EXECUTABLE "")
ENDIF()

# Builds the main executable of the application
ADD_EXECUTABLE(${PROJECT_NAME} ${OVITO_WIN32_EXECUTABLE} MACOSX_BUNDLE Main.cpp ${WINDOWS_RESOURCES})

# The executable directly depends on the core modules.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Base Core)

# Set name of executable.
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ovito")

# Link Qt5.
QT5_USE_MODULES(${PROJECT_NAME} ${OVITO_REQUIRED_QT_MODULES})

# This executable will be part of the installation package.
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

IF(OVITO_MONOLITHIC_BUILD)

	# Build a monolithic executable that includes everything.
	# When statically linking, the linker will typically eliminate
	# unreferenced static objects in the final executable. To prevent this from
	# happening, we have to use the -whole-archive linker option. 
	LINK_WHOLE_LIBRARY(${PROJECT_NAME} Core)
	FOREACH(plugin ${OVITO_PLUGINS_LIST})
		LINK_WHOLE_LIBRARY(${PROJECT_NAME} ${plugin})
	ENDFOREACH()

	IF(UNIX AND NOT APPLE)
		# The static Qt plugins must be linked into the executable.
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} "${_qt5Core_install_prefix}/plugins/platforms/libqxcb.a")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} "${_qt5Core_install_prefix}/plugins/platforms/libqminimal.a")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} "/home/stuko/progs/qt-everywhere-opensource-src-5.2.0/qtbase/src/plugins/platforms/xcb/xcb-static/libxcb-static.a")
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} "${_qt5Core_install_prefix}/lib/libQt5PlatformSupport.a")

		# Some additional system libraries are required.
		TARGET_LINK_LIBRARIES(${PROJECT_NAME} "pthread" "dl" "rt" "Xext" "X11" "Xrender" "X11-xcb" "xcb" "GL" 
				"freetype" "libfontconfig.a" "libexpat.a" "libpcre.a" "libpcreposix.a" "Xi" "libSM.a" "libICE.a")

		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/ovito")
		SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH "1")
		FIND_LIBRARY(OVITO_LIBSTDCXX NAMES libstdc++.so.6 PATHS /usr/lib NO_DEFAULT_PATH)
		GET_FILENAME_COMPONENT(OVITO_LIBSTDCXX_REAL ${OVITO_LIBSTDCXX} REALPATH)
		INSTALL(FILES "${OVITO_LIBSTDCXX}" "${OVITO_LIBSTDCXX_REAL}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
		
		# Install a qt.conf file.
		# INSTALL(CODE "
		#	file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_BINARY_DIRECTORY}/qt.conf\" \"[Paths]\\nLibraries = ../${OVITO_RELATIVE_LIBRARY_DIRECTORY}/\")
		#	" COMPONENT Runtime)

		# Install Qt fonts.
		# INSTALL(DIRECTORY "${_qt5Core_install_prefix}/lib/fonts" DESTINATION ${OVITO_RELATIVE_LIBRARY_DIRECTORY} COMPONENT Runtime)

	ENDIF()

ENDIF()

IF(APPLE)
	# Install the Info.plist file.
	CONFIGURE_FILE("resources/Info.plist" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")

	# Copy the application icon into the resource directory.
	INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/ovito.icns" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}")

	SET(QT_PLUGINS_DIR "${_qt5Core_install_prefix}/plugins")

	# Install needed Qt plugins by copying directories from the qt installation
	# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
	SET(plugin_dest_dir "${MACOSX_BUNDLE_NAME}.app/Contents/MacOS")
	SET(qtconf_dest_dir "${MACOSX_BUNDLE_NAME}.app/Contents/Resources")
	INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
	INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/platforms" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)
	INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/accessible" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime PATTERN "*_debug.dylib" EXCLUDE)

	# Install a qt.conf file.
	# This inserts some cmake code into the install script to write the file
	INSTALL(CODE "
	    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]\\nPlugins = MacOS/plugins/\")
	    " COMPONENT Runtime)

	# Use BundleUtilities to get all other dependencies for the application to work.
	# It takes a bundle or executable along with possible plugins and inspects it
	# for dependencies.  If they are not system dependencies, they are copied.
	OVITO_FIXUP_BUNDLE()
	
	# To create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
	# where CPackConfig.cmake is created by including CPack.
	SET(CPACK_BINARY_DRAGNDROP ON)		
ENDIF()
