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

# This executable will be part of the installation package.
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

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
	ENDFOREACH()

	# Some additional system libraries are required.
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} "pthread" "dl" "rt" "Xext" "X11" "Xrender" "X11-xcb" "xcb" "GL" 
			"freetype" "fontconfig" "libexpat.a" "libpcre.a" "libpcreposix.a" "Xi" "libSM.a" "libICE.a" "util")

	# Make the executable look in lib/ovito/ directory for shared libraries.
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib/ovito")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH "1")
	FIND_LIBRARY(OVITO_LIBSTDCXX NAMES libstdc++.so.6 PATHS /usr/lib NO_DEFAULT_PATH)
	IF(NOT OVITO_LIBSTDCXX)
		MESSAGE(FATAL_ERROR "Please specify the location of the C++ standard library with the OVITO_LIBSTDCXX option.") 
	ENDIF()
	GET_FILENAME_COMPONENT(OVITO_LIBSTDCXX_REAL ${OVITO_LIBSTDCXX} REALPATH)
	INSTALL(FILES "${OVITO_LIBSTDCXX}" "${OVITO_LIBSTDCXX_REAL}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
	
ENDIF()

# On Windows, the third-party library DLLs need to be installed in the OVITO directory.
IF(WIN32)
	# Gather Qt dynamic link libraries.
	FOREACH(component IN ITEMS Core Gui Widgets OpenGL Concurrent Network PrintSupport)
		GET_TARGET_PROPERTY(dll Qt5::${component} LOCATION_${CMAKE_BUILD_TYPE})
		OVITO_INSTALL_DLL("${dll}")
		IF(${component} MATCHES "Core")
			GET_FILENAME_COMPONENT(QtBinaryPath ${dll} PATH)
			OVITO_INSTALL_DLL("${QtBinaryPath}/icudt52.dll")		
			OVITO_INSTALL_DLL("${QtBinaryPath}/icuin52.dll")		
			OVITO_INSTALL_DLL("${QtBinaryPath}/icuuc52.dll")
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
	OVITO_INSTALL_DLL("${ZLIB_LIBRARY_PATH}/../bin/zlib.dll")
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
	SET(APPS "${CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app")
		
	# Install needed Qt plugins by copying directories from the qt installation
	# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
	SET(plugin_dest_dir "${MACOSX_BUNDLE_NAME}.app/Contents/MacOS")
	SET(qtconf_dest_dir "${MACOSX_BUNDLE_NAME}.app/Contents/Resources")

	# Use BundleUtilities to get all other dependencies for the application to work.
	# It takes a bundle or executable along with possible plugins and inspects it
	# for dependencies.  If they are not system dependencies, they are copied.

	# Directories to look for dependencies
	SET(DIRS 
		${QT_LIBRARY_DIRS} 
		"${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}" 
		"${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/imageformats"
		"${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/platforms"
		"${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/accessible"
		"/opt/local/lib")
	
	# Now the work of copying dependencies into the bundle/package
	# The quotes are escaped and variables to use at install time have their $ escaped
	# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
	# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
	# over.
	INSTALL(CODE "
		CMAKE_POLICY(SET CMP0011 NEW)
		CMAKE_POLICY(SET CMP0009 NEW)
		# Returns the path that others should refer to the item by when the item is embedded inside a bundle.
		# This ensures that all plugin libraries go into the plugins/ directory of the bundle.
		FUNCTION(gp_item_default_embedded_path_override item default_embedded_path_var)
			# Let everything that is already in the bundle stay where it comes from.
	    	IF(item MATCHES \"^.*/.*\\\\${MACOSX_BUNDLE_NAME}.app/.*$\")
				FILE(RELATIVE_PATH relpath \"${CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/\" \"\${item}\")
				GET_FILENAME_COMPONENT(relpath2 \${relpath} PATH)
			    SET(path \"@executable_path/\${relpath2}\")
			ENDIF()
	    	IF(item MATCHES \"@executable_path\")
				GET_FILENAME_COMPONENT(path \"\${item}\" PATH)
			ENDIF()
	    	IF(item MATCHES \"@rpath\" AND item MATCHES \"${OVITO_PLUGIN_LIBRARY_SUFFIX}\")
				SET(path \"@executable_path/plugins\")
			ENDIF()
			SET(\${default_embedded_path_var} \"\${path}\" PARENT_SCOPE)
		    MESSAGE(\"Embedding path override: \${item}\ -> \${path}\")
		ENDFUNCTION()
		FILE(GLOB_RECURSE QTPLUGINS
			\"${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
		FILE(GLOB_RECURSE OVITO_PLUGINS
			\"${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}/*${OVITO_PLUGIN_LIBRARY_SUFFIX}\")
		FILE(GLOB_RECURSE PYTHON_DYNLIBS
			\"${CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app/Contents/Frameworks/Python.framework/*.so\")
		SET(BUNDLE_LIBS \${QTPLUGINS} \${OVITO_PLUGINS} \${PYTHON_DYNLIBS})
		SET(BU_CHMOD_BUNDLE_ITEMS ON)	# Make copies of system libraries writable before install_name_tool tries to change them.
		INCLUDE(BundleUtilities)
		FIXUP_BUNDLE(\"${APPS}\" \"\${BUNDLE_LIBS}\" \"${DIRS}\")
		" COMPONENT Runtime)
ENDIF()
