###############################################################################
# 
#  Copyright (2013) Alexander Stukowski
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

# Find system Botan library.
FIND_PACKAGE(Botan)	

IF(BOTAN_FOUND)

	# Create exported target for the Botan library.
	ADD_LIBRARY(Botan SHARED IMPORTED GLOBAL)
	SET_PROPERTY(TARGET Botan PROPERTY IMPORTED_LOCATION "${BOTAN_LIBRARIES}")
	SET_PROPERTY(TARGET Botan APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${BOTAN_INCLUDE_DIRS}")

ELSE()

	# Build our own version of the Botan library.

	# Build library
	ADD_LIBRARY(Botan botan/botan.cpp)

	IF(APPLE)
		TARGET_COMPILE_DEFINITIONS(Botan PRIVATE
			BOTAN_TARGET_OS_IS_DARWIN 
			BOTAN_BUILD_COMPILER_IS_CLANG 
			BOTAN_TARGET_ARCH_IS_X86_64 
			BOTAN_HAS_MUTEX_PTHREAD
			BOTAN_TARGET_OS_IS_LINUX 
			BOTAN_TARGET_OS_HAS_DLOPEN 
			BOTAN_TARGET_OS_HAS_GMTIME_R 
			BOTAN_TARGET_OS_HAS_POSIX_MLOCK
			BOTAN_HAS_DYNAMICALLY_LOADED_ENGINE 
			BOTAN_HAS_DYNAMIC_LOADER
			BOTAN_TARGET_OS_HAS_GETTIMEOFDAY 
			BOTAN_HAS_ALLOC_MMAP
			BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM 
			BOTAN_HAS_ENTROPY_SRC_EGD 
			BOTAN_HAS_ENTROPY_SRC_FTW
			BOTAN_HAS_ENTROPY_SRC_UNIX 
			BOTAN_HAS_MUTEX_PTHREAD 
			BOTAN_HAS_PIPE_UNIXFD_IO
		)
	ELSEIF(UNIX)
		TARGET_COMPILE_DEFINITIONS(Botan PRIVATE
			BOTAN_TARGET_OS_HAS_GETTIMEOFDAY 
			BOTAN_HAS_ALLOC_MMAP
	    		BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM 
			BOTAN_HAS_ENTROPY_SRC_EGD 
			BOTAN_HAS_ENTROPY_SRC_FTW
			BOTAN_HAS_ENTROPY_SRC_UNIX 
			BOTAN_HAS_MUTEX_PTHREAD 
			BOTAN_HAS_PIPE_UNIXFD_IO
		)
	ELSEIF(WIN32)
		TARGET_COMPILE_DEFINITIONS(Botan PRIVATE
			BOTAN_TARGET_OS_IS_WINDOWS
			BOTAN_TARGET_OS_HAS_LOADLIBRARY 
			BOTAN_TARGET_OS_HAS_WIN32_GET_SYSTEMTIME
			BOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK 
			BOTAN_HAS_DYNAMICALLY_LOADED_ENGINE
			BOTAN_HAS_DYNAMIC_LOADER 
			BOTAN_HAS_ENTROPY_SRC_CAPI 
			BOTAN_HAS_ENTROPY_SRC_WIN32
			BOTAN_HAS_MUTEX_WIN32
			BOTAN_DLL=Q_DECL_EXPORT
		)
		IF(MSVC)
			TARGET_COMPILE_DEFINITIONS(Botan PRIVATE BOTAN_BUILD_COMPILER_IS_MSVC BOTAN_TARGET_OS_HAS_GMTIME_S)
			TARGET_COMPILE_OPTIONS(Botan PRIVATE -wd4251 -wd4290 -wd4250)
		ENDIF()
	ELSE()
		MESSAGE(FATAL_ERROR "Building on this platform is not supported.")
	ENDIF()

	IF(CMAKE_COMPILER_IS_GNUCXX)
		TARGET_COMPILE_DEFINITIONS(Botan PRIVATE BOTAN_BUILD_COMPILER_IS_GCC)
	ENDIF()

	# Link Qt5.
	QT5_USE_MODULES(Botan ${OVITO_REQUIRED_QT_MODULES})

	# Make header files of this library available to dependent targets.
	TARGET_INCLUDE_DIRECTORIES(Botan INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

	# This library is part of the installation package.
	IF(NOT OVITO_MONOLITHIC_BUILD)
		INSTALL(TARGETS Botan
			RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
			LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
	ENDIF()

ENDIF()
