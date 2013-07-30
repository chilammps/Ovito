MACRO(OVITO_ADD_PLUGIN_DEPENDENCY target_name plugin_name)

	STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
	IF(NOT OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
		STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
		MESSAGE(FATAL_ERROR "To build the ${target_name} plugin, the ${plugin_name} plugin has to be enabled too. Please set the OVITO_BUILD_PLUGIN_${uppercase_plugin_name} option to ON.")
	ENDIF()

	TARGET_LINK_LIBRARIES(${target_name} ${plugin_name})

ENDMACRO(OVITO_ADD_PLUGIN_DEPENDENCY)

# This macro adds a static library target to an executable target and makes sure that
# the library is linked completely into the executable without removal of unreferenced
# objects.
MACRO(LINK_WHOLE_LIBRARY _targetName _libraryTarget)
	IF(UNIX AND NOT APPLE)
		GET_TARGET_PROPERTY(EXE_LINKER_OPTIONS ${_targetName} LINK_FLAGS)
		IF(NOT EXE_LINKER_OPTIONS)
			SET(EXE_LINKER_OPTIONS "")
		ENDIF(NOT EXE_LINKER_OPTIONS)
		GET_TARGET_PROPERTY(TARGET_LOCATION ${_libraryTarget} LOCATION)
		SET(EXE_LINKER_OPTIONS "${EXE_LINKER_OPTIONS} -Wl,--whole-archive ${TARGET_LOCATION} -Wl,--no-whole-archive ")
		SET_TARGET_PROPERTIES(${_targetName} PROPERTIES LINK_FLAGS ${EXE_LINKER_OPTIONS})
	ENDIF()
	TARGET_LINK_LIBRARIES(${_targetName} ${_libraryTarget})
ENDMACRO(LINK_WHOLE_LIBRARY)

# Create a new target for a plugin.
MACRO(OVITO_PLUGIN target_name)

	SET(plugin_sources)
	SET(lib_dependencies)
	SET(resource_output)
	SET(resource_input)
	SET(plugin_dependencies)
	SET(optional_plugin_dependencies)
	SET(pch_header)
	FOREACH(currentArg ${ARGN})
		IF("${currentArg}" STREQUAL "SOURCES" OR 
			"${currentArg}" STREQUAL "LIB_DEPENDENCIES" OR 
			"${currentArg}" STREQUAL "PLUGIN_DEPENDENCIES" OR 
			"${currentArg}" STREQUAL "OPTIONAL_PLUGIN_DEPENDENCIES" OR
			"${currentArg}" STREQUAL "PCH" OR  
			"${currentArg}" STREQUAL "RESOURCE")
			SET(DOING_WHAT "${currentArg}")
		ELSE()
		    IF(${DOING_WHAT} STREQUAL "SOURCES")
				LIST(APPEND plugin_sources "${currentArg}")
		    ELSEIF(${DOING_WHAT} STREQUAL "LIB_DEPENDENCIES")
				LIST(APPEND lib_dependencies "${currentArg}")
		    ELSEIF(${DOING_WHAT} STREQUAL "PLUGIN_DEPENDENCIES")
				LIST(APPEND plugin_dependencies "${currentArg}")
		    ELSEIF(${DOING_WHAT} STREQUAL "OPTIONAL_PLUGIN_DEPENDENCIES")
				LIST(APPEND optional_plugin_dependencies "${currentArg}")
		    ELSEIF(${DOING_WHAT} STREQUAL "PCH")
				SET(pch_header "${currentArg}")
		    ELSEIF(${DOING_WHAT} STREQUAL "RESOURCE")
				SET(resource_output "${currentArg}")
				SET(DOING_WHAT "RESOURCE_INPUT")
		    ELSEIF(${DOING_WHAT} STREQUAL "RESOURCE_INPUT")
				LIST(APPEND resource_input "${currentArg}")
		    ENDIF()
		ENDIF()
	ENDFOREACH(currentArg)

	# This is needed to export the symbols in this shared library.
	STRING(TOUPPER "${target_name}" uppercase_target_name)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMAKING_MODULE_${uppercase_target_name}")

	# Create the library target for the plugin.
	ADD_LIBRARY(${target_name} ${plugin_sources})

	# Link mandatory OVITO libraries.
	TARGET_LINK_LIBRARIES(${target_name} Base Core)

	# Link required OVITO libraries.
	TARGET_LINK_LIBRARIES(${target_name} ${lib_dependencies})

	# Link Qt5.
	QT5_USE_MODULES(${target_name} Widgets Xml Concurrent Network PrintSupport)

	# Link plugin dependencies.
	FOREACH(plugin_name ${plugin_dependencies})
		OVITO_ADD_PLUGIN_DEPENDENCY(${target_name} ${plugin_name})
	ENDFOREACH(plugin_name)

	# Link optional plugin dependencies.
	FOREACH(plugin_name ${optional_plugin_dependencies})
		STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
		IF(OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
			OVITO_ADD_PLUGIN_DEPENDENCY(${target_name} ${plugin_name})
		ENDIF()
	ENDFOREACH(plugin_name)

	IF(APPLE)
		# Assign an absolute install path to this dynamic link library.
		SET_TARGET_PROPERTIES(${target_name} PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
		# This is required to avoid error by install_name_tool.
		SET_TARGET_PROPERTIES(${target_name} PROPERTIES LINK_FLAGS "-headerpad_max_install_names")
	ENDIF(APPLE)
	
	# Copy Plugin manifest to destination directory.
	CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/resources/${target_name}.manifest.xml"
               "${OVITO_PLUGINS_DIRECTORY}/${target_name}.manifest.xml")	

	# This plugin will be part of the installation package.
	IF(NOT OVITO_MONOLITHIC_BUILD)
		INSTALL(TARGETS ${target_name} 
			RUNTIME DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}"
			LIBRARY DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
	ENDIF(NOT OVITO_MONOLITHIC_BUILD)

	# Build optional resource file for this plugin which is not linked into the shared library.
	IF(resource_output)
		QT_COMPILE_RESOURCES(${target_name} "${OVITO_PLUGINS_DIRECTORY}/${resource_output}" ${resource_input})
		# The resource file will be part of the installation package.
		INSTALL(FILES "${OVITO_PLUGINS_DIRECTORY}/${resource_output}" DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
	ENDIF()

ENDMACRO(OVITO_PLUGIN)

# Fixes the Ovito installation bundle on MacOS.
MACRO(OVITO_FIXUP_BUNDLE)
		SET(APPS "${OVITO_CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app")
		
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
			"${OVITO_CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}" 
			"${OVITO_CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/imageformats"
			"${OVITO_CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/platforms"
			"${OVITO_CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/accessible")
		IF(OVITO_VIDEO_OUTPUT_SUPPORT)
			SET(DIRS ${DIRS} "/opt/local/lib")
		ENDIF()
		
		# Now the work of copying dependencies into the bundle/package
		# The quotes are escaped and variables to use at install time have their $ escaped
		# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
		# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
		# over.
		INSTALL(CODE "
			# Returns the path that others should refer to the item by when the item is embedded inside a bundle.
			# This ensures that all plugin libraries go into the plugins/ directory of the bundle.
			FUNCTION(gp_item_default_embedded_path_override item default_embedded_path_var)
				# Let everything that is already in the bundle stay where it comes from.
		    	if(item MATCHES \"^.*/.*\\\\${MACOSX_BUNDLE_NAME}.app/.*$\")
					FILE(RELATIVE_PATH relpath \"${OVITO_CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/\" \"\${item}\")
					GET_FILENAME_COMPONENT(relpath2 \${relpath} PATH)
				    SET(path \"@executable_path/\${relpath2}\")
				endif()
		    	if(item MATCHES \"@executable_path\")
					GET_FILENAME_COMPONENT(path \"\${item}\" PATH)
				endif()
				SET(\${default_embedded_path_var} \"\${path}\" PARENT_SCOPE)
			    MESSAGE(\"Embedding path override: \${item}\ -> \${path}\")
			ENDFUNCTION(gp_item_default_embedded_path_override)
			file(GLOB_RECURSE QTPLUGINS
				\"${OVITO_CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
			file(GLOB_RECURSE OVITO_PLUGINS
				\"${OVITO_CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
			set(BUNDLE_LIBS \${QTPLUGINS} \${OVITO_PLUGINS})
			include(BundleUtilities)
			fixup_bundle(\"${APPS}\" \"\${BUNDLE_LIBS}\" \"${DIRS}\")
			" COMPONENT Runtime)
ENDMACRO()
