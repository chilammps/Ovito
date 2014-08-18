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
# by transforming the  docbook files to HTML.

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
					COMMENT "Building documentation files")
	
	INSTALL(DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual/html/" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/doc/manual/html/")
	ADD_DEPENDENCIES(${PROJECT_NAME} documentation)
	
	# Generate documentation for OVITO's scripting interface.
	IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	
		# Find Sphinx program.
		FIND_PROGRAM(SPINX_PROCESSOR "sphinx-build" DOC "Path to the Sphinx build program used to generate the Python interface documentation.")
		IF(NOT SPINX_PROCESSOR)
			MESSAGE(FATAL_ERROR "The Sphinx program (sphinx-build) was not found. Please install it and/or specify its location manually.")
		ENDIF()
		
		# Let OVITO's built in Python interpreter execute the Sphinx program.
		# We cannot use the standard Python interpreter, because it cannot load OVITO's scripting modules, which is required to auto-generate the
		# interface documentation from the docstrings.
		GET_PROPERTY(OVITO_MAIN_EXECUTABLE TARGET ${PROJECT_NAME} PROPERTY LOCATION)
		ADD_CUSTOM_TARGET(scripting_documentation ALL 
					COMMAND "${OVITO_MAIN_EXECUTABLE}" "--nogui" "--script" ${SPINX_PROCESSOR} "--scriptarg" "-b" "--scriptarg" "html" 
					"--scriptarg" "-D" "--scriptarg" "version=${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}" 
					"--scriptarg" "-D" "--scriptarg" "release=${OVITO_VERSION_MAJOR}.${OVITO_VERSION_MINOR}.${OVITO_VERSION_REVISION}"
					"--scriptarg" "." "--scriptarg" "${OVITO_SHARE_DIRECTORY}/doc/python/" 
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc/python/"
					COMMENT "Building scripting documentation files")

		INSTALL(DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/python/" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/doc/python/")
		
		# Run Sphinx only after OVITO and all plugins have been built.
		ADD_DEPENDENCIES(scripting_documentation ${PROJECT_NAME} ${OVITO_PLUGINS_LIST})
		# Build the scripting documentation every time the main documentation target is built.
		ADD_DEPENDENCIES(scripting_documentation documentation)
	ENDIF()

ENDIF(OVITO_BUILD_DOCUMENTATION)

# Controls the generation of the API docs.
OPTION(OVITO_BUILD_API_DOCS "Generate developer documentation from source code comments (requires Doxygen)" "OFF")

IF(OVITO_BUILD_API_DOCS)

	# Find the Doxygen program.
	FIND_PACKAGE(Doxygen REQUIRED)
	
	# Generate API documentation files.
	ADD_CUSTOM_TARGET(apidocs ALL
					COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
					WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc/develop/"
					COMMENT "Building API documentation files")
	
ENDIF(OVITO_BUILD_API_DOCS)
