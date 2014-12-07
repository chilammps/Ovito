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
FIND_PACKAGE(NetCDF REQUIRED)

SET(SourceFiles
	NetCDFImporter.cpp
)

IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	LIST(APPEND SourceFiles PythonInterface.cpp)
ENDIF()

OVITO_STANDARD_PLUGIN(
	NetCDFPlugin
	SOURCES ${SourceFiles}
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

ELSEIF(UNIX AND NOT APPLE AND OVITO_MONOLITHIC_BUILD)

	# Deploy NetCDF shared library.
	GET_TARGET_PROPERTY(NETCDF_LIBRARY netcdf LOCATION)
	FILE(GLOB lib_versions "${NETCDF_LIBRARY}*")
	FOREACH(lib_version ${lib_versions})
		INSTALL(FILES "${lib_version}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
	ENDFOREACH()

ENDIF()
