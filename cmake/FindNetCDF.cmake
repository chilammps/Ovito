# - Find NetCDF
# Find the native NetCDF includes and library.
# Once done this will define
#
#  netcdf_INCLUDE_DIRS   - where to find netcdf.h.
#  netcdf                - The imported library target.

# First look for netcdf-config.cmake
FIND_PACKAGE(netcdf QUIET CONFIG)

IF(NOT netcdf_FOUND)

	# If that doesn't work, use our method to find the library
	FIND_PATH(NETCDF_INCLUDE_DIR netcdf.h)

	FIND_LIBRARY(NETCDF_LIBRARY NAMES netcdf)
	MARK_AS_ADVANCED(NETCDF_LIBRARY NETCDF_INCLUDE_DIR)

	INCLUDE(FindPackageHandleStandardArgs)
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(NetCDF DEFAULT_MSG NETCDF_LIBRARY NETCDF_INCLUDE_DIR)

	# Create imported target for the library.
	ADD_LIBRARY(netcdf SHARED IMPORTED GLOBAL)
	SET_PROPERTY(TARGET netcdf PROPERTY IMPORTED_LOCATION "${NETCDF_LIBRARY}")
	SET_PROPERTY(TARGET netcdf APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${NETCDF_INCLUDE_DIR}")

ENDIF()