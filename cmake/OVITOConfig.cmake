INCLUDE("${CMAKE_CURRENT_LIST_DIR}/OVITOTargets.cmake")

INCLUDE(CMakeParseArguments)

# Find Qt5 libraries.
FOREACH(component IN ITEMS Core Gui Widgets OpenGL Concurrent Network PrintSupport)
	FIND_PACKAGE(Qt5${component} REQUIRED)
ENDFOREACH()

# Tell CMake to run Qt moc whenever necessary.
SET(CMAKE_AUTOMOC ON)

FUNCTION(OVITO_PLUGIN plugin_name)
    SET(options)
    SET(oneValueArgs)
    SET(multiValueArgs SOURCES PLUGIN_DEPENDENCIES)
    CMAKE_PARSE_ARGUMENTS(OVITO_PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# Create the library target for the plugin.
	ADD_LIBRARY(${plugin_name} SHARED ${OVITO_PLUGIN_SOURCES})

	# Link to OVITO's core library.
	TARGET_LINK_LIBRARIES(${plugin_name} PUBLIC Ovito::Core)

	# Link Qt5.
	TARGET_LINK_LIBRARIES(${plugin_name} PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Concurrent)

	# Link to plugin dependencies.
	FOREACH(name ${OVITO_PLUGIN_PLUGIN_DEPENDENCIES})
    	TARGET_LINK_LIBRARIES(${plugin_name} PUBLIC Ovito::${name})
	ENDFOREACH()

    # Enable C++11 language standard.
    SET_TARGET_PROPERTIES(${plugin_name} PROPERTIES COMPILE_OPTIONS "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-std=c++11>")

	# Set prefix and suffix of library name.
	# This is needed so that the Python interpreter can load OVITO plugins as modules.
	SET_TARGET_PROPERTIES(${plugin_name} PROPERTIES PREFIX "" SUFFIX "@OVITO_PLUGIN_LIBRARY_SUFFIX@")

    # Place compiled plugin module in the right directory.
    SET_TARGET_PROPERTIES(${plugin_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "@OVITO_PLUGINS_DIRECTORY@")

    # Set rpath.
    SET_TARGET_PROPERTIES(${plugin_name} PROPERTIES INSTALL_RPATH "@executable_path/plugins/;@executable_path/")
    SET_TARGET_PROPERTIES(${plugin_name} PROPERTIES MACOSX_RPATH TRUE)

    # The build tree target should have rpath of install tree target.
    SET_TARGET_PROPERTIES(${plugin_name} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)

    # Auto-generate plugin manifest.
	SET(PLUGIN_MANIFEST "@OVITO_PLUGINS_DIRECTORY@/${plugin_name}.json")
	FILE(WRITE "${PLUGIN_MANIFEST}" "{\n  \"plugin-id\" : \"${plugin_name}\",\n")
	FILE(APPEND "${PLUGIN_MANIFEST}" "  \"dependencies\" : [ ")
	FOREACH(name ${OVITO_PLUGIN_PLUGIN_DEPENDENCIES})
		FILE(APPEND "${PLUGIN_MANIFEST}" "${delimiter}\"${name}\"")
		SET(delimiter ", ")
	ENDFOREACH()
	FILE(APPEND "${PLUGIN_MANIFEST}" " ],\n")
	FILE(APPEND "${PLUGIN_MANIFEST}" "  \"native-library\" : \"${plugin_name}\"\n}\n")
		
ENDFUNCTION()