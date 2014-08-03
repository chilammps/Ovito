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

FIND_PACKAGE(PythonLibs REQUIRED)

SET(OVITO_PYSCRIPT_SOURCE_FILES
	engine/ScriptEngine.cpp
	engine/ScriptAutostarter.cpp
	binding/PythonInterface.cpp
)

# Generate plugin module.
OVITO_PLUGIN(PyScript
	SOURCES ${OVITO_PYSCRIPT_SOURCE_FILES}
	LIB_DEPENDENCIES ${PYTHON_LIBRARIES} ${Boost_PYTHON_LIBRARY}
)

# Add Python and Boost include directories.
TARGET_INCLUDE_DIRECTORIES(PyScript PUBLIC ${PYTHON_INCLUDE_PATH} ${Boost_INCLUDE_DIRS})

# Install the Python source files that belong to the plugin and which provide the scripting interface.
ADD_CUSTOM_COMMAND(TARGET PyScript POST_BUILD COMMAND ${CMAKE_COMMAND} "-E" copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/python/" "${OVITO_PLUGINS_DIRECTORY}/python/")
INSTALL(DIRECTORY "${OVITO_PLUGINS_DIRECTORY}/python/" DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}/python/")

# Install standard Python packages. 
IF(APPLE)
	STRING(REGEX MATCH "[0-9]+\\.[0-9]+" PYTHON_MAJORMINOR_VERSION ${PYTHONLIBS_VERSION_STRING})
	LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_FIRST_LIBRARY}" PATH)
	INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/python${PYTHON_MAJORMINOR_VERSION}" 
		DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/../Frameworks/Python.framework/Versions/${PYTHON_MAJORMINOR_VERSION}/lib/"
		PATTERN "*.pyc" EXCLUDE PATTERN "*.pyo" EXCLUDE)
ENDIF()
