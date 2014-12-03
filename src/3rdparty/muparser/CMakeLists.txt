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

# Find muParser library.
FIND_PACKAGE(MuParser)

IF(MUPARSER_FOUND)

	# Create imported target for the existing muParser library.
	ADD_LIBRARY(muParser SHARED IMPORTED GLOBAL)
	SET_PROPERTY(TARGET muParser PROPERTY IMPORTED_LOCATION "${MUPARSER_LIBRARIES}")
	SET_PROPERTY(TARGET muParser APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${MUPARSER_INCLUDE_DIRS}")

ELSE()

	# Build our own version of the library.
	SET(MUPARSER_SOURCE_FILES
		muParser.cpp
		muParserBytecode.cpp
		muParserInt.cpp
		muParserTokenReader.cpp
		muParserError.cpp
		muParserCallback.cpp
		muParserBase.cpp
	)

	# Build library.
	ADD_LIBRARY(muParser STATIC EXCLUDE_FROM_ALL ${MUPARSER_SOURCE_FILES})

	# Make header files of this library available to dependent targets.
	TARGET_INCLUDE_DIRECTORIES(muParser INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

ENDIF()
