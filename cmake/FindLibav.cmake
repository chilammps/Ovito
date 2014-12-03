# - Finds Libav libraries and headers
#
#  Will look for header files in LIBAV_INCLUDE_DIR if defined.
#  Will look for library files in LIBAV_LIBRARY_DIR if defined.
#
# Once done this will define
#  
#  LIBAV_FOUND			- system has Libav
#  LIBAV_INCLUDE_DIRS	- the include directories
#  LIBAV_LIBRARY_DIR	- the directory containing the libraries
#  LIBAV_LIBRARIES		- link these to use Libav
#

# List of required headers.
SET(LIBAV_HEADER_NAMES libavformat/avformat.h libavcodec/avcodec.h libavutil/avutil.h libswscale/swscale.h)
# List of required libraries.
SET(LIBAV_LIBRARY_NAMES avformat avcodec avutil avresample swscale)

IF(WIN32)

	# Detect header path.
	FIND_PATH(LIBAV_INCLUDE_DIR NAMES libavcodec/avcodec.h)
	SET(LIBAV_INCLUDE_DIRS "${LIBAV_INCLUDE_DIR}")
	
	# Detect library path.
	IF(NOT LIBAV_LIBRARY_DIR)
		FIND_LIBRARY(LIBAV_AVCODEC_LIBRARY NAMES avcodec)
		IF(LIBAV_AVCODEC_LIBRARY)
			GET_FILENAME_COMPONENT(LIBAV_LIBRARY_DIR "${LIBAV_AVCODEC_LIBRARY}" PATH)
		ENDIF()
	ENDIF()

	MARK_AS_ADVANCED(LIBAV_INCLUDE_DIR LIBAV_LIBRARY_DIR)

ELSE()
	INCLUDE(FindPkgConfig)
	IF(PKG_CONFIG_FOUND)
		PKG_CHECK_MODULES(AVFORMAT libavformat)
		PKG_CHECK_MODULES(AVCODEC libavcodec)
		PKG_CHECK_MODULES(AVUTIL libavutil)
		PKG_CHECK_MODULES(AVDEVICE libavdevice)
		PKG_CHECK_MODULES(SWSCALE libswscale)
	ENDIF()

	SET(LIBAV_LIBRARY_DIR ${AVFORMAT_LIBRARY_DIRS}
			     ${AVCODEC_LIBRARY_DIRS}
			     ${AVUTIL_LIBRARY_DIRS}
			     ${AVDEVICE_LIBRARY_DIRS}
			     ${SWSCALE_LIBRARY_DIRS})
	SET(LIBAV_INCLUDE_PATHS ${AVFORMAT_INCLUDE_DIRS}
			     ${AVCODEC_INCLUDE_DIRS}
			     ${AVUTIL_INCLUDE_DIRS}
			     ${AVDEVICE_INCLUDE_DIRS}
			     ${SWSCALE_INCLUDE_DIRS})

	IF(NOT APPLE)
		IF(NOT OVITO_MONOLITHIC_BUILD OR NOT UNIX)
			SET(LIBAV_LIBRARIES avformat avcodec avutil avdevice swscale)
		ELSE()
			SET(LIBAV_LIBRARIES libavformat.a libavcodec.a libavutil.a libavdevice.a libswscale.a)
		ENDIF()
	ELSE()
		SET(LIBAV_LIBRARIES libavformat.a libavcodec.a libavutil.a libavdevice.a libswscale.a bz2)
	ENDIF()
ENDIF()

# Check if all required headers exist.
SET(LIBAV_FOUND TRUE)
FOREACH(header ${LIBAV_HEADER_NAMES})
	UNSET(header_path CACHE)
	FIND_PATH(header_path ${header} PATHS ${LIBAV_INCLUDE_DIRS} NO_DEFAULT_PATH)
	IF(NOT header_path)
		MESSAGE("Could not find Libav header file '${header}' in search path(s) '${LIBAV_INCLUDE_DIRS}'.") 
		SET(LIBAV_FOUND FALSE)
	ENDIF()
	UNSET(header_path CACHE)
ENDFOREACH()

# Find the full paths of the libraries
UNSET(LIBAV_LIBRARIES)
FOREACH(lib ${LIBAV_LIBRARY_NAMES})
	UNSET(lib_path CACHE)
	FIND_LIBRARY(lib_path ${lib} PATHS "${LIBAV_LIBRARY_DIR}" NO_DEFAULT_PATH)
	IF(lib_path)
		LIST(APPEND LIBAV_LIBRARIES "${lib_path}")
	ELSE()
		MESSAGE("Could not find Libav library '${lib}' in search path(s) '${LIBAV_LIBRARY_DIR}'.") 
		SET(LIBAV_FOUND FALSE)
	ENDIF()
	UNSET(lib_path CACHE)
ENDFOREACH()

IF(APPLE)
	FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
	FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
	FIND_LIBRARY(VIDEODECODEACCELERATION_LIBRARY VideoDecodeAcceleration)
	LIST(APPEND LIBAV_LIBRARIES ${COREFOUNDATION_LIBRARY} ${COREVIDEO_LIBRARY} ${VIDEODECODEACCELERATION_LIBRARY})
ENDIF()

