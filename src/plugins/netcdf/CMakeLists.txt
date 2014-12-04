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

# Locate the NetCDF library.
FIND_PACKAGE(netcdf REQUIRED)

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
	NetCDFPlugin
	SOURCES ${OVITO_NETCDF_SOURCE_FILES}
	PLUGIN_DEPENDENCIES Particles
	OPTIONAL_PLUGIN_DEPENDENCIES PyScript	
	PYTHON_WRAPPERS "${CMAKE_CURRENT_SOURCE_DIR}/resources/python/"
	LIB_DEPENDENCIES netcdf
)

TARGET_INCLUDE_DIRECTORIES(NetCDFPlugin PRIVATE "${netcdf_INCLUDE_DIRS}")

IF(WIN32)
	# Install NetCDF dll.
	GET_TARGET_PROPERTY(NETCDF_DLL_LOCATION netcdf IMPORTED_LOCATION_RELEASE)
	OVITO_INSTALL_DLL("${NETCDF_DLL_LOCATION}")

	# Install HDF5 dlls, which are required by NetCDF library.
	GET_TARGET_PROPERTY(NETCDF_DEPENDENCIES netcdf IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE)
	FOREACH(dep ${NETCDF_DEPENDENCIES})
		GET_FILENAME_COMPONENT(dep_base_name "${dep}" NAME_WE)
		GET_FILENAME_COMPONENT(dep_lib_path "${dep}" PATH)
		OVITO_INSTALL_DLL("${dep_lib_path}/../bin/${dep_base_name}.dll")
	ENDFOREACH()
ENDIF()
