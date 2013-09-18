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

# This CMake script compiles the documentation files for OVITO
# by transforming the source XML docbook files to a HTML-based format.

#
# Find the XSLT processor program.
#
FIND_PROGRAM(XSLT_PROCESSOR "xsltproc" DOC "Path to the XSLT processor program used to build the documentation.")
IF(NOT XSLT_PROCESSOR)	
	MESSAGE(FATAL_ERROR "The XSLT processor program (xsltproc) was not found. Please install it and/or specify its location manually.")
ENDIF(NOT XSLT_PROCESSOR)
SET(XSLT_PROCESSOR_OPTIONS "--xinclude" CACHE STRING "Additional to pass to the XSLT processor program when building the documentation" )
MARK_AS_ADVANCED(XSLT_PROCESSOR_OPTIONS)

#
# Find the Qt help generator program.
#
FIND_PROGRAM(QT_HELP_COLLECTION_GENERATOR 
	NAMES "qcollectiongenerator"
	HINTS "${_qt5Core_install_prefix}/bin"
	DOC "Path to the Qt Help Collection Generator program used to build the documentation."
)
IF(NOT QT_HELP_COLLECTION_GENERATOR)
	MESSAGE(FATAL_ERROR "The Qt Help Collection Generator program (qcollectiongenerator) was not found. Please install it and/or specify its location manually.")
ENDIF(NOT QT_HELP_COLLECTION_GENERATOR)

# Create destination directories.
FILE(MAKE_DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual")
FILE(MAKE_DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual/assistant")
FILE(MAKE_DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual/html")

#
# XSL transform documentation files.
#
# We create the manual in two different format: 
# Standard HTML format for the manual on the webserver and
# the special Qt Help assistant format for online help in the application.
#
ADD_CUSTOM_TARGET(documentation 
				COMMAND ${CMAKE_COMMAND} "-E" copy_directory "images/" "${OVITO_SHARE_DIRECTORY}/doc/manual/html/images/"
				COMMAND ${CMAKE_COMMAND} "-E" copy "manual.css" "${OVITO_SHARE_DIRECTORY}/doc/manual/html/"
				COMMAND ${XSLT_PROCESSOR} ${XSLT_PROCESSOR_OPTIONS} --stringparam base.dir "${OVITO_SHARE_DIRECTORY}/doc/manual/html/" html-customization-layer.xsl Manual.docbook
				COMMAND ${CMAKE_COMMAND} "-E" copy_directory "images/" "${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/images/"
				COMMAND ${XSLT_PROCESSOR} ${XSLT_PROCESSOR_OPTIONS} --stringparam base.dir "${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/" assistant-customization-layer.xsl Manual.docbook
				COMMAND ${CMAKE_COMMAND} "-E" copy "documentation.qhcp" "${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/documentation.qhcp"
				COMMAND ${QT_HELP_COLLECTION_GENERATOR} ${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/documentation.qhcp -o ${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/documentation.qhc
				WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc/manual/"
				COMMENT "Building documentation files")

INSTALL(FILES "${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/documentation.qhc" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/doc/manual/assistant/")
INSTALL(FILES "${OVITO_SHARE_DIRECTORY}/doc/manual/assistant/documentation.qch" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/doc/manual/assistant/")
INSTALL(DIRECTORY "${OVITO_SHARE_DIRECTORY}/doc/manual/html/" DESTINATION "${OVITO_RELATIVE_SHARE_DIRECTORY}/doc/manual/html/")
