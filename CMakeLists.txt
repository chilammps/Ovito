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

PROJECT(Ovito)

# Make sure we have a recent version of CMake.
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8 FATAL_ERROR)

# Include program revision information.
GET_FILENAME_COMPONENT(OVITO_SOURCE_BASE_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
INCLUDE(${OVITO_SOURCE_BASE_DIR}/cmake/Version.cmake)

# Tell CMake to run Qt moc when necessary:
SET(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
  
# Find and setup Qt5 for this project
FIND_PACKAGE(Qt5Widgets REQUIRED)

# Use the Q_SIGNALS and Q_SLOTS macros to avoid name conflicts.
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

# Enable C++11.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

# This is the base directory for all included header files.
INCLUDE_DIRECTORIES("${ovito_SOURCE_DIR}/src")

IF(UNIX)
	IF(APPLE)
		SET(MACOSX_BUNDLE_NAME "Ovito") 
		# The directory where the main executable goes to.
		SET(OVITO_RELATIVE_BINARY_DIRECTORY ".")
		# The directory where the main libraries go to.
		SET(OVITO_RELATIVE_LIBRARY_DIRECTORY "${MACOSX_BUNDLE_NAME}.app/Contents/MacOS")
		# The directory where the auxiliary files go to.
		SET(OVITO_RELATIVE_SHARE_DIRECTORY "${MACOSX_BUNDLE_NAME}.app/Contents/Resources")
	ELSE(APPLE)
		# The directory where the main executable goes to.
		SET(OVITO_RELATIVE_BINARY_DIRECTORY "bin")
		# The directory where the main libraries go to.
		SET(OVITO_RELATIVE_LIBRARY_DIRECTORY "lib/ovito")
		# The directory where the auxiliary files go to.
		SET(OVITO_RELATIVE_SHARE_DIRECTORY "share/ovito")
	ENDIF(APPLE)
ELSE(UNIX)
	# The directory where the main executable goes to.
	SET(OVITO_RELATIVE_BINARY_DIRECTORY ".")
	# The directory where the main libraries go to.
	SET(OVITO_RELATIVE_LIBRARY_DIRECTORY ".")
	# The directory where the auxiliary files go to.
	SET(OVITO_RELATIVE_SHARE_DIRECTORY ".")
ENDIF(UNIX)

# The directory where the main executable goes to.
SET(OVITO_BINARY_DIRECTORY "${ovito_BINARY_DIR}/${OVITO_RELATIVE_BINARY_DIRECTORY}")
# The directory where the main libraries go to.
SET(OVITO_LIBRARY_DIRECTORY "${ovito_BINARY_DIR}/${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
# The directory where the auxiliary files go to.
SET(OVITO_SHARE_DIRECTORY "${ovito_BINARY_DIR}/${OVITO_RELATIVE_SHARE_DIRECTORY}")

# Build program.
ADD_SUBDIRECTORY(src)

