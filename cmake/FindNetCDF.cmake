# - Find NetCDF
# Find the native NetCDF includes and library.
# Once done this will define
#
#  netcdf_INCLUDE_DIRS   - where to find netcdf.h.
#  netcdf                - The imported library target.

# First look for netcdf-config.cmake
FIND_PACKAGE(netcdf QUIET NO_MODULE)

# If that doesn't work, use our method to find the library
FIND_PATH(NETCDF_INCLUDE_DIR netcdf.h)

IF(NOT OVITO_MONOLITHIC_BUILD)
	FIND_LIBRARY(NETCDF_LIBRARY NAMES netcdf)
ELSE()
	FIND_LIBRARY(NETCDF_LIBRARY NAMES libnetcdf.a)
ENDIF()
MARK_AS_ADVANCED(NETCDF_LIBRARY NETCDF_INCLUDE_DIR)

IF(NOT NETCDF_INCLUDE_DIR OR NOT NETCDF_LIBRARY)
	MESSAGE(FATAL_ERROR "Could not locate the NetCDF library. Please set NETCDF_INCLUDE_DIR and NETCDF_LIBRARY to sepcify its location.")
ENDIF()

# Create imported target for the library.
ADD_LIBRARY(netcdf SHARED IMPORTED GLOBAL)
SET_PROPERTY(TARGET netcdf PROPERTY IMPORTED_LOCATION "${NETCDF_LIBRARY}")
SET_PROPERTY(TARGET netcdf APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${NETCDF_INCLUDE_DIR}")
