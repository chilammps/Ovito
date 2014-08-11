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

# Locate the HDF5 library.
IF(OVITO_MONOLITHIC_BUILD)
	SET(HDF5_USE_STATIC_LIBRARIES TRUE)
ENDIF()
FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)

IF(WIN32)
	# Locate HDF5 DLLs
	LIST(GET HDF5_LIBRARIES 0 HDF5_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(HDF5_LIBRARY_PATH "${HDF5_FIRST_LIBRARY}" PATH)
	SET(HDF5_DLL_PATH "${HDF5_LIBRARY_PATH}/../bin" CACHE PATH "Directory where the HDF5 dlls are located.")
	MARK_AS_ADVANCED(HDF5_DLL_PATH)

	# Locate NetCDF DLL
	LIST(GET NETCDF_LIBRARIES 0 NETCDF_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(NETCDF_LIBRARY_PATH "${NETCDF_FIRST_LIBRARY}" PATH)
	SET(NETCDF_DLL_PATH "${NETCDF_LIBRARY_PATH}/../bin" CACHE PATH "Directory where the NetCDF dll is located.")
	MARK_AS_ADVANCED(NETCDF_DLL_PATH)
ENDIF()

# Also find the auxiliary SZIP library that comes with the HDF5 distribution.
#IF(WIN32)
#	FIND_LIBRARY(HDF5_SZIP_LIBRARY NAMES szip HINTS ${HDF5_LIBRARY_DIRS} ENV HDF5_ROOT PATH_SUFFIXES lib Lib)
#ENDIF(WIN32)

SET(OVITO_NETCDF_SOURCE_FILES
	NetCDFImporter.cpp
)

IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	LIST(APPEND OVITO_NETCDF_SOURCE_FILES 
		PythonInterface.cpp
	)
ENDIF()

# Generate plug-in module.
OVITO_PLUGIN(
	NetCDFImporter
	SOURCES ${OVITO_NETCDF_SOURCE_FILES}
	PLUGIN_DEPENDENCIES Particles
	LIB_DEPENDENCIES ${NETCDF_LIBRARIES} ${HDF5_LIBRARIES} ${HDF5_SZIP_LIBRARY}
	OPTIONAL_PLUGIN_DEPENDENCIES PyScript	
)

TARGET_INCLUDE_DIRECTORIES(NetCDFImporter PRIVATE "${NETCDF_INCLUDE_DIRS}")

IF(WIN32)
	# Install NetCDF and HDF5 dlls.
	OVITO_INSTALL_DLL("${NETCDF_DLL_PATH}/libnetcdf.dll")
	OVITO_INSTALL_DLL("${HDF5_DLL_PATH}/hdf5.dll")
	OVITO_INSTALL_DLL("${HDF5_DLL_PATH}/hdf5_hl.dll")
ENDIF()

# Install the Python source files that belong to the plugin and which provide the scripting interface.
ADD_CUSTOM_COMMAND(TARGET Particles POST_BUILD COMMAND ${CMAKE_COMMAND} "-E" copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/python/" "${OVITO_PLUGINS_DIRECTORY}/python/")
