IF(APPLE)
	# Install the Info.plist file.
	CONFIGURE_FILE("${OVITO_SOURCE_BASE_DIR}/src/main/resources/Info.plist" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/Info.plist")

	# Copy the application icon into the resource directory.
	INSTALL(FILES "${OVITO_SOURCE_BASE_DIR}/src/main/resources/ovito.icns" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}")

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
	#IF(FALSE)
	INSTALL(CODE "
		CMAKE_POLICY(SET CMP0011 NEW)
		CMAKE_POLICY(SET CMP0009 NEW)
		# Returns the path that others should refer to the item by when the item is embedded inside a bundle.
		# This ensures that all plugin libraries go into the plugins/ directory of the bundle.
		FUNCTION(gp_item_default_embedded_path_override item default_embedded_path_var)
		    # Embed plugin libraries (.so) in the plugins/ subdirectory:
            IF(item MATCHES \"\\\\${OVITO_PLUGIN_LIBRARY_SUFFIX}$\" AND (item MATCHES \"^@rpath\" OR item MATCHES \"MacOS/plugins/\"))
	    	    SET(path \"@executable_path/plugins\")
		        SET(\${default_embedded_path_var} \"\${path}\" PARENT_SCOPE)
    		    MESSAGE(\"     Embedding path override: \${item} -> \${path}\")
            ENDIF()
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
	#ENDIF()
ENDIF()
