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

# Let the user choose whether to use the system library or to build 
# our own library from the included third-party source code.
OPTION(OVITO_USE_SYSTEM_QCUSTOMPLOT "Use the QCustomPlot library from the operating system." "OFF")

IF(OVITO_USE_SYSTEM_QCUSTOMPLOT)

	# Find system QCustomPlot library.
	FIND_PACKAGE(QCustomPlot REQUIRED)
	IF(NOT QCUSTOMPLOT_FOUND)
		MESSAGE(FATAL_ERROR "QCustomPlot library was not found.")
	ENDIF()

	# Create imported target for the existing qcustomplot library.
	ADD_LIBRARY(qcustomplot SHARED IMPORTED GLOBAL)
	SET_PROPERTY(TARGET qcustomplot PROPERTY IMPORTED_LOCATION "${QCUSTOMPLOT_LIBRARIES}")
	SET_PROPERTY(TARGET qcustomplot APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${QCUSTOMPLOT_INCLUDE_DIRS}")

ELSE()

	# Build our own version of the library.
	ADD_LIBRARY(qcustomplot qcustomplot.cpp)

	# Link Qt5.
	QT5_USE_MODULES(qcustomplot ${OVITO_REQUIRED_QT_MODULES})

	# Make header files of this library available to dependent targets.
	TARGET_INCLUDE_DIRECTORIES(qcustomplot INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

	# This library is part of the installation package.
	IF(NOT OVITO_MONOLITHIC_BUILD)
		INSTALL(TARGETS qcustomplot 
			RUNTIME DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}"
			LIBRARY DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
	ENDIF()

ENDIF()
