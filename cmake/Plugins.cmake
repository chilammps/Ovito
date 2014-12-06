MACRO(OVITO_ADD_PLUGIN_DEPENDENCY target_name dependency_name)
	STRING(TOUPPER "${dependency_name}" uppercase_plugin_name)
	IF(NOT OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
		MESSAGE(FATAL_ERROR "To build the ${target_name} plugin, the ${dependency_name} plugin has to be enabled too. Please set the OVITO_BUILD_PLUGIN_${uppercase_plugin_name} option to ON.")
	ENDIF()

	TARGET_LINK_LIBRARIES(${target_name} ${dependency_name})
ENDMACRO()

# Fix the library paths in the build directory.
# This is required so that OVITO can be run from the build directory (and not only from the installation directory).
MACRO(OVITO_FIXUP_BUILD_OBJECT target_name)
	IF(APPLE)
		IF(${ARGC} EQUAL 1)
			SET(dependency_list "")
			GET_PROPERTY(TARGET_LINK_LIBS TARGET ${target_name} PROPERTY LINK_LIBRARIES)
		ELSE()
			GET_PROPERTY(TARGET_LINK_LIBS TARGET ${ARGV1} PROPERTY LINK_LIBRARIES)
		ENDIF()
		FOREACH(dependency ${TARGET_LINK_LIBS})
			IF(NOT IS_ABSOLUTE "${dependency}" AND NOT ";${dependency_list};" MATCHES ";${dependency};")
				GET_PROPERTY(IS_IMPORTED TARGET ${dependency} PROPERTY IMPORTED)
				IF(NOT ${IS_IMPORTED})
					GET_PROPERTY(TARGET_TYPE TARGET ${dependency} PROPERTY TYPE)
					IF(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
						LIST(APPEND dependency_list ${dependency})
						GET_PROPERTY(TARGET_LOCATION TARGET ${dependency} PROPERTY LOCATION)
						GET_FILENAME_COMPONENT(TARGET_FILE "${TARGET_LOCATION}" NAME)
						IF(";${OVITO_PLUGINS_LIST};" MATCHES ";${dependency};")
							ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD 
								COMMAND "install_name_tool" "-change" "${TARGET_FILE}" "@executable_path/plugins/${TARGET_FILE}" "$<TARGET_FILE:${target_name}>")
						ELSE()
							ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD 
								COMMAND "install_name_tool" "-change" "${TARGET_FILE}" "@executable_path/${TARGET_FILE}" "$<TARGET_FILE:${target_name}>")
						ENDIF()
						OVITO_FIXUP_BUILD_OBJECT(${target_name} ${dependency}) 
					ENDIF()
				ENDIF()
			ENDIF()
		ENDFOREACH()
	ENDIF()
ENDMACRO()

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
ENDMACRO()

