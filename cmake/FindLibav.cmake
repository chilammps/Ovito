# - Finds Libav libraries and headers
#
#  Will look for header files in LIBAV_INCLUDE_DIR if defined.
#  Will look for library files in LIBAV_LIBRARY_DIR if defined.
#
# Once done this will define
#  
#  LIBAV_FOUND			- system has Libav
#  Libav_INCLUDE_DIRS	- the include directories
#  Libav_LIBRARY_DIR	- the directory containing the libraries
#  Libav_LIBRARIES		- link these to use Libav
#

# List of required headers.
SET(LIBAV_HEADER_NAMES libavformat/avformat.h libavcodec/avcodec.h libavutil/avutil.h libswscale/swscale.h)
# List of required libraries.
SET(LIBAV_LIBRARY_NAMES avformat avcodec avutil avfilter swscale)

# avresample.dll is an indirect dependency that needs to be deployed on Windows.
IF(WIN32)
	LIST(APPEND LIBAV_LIBRARY_NAMES avresample)
ENDIF()

# Detect header path.
FIND_PATH(LIBAV_INCLUDE_DIR NAMES libavcodec/avcodec.h)
SET(Libav_INCLUDE_DIRS "${LIBAV_INCLUDE_DIR}")

# Detect library path.
IF(NOT LIBAV_LIBRARY_DIR)
	FIND_LIBRARY(LIBAV_AVCODEC_LIBRARY NAMES avcodec)
	IF(LIBAV_AVCODEC_LIBRARY)
		GET_FILENAME_COMPONENT(LIBAV_LIBRARY_DIR "${LIBAV_AVCODEC_LIBRARY}" PATH)
		SET(LIBAV_LIBRARY_DIR ${LIBAV_LIBRARY_DIR} CACHE PATH "Location of Libav libraries.")
	ENDIF()
ENDIF()

# Check if all required headers exist.
FOREACH(header ${LIBAV_HEADER_NAMES})
	UNSET(header_path CACHE)
	FIND_PATH(header_path ${header} PATHS ${Libav_INCLUDE_DIRS} NO_DEFAULT_PATH)
	IF(NOT header_path)
	    IF(NOT Libav_FIND_QUIETLY)
		    MESSAGE("Could not find Libav header file '${header}' in search path(s) '${LIBAV_INCLUDE_DIR}'.")
        ENDIF() 
        UNSET(LIBAV_INCLUDE_DIR)
	ENDIF()
	UNSET(header_path CACHE)
ENDFOREACH()

# Find the full paths of the libraries
UNSET(Libav_LIBRARIES)
FOREACH(lib ${LIBAV_LIBRARY_NAMES})
	UNSET(lib_path CACHE)
	FIND_LIBRARY(lib_path ${lib} PATHS "${LIBAV_LIBRARY_DIR}" NO_DEFAULT_PATH)
	IF(lib_path)
		LIST(APPEND Libav_LIBRARIES "${lib_path}")
	ELSE()
	    IF(NOT Libav_FIND_QUIETLY)
		    MESSAGE("Could not find Libav library '${lib}' in search path(s) '${LIBAV_LIBRARY_DIR}'.")
		ENDIF() 
		UNSET(LIBAV_LIBRARY_DIR)
	ENDIF()
	UNSET(lib_path CACHE)
ENDFOREACH()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libav DEFAULT_MSG LIBAV_LIBRARY_DIR LIBAV_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBAV_INCLUDE_DIR LIBAV_LIBRARY_DIR)

IF(APPLE)
	# libbz2 is an indirect dependency that needs to be linked in on MacOS.
	FIND_PACKAGE(BZip2 REQUIRED)
	
	# Apple's system libraries are required too.
	FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
	FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
	FIND_LIBRARY(VIDEODECODEACCELERATION_LIBRARY VideoDecodeAcceleration)

	LIST(APPEND Libav_LIBRARIES ${BZIP2_LIBRARIES} ${COREFOUNDATION_LIBRARY} ${COREVIDEO_LIBRARY} ${VIDEODECODEACCELERATION_LIBRARY})
	LIST(APPEND Libav_INCLUDE_DIRS ${BZIP2_INCLUDE_DIR})
ENDIF()

