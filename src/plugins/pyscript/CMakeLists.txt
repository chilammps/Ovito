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
	binding/ViewportBinding.cpp
	binding/AnimationBinding.cpp
	binding/SceneBinding.cpp
	binding/FileIOBinding.cpp
	binding/RenderingBinding.cpp
	binding/LinearAlgebraBinding.cpp
	binding/ContainerBinding.cpp
	binding/AppBinding.cpp
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

# Install "ovitos" script launcher.
IF(APPLE)
	CONFIGURE_FILE("resources/ovitos" "${OVITO_LIBRARY_DIRECTORY}/ovitos" COPYONLY)
	INSTALL(PROGRAMS "resources/ovitos" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")
ELSEIF(UNIX)
	CONFIGURE_FILE("resources/ovitos" "${OVITO_BINARY_DIRECTORY}/ovitos" COPYONLY)
	INSTALL(PROGRAMS "resources/ovitos" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")
ELSEIF(WIN32)
	# We have to build a second executable under Windows because the main ovito.exe is not a console application.
	SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})
	ADD_EXECUTABLE(ovitos launcher/WinMain.cpp)
	TARGET_LINK_LIBRARIES(ovitos Base Core)
	QT5_USE_MODULES(ovitos ${OVITO_REQUIRED_QT_MODULES})
	INSTALL(TARGETS ovitos DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")
ENDIF()

# Install standard Python packages. 
IF(APPLE)
	STRING(REGEX MATCH "[0-9]+\\.[0-9]+" PYTHON_MAJORMINOR_VERSION ${PYTHONLIBS_VERSION_STRING})
	LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_FIRST_LIBRARY}" PATH)
	INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/python${PYTHON_MAJORMINOR_VERSION}" 
		DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/../Frameworks/Python.framework/Versions/${PYTHON_MAJORMINOR_VERSION}/lib/"
		PATTERN "*.pyc" EXCLUDE PATTERN "*.pyo" EXCLUDE)
ELSEIF(UNIX)
	IF(OVITO_MONOLITHIC_BUILD)
		# Install standard Python packages. 
		STRING(REGEX MATCH "[0-9]+\\.[0-9]+" PYTHON_MAJORMINOR_VERSION ${PYTHONLIBS_VERSION_STRING})
		LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
		GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_LIBRARY}" PATH)
		INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/python${PYTHON_MAJORMINOR_VERSION}" 
			DESTINATION "lib/"
			PATTERN "*.pyc" EXCLUDE PATTERN "*.pyo" EXCLUDE PATTERN "test" EXCLUDE)

		# Install a copy of libpython.so shared library.
		CONFIGURE_FILE("${PYTHON_LIBRARIES}" "${OVITO_LIBRARY_DIRECTORY}" COPYONLY)
		INSTALL(FILES "${PYTHON_LIBRARIES}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")

		# Also install copies of the libssl and libcrypto shared libraries.
		# They are required by the SSL module of Python but might not be present on all target systems.
		FIND_LIBRARY(OVITO_LIBSSL NAMES libssl.so PATHS /usr/lib NO_DEFAULT_PATH)
		FIND_LIBRARY(OVITO_LIBCRYPTO NAMES libcrypto.so PATHS /usr/lib NO_DEFAULT_PATH)
		GET_FILENAME_COMPONENT(OVITO_LIBSSL_REAL ${OVITO_LIBSSL} REALPATH)
		GET_FILENAME_COMPONENT(OVITO_LIBCRYPTO_REAL ${OVITO_LIBCRYPTO} REALPATH)
		INSTALL(FILES "${OVITO_LIBSSL}" "${OVITO_LIBSSL_REAL}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
		INSTALL(FILES "${OVITO_LIBCRYPTO}" "${OVITO_LIBCRYPTO_REAL}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
	ENDIF()
ELSEIF(WIN32)
	# Install standard Python packages. 
	LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_FIRST_LIBRARY}" PATH)
	INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/../Lib" 
		DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}"
		PATTERN "*.pyc" EXCLUDE PATTERN "*.pyo" EXCLUDE PATTERN "test" EXCLUDE)
	
	# Install Python interpreter and Boost.Python DLL libraries, which are required by the OVITO PyScript plugin.

	# The boost_python DLL is located in the lib directory of the Boost installation.
	IF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		OVITO_INSTALL_DLL("${Boost_LIBRARY_DIRS}/libboost_python-mt.dll")
	ELSE()
		OVITO_INSTALL_DLL("${Boost_LIBRARY_DIRS}/libboost_python-mt-d.dll")
	ENDIF()
	
	# The 32-bit python interpreter DLL (python27.dll) is usually located in the Windows SysWOW64 directory.
	FIND_FILE(PYTHON_DLL NAMES python27.dll HINTS "C:/Windows/SysWOW64" ENV PATH DOC "Python interpreter DLL")
	IF(NOT PYTHON_DLL)
		MESSAGE(FATAL_ERROR "Please set the PYTHON_DLL variable to specify the location of the Python interpreter DLL.")
	ENDIF()
	OVITO_INSTALL_DLL("${PYTHON_DLL}")
ENDIF()

