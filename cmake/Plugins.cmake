
# Create an official OVITO plugin.
FUNCTION(OVITO_STANDARD_PLUGIN target_name)

    # Parse macro parameters
    SET(options)
    SET(oneValueArgs)
    SET(multiValueArgs SOURCES LIB_DEPENDENCIES PLUGIN_DEPENDENCIES OPTIONAL_PLUGIN_DEPENDENCIES PYTHON_WRAPPERS)
    CMAKE_PARSE_ARGUMENTS(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	SET(plugin_sources ${ARG_SOURCES})
	SET(lib_dependencies ${ARG_LIB_DEPENDENCIES})
	SET(plugin_dependencies ${ARG_PLUGIN_DEPENDENCIES})
	SET(optional_plugin_dependencies ${ARG_OPTIONAL_PLUGIN_DEPENDENCIES})
	SET(python_wrappers ${ARG_PYTHON_WRAPPERS})

	# Create the library target for the plugin.
    ADD_LIBRARY(${target_name} ${OVITO_DEFAULT_LIBRARY_TYPE} ${plugin_sources})

    # Set default include directory.
    TARGET_INCLUDE_DIRECTORIES(${target_name} PUBLIC 
        "$<BUILD_INTERFACE:${OVITO_SOURCE_BASE_DIR}/src>")

	# Link to OVITO's core library.
	TARGET_LINK_LIBRARIES(${target_name} PUBLIC Core)

	# Link other required libraries.
	TARGET_LINK_LIBRARIES(${target_name} PUBLIC ${lib_dependencies})

	# Link Qt5.
	TARGET_LINK_LIBRARIES(${target_name} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Concurrent)

	# Link plugin dependencies.
	FOREACH(plugin_name ${plugin_dependencies})
    	STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
    	IF(NOT OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
    		MESSAGE(FATAL_ERROR "To build the ${target_name} plugin, the ${plugin_name} plugin has to be enabled too. Please set the OVITO_BUILD_PLUGIN_${uppercase_plugin_name} option to ON.")
    	ENDIF()
    	TARGET_LINK_LIBRARIES(${target_name} PUBLIC ${plugin_name})
	ENDFOREACH()

	# Link optional plugin dependencies.
	FOREACH(plugin_name ${optional_plugin_dependencies})
		STRING(TOUPPER "${plugin_name}" uppercase_plugin_name)
		IF(OVITO_BUILD_PLUGIN_${uppercase_plugin_name})
        	TARGET_LINK_LIBRARIES(${target_name} PUBLIC ${plugin_name})
		ENDIF()
	ENDFOREACH()
	
	IF(NOT OVITO_MONOLITHIC_BUILD)
		# Set prefix and suffix of library name.
		# This is needed so that the Python interpreter can load OVITO plugins as modules.
		SET_TARGET_PROPERTIES(${target_name} PROPERTIES PREFIX "" SUFFIX "${OVITO_PLUGIN_LIBRARY_SUFFIX}")
	ENDIF()

	IF(APPLE)
		# This is required to avoid error by install_name_tool.
		SET_TARGET_PROPERTIES(${target_name} PROPERTIES LINK_FLAGS "-headerpad_max_install_names")
	ENDIF(APPLE)

    # Enable the use of @rpath on OSX.
    SET_TARGET_PROPERTIES(${target_name} PROPERTIES MACOSX_RPATH TRUE)
	
    # Place compiled plugin module in the right directory.
    SET_TARGET_PROPERTIES(${target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${OVITO_PLUGINS_DIRECTORY}")
	
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
	FILE(APPEND "${PLUGIN_MANIFEST}" "  \"native-library\" : \"${target_name}\"\n}\n")
	INSTALL(FILES "${PLUGIN_MANIFEST}" DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")

	# Install Python wrapper files.
	IF(python_wrappers)
		# Install the Python source files that belong to the plugin, which provide the scripting interface.
		ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD COMMAND ${CMAKE_COMMAND} "-E" copy_directory "${python_wrappers}" "${OVITO_PLUGINS_DIRECTORY}/python/")
	ENDIF()

	# This plugin will be part of the installation package.
	IF(NOT OVITO_MONOLITHIC_BUILD)
		INSTALL(TARGETS ${target_name} EXPORT OVITO
			RUNTIME DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}"
			LIBRARY DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}")
	ENDIF(NOT OVITO_MONOLITHIC_BUILD)

    # Build this plugin when building OVITO's main executable
	ADD_DEPENDENCIES(${PROJECT_NAME} ${target_name})

ENDFUNCTION()

