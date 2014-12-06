###############################################################################
# 
#  Copyright (2013) Alexander Stukowski
#
#  This file is part of OVITO (Open Visualization Tool).
#
#  OVITO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
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

# This CMake script compiles the user manual for OVITO
# by transforming the docbook files to HTML.

# It also generates the scripting interface documentation using
# Sphinx, and the C++ API documentation using Doxygen.

# Controls the generation of the user manual.
OPTION(OVITO_BUILD_DOCUMENTATION "Build the user manual" "OFF")

IF(OVITO_BUILD_DOCUMENTATION)

	# Find the XSLT processor program.
	FIND_PROGRAM(XSLT_PROCESSOR "xsltproc" DOC "Path to the XSLT processor program used to build the documentation.")
	IF(NOT XSLT_PROCESSOR)
		MESSAGE(FATAL_ERROR "The XSLT processor program (xsltproc) was not found. Please install it and/or specify its location manually.")
	ENDIF()
	SET(XSLT_PROCESSOR_OPTIONS "--xinclude" CACHE STRING "Additional to pass to the XSLT processor program when building the documentation")
	MARK_AS_ADVANCED(XSLT_PROCESSOR_OPTIONS)
	
	# Create destination directories.
	FILE(MAKE_DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual")
	FILE(MAKE_DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual/html")
	
	# XSL transform documentation files.
	ADD_CUSTOM_TARGET(documentation
					COMMAND ${CMAKE_COMMAND} "-E" copy_directory "images/" "${OVITO_SHARE_DIRECTORY}/doc/manual/html/images/"
					COMMAND ${CMAKE_COMMAND} "-E" copy "manual.css" "${OVITO_SHARE_DIRECTORY}/doc/manual/html/"
					COMMAND ${XSLT_PROCESSOR} "${XSLT_PROCESSOR_OPTIONS}" --stringparam base.dir "${OVITO_SHARE_DIRECTORY}/doc/manual/html/" html-customization-layer.xsl Manual.docbook
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc/manual/"
					COMMENT "Generating user documentation")
	
	INSTALL(DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual/html/" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/doc/manual/html/")
	ADD_DEPENDENCIES(${PROJECT_NAME} documentation)
	
	# Generate documentation for OVITO's scripting interface.
	IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	
		# Find Sphinx program.
		FIND_PROGRAM(SPHINX_PROCESSOR NAMES "sphinx-build-script.py" "sphinx-build" HINTS "${SPHINX_PROCESSOR_PATH}" DOC "Path to the Sphinx build program used to generate the Python interface documentation.")
		IF(NOT SPHINX_PROCESSOR)
			MESSAGE(FATAL_ERROR "The Sphinx program (sphinx-build) was not found. Please install it and/or specify its location manually using the SPHINX_PROCESSOR setting.")
		ENDIF()
		
		# Use OVITO's built in Python interpreter to run the Sphinx doc program.
		# We cannot use the standard Python interpreter for this, because it cannot load OVITO's scripting modules, which is required to auto-generate the
		# interface documentation from the docstrings.
		IF(WIN32)
			GET_PROPERTY(OVITOS_EXECUTABLE TARGET ovitos PROPERTY LOCATION)
		ELSE()
			GET_PROPERTY(OVITO_MAIN_EXECUTABLE TARGET ${PROJECT_NAME} PROPERTY LOCATION)
			GET_FILENAME_COMPONENT(OVITO_MAIN_EXECUTABLE_DIR "${OVITO_MAIN_EXECUTABLE}" PATH)
			SET(OVITOS_EXECUTABLE "${OVITO_MAIN_EXECUTABLE_DIR}/ovitos")
		ENDIF()
		ADD_CUSTOM_TARGET(scripting_documentation ALL 
					COMMAND "${OVITOS_EXECUTABLE}" ${SPHINX_PROCESSOR} "-b" "html" "-a" 
					"-D" "version=${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}" 
					"-D" "release=${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}.${OVITO_VERSION_REVISION}"
					"." "${OVITO_SHARE_DIRECTORY}/doc/manual/html/python/" 
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc/python/"
					COMMENT "Generating scripting documentation")
		
		# Run Sphinx only after OVITO and all its plugins have been built.
		IF(WIN32)
			ADD_DEPENDENCIES(scripting_documentation ovitos ${OVITO_PLUGINS_LIST})
		ELSE()
			ADD_DEPENDENCIES(scripting_documentation ${PROJECT_NAME} ${OVITO_PLUGINS_LIST})
		ENDIF()
		# Build the scripting documentation together with the main documentation.
		ADD_DEPENDENCIES(scripting_documentation documentation)
	ENDIF()

ENDIF(OVITO_BUILD_DOCUMENTATION)

# Controls the generation of the API docs.
OPTION(OVITO_BUILD_API_DOCS "Creates the 'apidocs' make target, which generates developer documentation from C++ source code comments (requires Doxygen)" "OFF")

IF(OVITO_BUILD_API_DOCS)

	# Find the Doxygen program.
	FIND_PACKAGE(Doxygen REQUIRED)
	
	# Generate API documentation files.
	ADD_CUSTOM_TARGET(apidocs
					COMMAND "env" "OVITO_VERSION_STRING=${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}.${OVITO_VERSION_REVISION}" 
					"OVITO_INCLUDE_PATH=${CMAKE_SOURCE_DIR}/src/" ${DOXYGEN_EXECUTABLE} Doxyfile
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc/develop/"
					COMMENT "Generating C++ API documentation")
	
ENDIF(OVITO_BUILD_API_DOCS)
