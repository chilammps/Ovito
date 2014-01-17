###############################################################################
# 
#  Copyright (2012) Alexander Stukowski
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

# Locate the NetCDF library and headers.
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
FIND_PACKAGE(NetCDF REQUIRED)
IF(NOT NETCDF_FOUND)
	MESSAGE(FATAL_ERROR "NetCDF library not found on your system.")
ENDIF(NOT NETCDF_FOUND)
INCLUDE_DIRECTORIES(${NETCDF_INCLUDE_DIRS})

# Locate the HDF5 library.
FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)
IF(NOT HDF5_FOUND)
	MESSAGE(FATAL_ERROR "HDF5 library not found on your system.")
ENDIF(NOT HDF5_FOUND)
# Also find the auxiliary SZIP library that comes with the HDF5 distribution.
#IF(WIN32)
#	FIND_LIBRARY(HDF5_SZIP_LIBRARY NAMES szip HINTS ${HDF5_LIBRARY_DIRS} ENV HDF5_ROOT PATH_SUFFIXES lib Lib)
#ENDIF(WIN32)

SET(OVITO_NETCDF_SOURCE_FILES
	NetCDFImporter.cpp
)

# Generate plug-in module.
OVITO_PLUGIN(
	NetCDFImporter
	SOURCES ${OVITO_NETCDF_SOURCE_FILES}
	PLUGIN_DEPENDENCIES Particles
	LIB_DEPENDENCIES ${NETCDF_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_SZIP_LIBRARY}
)
