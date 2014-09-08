###############################################################################
# 
#  Copyright (2014) Alexander Stukowski
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

# Find Voro++ library.
FIND_PACKAGE(VoroPlusPlus)

IF(VOROPLUSPLUS_FOUND)

	# Create imported target for the existing Voro++ library.
	ADD_LIBRARY(VoroPlusPlus SHARED IMPORTED GLOBAL)
	SET_PROPERTY(TARGET VoroPlusPlus PROPERTY IMPORTED_LOCATION "${VOROPLUSPLUS_LIBRARIES}")
	SET_PROPERTY(TARGET VoroPlusPlus APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${VOROPLUSPLUS_INCLUDE_DIRS}")

ELSE()

	# Build our own version of the library.
	SET(VOROPLUSPLUS_SOURCE_FILES
		src/cell.cc 
		src/common.cc 
		src/container.cc
		src/unitcell.cc
		src/v_compute.cc
		src/c_loops.cc
		src/v_base.cc
		src/wall.cc
		src/pre_container.cc
		src/container_prd.cc
	)

	# Build library.
	ADD_LIBRARY(VoroPlusPlus ${VOROPLUSPLUS_SOURCE_FILES})

	# Make header files of this library available to dependent targets.
	TARGET_INCLUDE_DIRECTORIES(VoroPlusPlus INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

	# This library is part of the installation package.
	IF(NOT OVITO_MONOLITHIC_BUILD)
		INSTALL(TARGETS VoroPlusPlus 
			RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
			LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
	ENDIF()

ENDIF()
