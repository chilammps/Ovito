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
FIND_PACKAGE(VoroPlusPlus QUIET)

IF(VOROPLUSPLUS_FOUND)

	# Create imported target for the existing Voro++ library.
	ADD_LIBRARY(VoroPlusPlus SHARED IMPORTED GLOBAL)
	SET_PROPERTY(TARGET VoroPlusPlus PROPERTY IMPORTED_LOCATION "${VoroPlusPlus_LIBRARIES}")
	SET_PROPERTY(TARGET VoroPlusPlus APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${VoroPlusPlus_INCLUDE_DIRS}")

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
	ADD_LIBRARY(VoroPlusPlus STATIC EXCLUDE_FROM_ALL ${VOROPLUSPLUS_SOURCE_FILES})

	# Make header files of this library available to dependent targets.
	TARGET_INCLUDE_DIRECTORIES(VoroPlusPlus INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")
	
	# Since we will link this into the Particles plugin, we need to use the same setting for the fPIC flag.
	IF(NOT OVITO_MONOLITHIC_BUILD)
	    SET_PROPERTY(TARGET VoroPlusPlus PROPERTY POSITION_INDEPENDENT_CODE ON)        
    ENDIF()

ENDIF()