# Create a OVITO plugin.
MACRO(OVITO_PLUGIN target_name)

	SET(plugin_sources)
	SET(lib_dependencies)
	SET(resource_output)
	SET(resource_input)
	SET(plugin_dependencies)
	SET(optional_plugin_dependencies)
	FOREACH(currentArg ${ARGN})
		IF("${currentArg}" STREQUAL "SOURCES" OR 
			"${currentArg}" STREQUAL "LIB_DEPENDENCIES" OR 
			"${currentArg}" STREQUAL "PLUGIN_DEPENDENCIES" OR 
			"${currentArg}" STREQUAL "OPTIONAL_PLUGIN_DEPENDENCIES" OR
			"${currentArg}" STREQUAL "PYTHON_WRAPPERS" OR
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
		    ELSEIF(${DOING_WHAT} STREQUAL "PYTHON_WRAPPERS")
				LIST(APPEND python_wrappers "${currentArg}")
		    ELSEIF(${DOING_WHAT} STREQUAL "RESOURCE")
				SET(resource_output "${currentArg}")
				SET(DOING_WHAT "RESOURCE_INPUT")
		    ELSEIF(${DOING_WHAT} STREQUAL "RESOURCE_INPUT")
				LIST(APPEND resource_input "${currentArg}")
		    ENDIF()
		ENDIF()
	ENDFOREACH(currentArg)

	# Create the library target for the plugin.
	ADD_LIBRARY(${target_name} ${plugin_sources})

	# Link to OVITO's core library.
	TARGET_LINK_LIBRARIES(${target_name} PUBLIC Core)

	# Link other required libraries.
	TARGET_LINK_LIBRARIES(${target_name} PUBLIC ${lib_dependencies})

	# Link Qt5.
	TARGET_LINK_LIBRARIES(${target_name} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Concurrent)

	# Link plugin dependencies.
	FOREACH(plugin_name ${plugin_dependencies})
		OVITO_ADD_PLUGIN_DEPENDENCY(${target_name} ${plugin_name})
	ENDFOREACH()

	# Link optional plugin dependencies.
	FOREACH(plugin_name ${optional_plugin_dependencies})
		STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
		IF(OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
			OVITO_ADD_PLUGIN_DEPENDENCY(${target_name} ${plugin_name})
		ENDIF()
	ENDFOREACH()
	
	IF(NOT OVITO_MONOLITHIC_BUILD)
		# Set prefix and suffix of library name.
		# This is needed so that the Python interpreter can load OVITO plugins as modules.
		SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "" SUFFIX "${OVITO_PLUGIN_LIBRARY_SUFFIX}")
	ENDIF()

	IF(APPLE)
		# Assign an absolute install path to this dynamic link library.
		# SET_TARGET_PROPERTIES(${target_name} PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
		# This is required to avoid error by install_name_tool.
		SET_TARGET_PROPERTIES(${target_name} PROPERTIES LINK_FLAGS "-headerpad_max_install_names")
	ENDIF(APPLE)
	
	# Generate plugin manifest.
	SET(PLUGIN_MANIFEST "${OVITO_PLUGINS_DIRECTORY}/${target_name}.json")
	FILE(WRITE "${PLUGIN_MANIFEST}" "{\n  \"plugin-id\" : \"${target_name}\",\n")
	FILE(APPEND "${PLUGIN_MANIFEST}" "  \"plugin-version\" : \"${OVITO_VERSION_STRING}\",\n")
	FILE(APPEND "${PLUGIN_MANIFEST}" "  \"dependencies\" : [ ")
	FOREACH(plugin_name ${plugin_dependencies})
		FILE(APPEND "${PLUGIN_MANIFEST}" "${delimiter}\"${plugin_name}\"")
		SET(delimiter ", ")
	ENDFOREACH()
	FOREACH(plugin_name ${optional_plugin_dependencies})
		STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
		IF(OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
			FILE(APPEND "${PLUGIN_MANIFEST}" "${delimiter}\"${plugin_name}\"")
			SET(delimiter ", ")
		ENDIF()
	ENDFOREACH()
	FILE(APPEND "${PLUGIN_MANIFEST}" " ],\n")
	IF(resource_output)
		FILE(APPEND "${PLUGIN_MANIFEST}" "  \"resource-files\" : [ \"${resource_output}\" ],\n")
	ENDIF()
	FILE(APPEND "${PLUGIN_MANIFEST}" "  \"native-library\" : \"${target_name}\"\n}\n")
	INSTALL(FILES "${PLUGIN_MANIFEST}" DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")

	# Install Python wrapper files.
	IF(python_wrappers)
		# Install the Python source files that belong to the plugin, which provide the scripting interface.
		ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD COMMAND ${CMAKE_COMMAND} "-E" copy_directory "${python_wrappers}" "${OVITO_PLUGINS_DIRECTORY}/python/")
	ENDIF()

	# This plugin will be part of the installation package.
	IF(NOT OVITO_MONOLITHIC_BUILD)
		INSTALL(TARGETS ${target_name} 
			RUNTIME DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}"
			LIBRARY DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
	ENDIF(NOT OVITO_MONOLITHIC_BUILD)

	# Generate resource file for this plugin.
	IF(resource_output)
		QT_COMPILE_RESOURCES(${target_name} "${OVITO_PLUGINS_DIRECTORY}/${resource_output}" ${resource_input})
		# Make resource file part of the installation package.
		INSTALL(FILES "${OVITO_PLUGINS_DIRECTORY}/${resource_output}" DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
	ENDIF()
	
	# Keep a list of plugins.
	LIST(APPEND OVITO_PLUGINS_LIST ${target_name})
	SET(OVITO_PLUGINS_LIST "${OVITO_PLUGINS_LIST}" PARENT_SCOPE)

	OVITO_FIXUP_BUILD_OBJECT(${target_name})

ENDMACRO()

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
			"${OVITO_CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/accessible"
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
					FILE(RELATIVE_PATH relpath \"${OVITO_CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/\" \"\${item}\")
					GET_FILENAME_COMPONENT(relpath2 \${relpath} PATH)
				    SET(path \"@executable_path/\${relpath2}\")
				ENDIF()
		    	IF(item MATCHES \"@executable_path\")
					GET_FILENAME_COMPONENT(path \"\${item}\" PATH)
				ENDIF()
				SET(\${default_embedded_path_var} \"\${path}\" PARENT_SCOPE)
			    MESSAGE(\"Embedding path override: \${item}\ -> \${path}\")
			ENDFUNCTION(gp_item_default_embedded_path_override)
			FILE(GLOB_RECURSE QTPLUGINS
				\"${OVITO_CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
			FILE(GLOB_RECURSE OVITO_PLUGINS
				\"${OVITO_CMAKE_INSTALL_PREFIX}/${OVITO_RELATIVE_PLUGINS_DIRECTORY}/*${OVITO_PLUGIN_LIBRARY_SUFFIX}\")
			FILE(GLOB_RECURSE PYTHON_DYNLIBS
				\"${OVITO_CMAKE_INSTALL_PREFIX}/${MACOSX_BUNDLE_NAME}.app/Contents/Frameworks/Python.framework/*.so\")
			SET(BUNDLE_LIBS \${QTPLUGINS} \${OVITO_PLUGINS} \${PYTHON_DYNLIBS})
			SET(BU_CHMOD_BUNDLE_ITEMS ON)	# Make copies of system libraries writable before install_name_tool tries to change them.
			INCLUDE(BundleUtilities)
			FIXUP_BUNDLE(\"${APPS}\" \"\${BUNDLE_LIBS}\" \"${DIRS}\")
			" COMPONENT Runtime)
ENDMACRO()
