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

# The source files of the muParser library.
SET(MUPARSER_SOURCE_FILES
	muParser.cpp
	muParserBytecode.cpp
	muParserTest.cpp
	muParserDLL.cpp
	muParserInt.cpp
	muParserTokenReader.cpp
	muParserError.cpp
	muParserCallback.cpp
	muParserBase.cpp
)

# Build library
ADD_LIBRARY(muParser ${MUPARSER_SOURCE_FILES})

# This library is part of the installation package.
IF(NOT OVITO_MONOLITHIC_BUILD)
	INSTALL(TARGETS muParser 
		RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
		LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ENDIF()
