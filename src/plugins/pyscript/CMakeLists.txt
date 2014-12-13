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

SET(Python_ADDITIONAL_VERSIONS "3.4" "3.5" "3.6")
FIND_PACKAGE(PythonLibs REQUIRED)
FIND_PACKAGE(QScintilla REQUIRED)

IF(MSVC)
	# Precompiled headers require more compiler memory.
	ADD_DEFINITIONS(-Zm150)
ENDIF()

SET(SourceFiles
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
	extensions/PythonViewportOverlay.cpp
)

OVITO_STANDARD_PLUGIN(PyScript
	SOURCES PyScript.cpp ${SourceFiles}
	LIB_DEPENDENCIES ${PYTHON_LIBRARIES} ${Boost_PYTHON_LIBRARY} ${QScintilla_LIBRARIES}
	PYTHON_WRAPPERS "${CMAKE_CURRENT_SOURCE_DIR}/python/"
)

# Add Python include directory.
TARGET_INCLUDE_DIRECTORIES(PyScript PUBLIC ${PYTHON_INCLUDE_PATH})

# Dependency on Python and Boost.Python library should be transitive.
TARGET_LINK_LIBRARIES(PyScript PUBLIC ${PYTHON_LIBRARIES} ${Boost_PYTHON_LIBRARY})

# Add QScintilla include directory.
TARGET_INCLUDE_DIRECTORIES(PyScript PRIVATE ${QScintilla_INCLUDE_DIRS})

# Speed up compilation by using precompiled headers.
ADD_PRECOMPILED_HEADER(PyScript plugins/pyscript/PyScript.h)

# Install the Python source files that belong to the plugin and which provide the scripting interface.
INSTALL(DIRECTORY "${OVITO_PLUGINS_DIRECTORY}/python/" DESTINATION "${OVITO_RELATIVE_PLUGINS_DIRECTORY}/python/")

# Install "ovitos" script launcher.
IF(APPLE)

	SET(OVITOS_EXECUTABLE "${OVITO_LIBRARY_DIRECTORY}/ovitos")
	CONFIGURE_FILE("resources/ovitos" "${OVITOS_EXECUTABLE}" COPYONLY)
	INSTALL(PROGRAMS "resources/ovitos" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}")

	# Create a nested application bundle with a modified Info.plist file
	# that starts the application without displaying the dock icon.
	EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -E make_directory "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/Ovito.App/Contents")
	EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -E create_symlink "../../../MacOS" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/Ovito.App/Contents/MacOS")
	EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -E create_symlink "../../../Resources" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/Ovito.App/Contents/Resources")
	EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -E create_symlink "../../../Frameworks" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/Ovito.App/Contents/Frameworks")
	CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/src/main/resources/Info.plist" "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/Ovito.App/Contents/Info.plist")
	EXECUTE_PROCESS(COMMAND defaults write "${OVITO_BINARY_DIRECTORY}/${MACOSX_BUNDLE_NAME}.app/Contents/MacOS/Ovito.App/Contents/Info" LSUIElement 1)

ELSEIF(UNIX)

	SET(OVITOS_EXECUTABLE "${OVITO_BINARY_DIRECTORY}/ovitos")
	CONFIGURE_FILE("resources/ovitos" "${OVITOS_EXECUTABLE}" COPYONLY)
	INSTALL(PROGRAMS "resources/ovitos" DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

ELSEIF(WIN32)

	# We have to build a second executable under Windows because the main ovito.exe is not a console application.
	ADD_EXECUTABLE(ovitos launcher/WinMain.cpp)
    SET_TARGET_PROPERTIES(ovitos PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OVITO_BINARY_DIRECTORY}")
	TARGET_LINK_LIBRARIES(ovitos Core)
	TARGET_LINK_LIBRARIES(ovitos Qt5::Core Qt5::Gui)
	INSTALL(TARGETS ovitos DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")
	GET_PROPERTY(OVITOS_EXECUTABLE TARGET ovitos PROPERTY LOCATION)
	# Build script launcher together with PyScript plugin.
	ADD_DEPENDENCIES(PyScript ovitos)

ENDIF()

# Install standard Python packages. 
IF(APPLE)
	STRING(REGEX MATCH "[0-9]+\\.[0-9]+" PYTHON_MAJORMINOR_VERSION ${PYTHONLIBS_VERSION_STRING})
	LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_FIRST_LIBRARY}" PATH)
	INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/python${PYTHON_MAJORMINOR_VERSION}" 
		DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/../Frameworks/Python.framework/Versions/${PYTHON_MAJORMINOR_VERSION}/lib/"
		PATTERN "*.pyc" EXCLUDE 
		PATTERN "*.pyo" EXCLUDE 
		PATTERN "test" EXCLUDE
		PATTERN "*.exe" EXCLUDE
		PATTERN "sphinx" EXCLUDE)	
ELSEIF(UNIX)
	IF(OVITO_MONOLITHIC_BUILD)

		# Install standard Python packages. 
		STRING(REGEX MATCH "[0-9]+\\.[0-9]+" PYTHON_MAJORMINOR_VERSION ${PYTHONLIBS_VERSION_STRING})
		LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
		GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_LIBRARY}" PATH)
		INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/python${PYTHON_MAJORMINOR_VERSION}" 
			DESTINATION "lib/"
			PATTERN "*.pyc" EXCLUDE 
			PATTERN "*.pyo" EXCLUDE 
			PATTERN "__pycache__" EXCLUDE
			PATTERN "test" EXCLUDE
			PATTERN "sphinx" EXCLUDE
			PATTERN "Sphinx*" EXCLUDE
			PATTERN "Jinja*" EXCLUDE
			PATTERN "Pygments*" EXCLUDE
			PATTERN "Babel" EXCLUDE)

		# Install a copy of libpython.so shared library.
		CONFIGURE_FILE("${PYTHON_LIBRARIES}" "${OVITO_LIBRARY_DIRECTORY}" COPYONLY)
		INSTALL(FILES "${PYTHON_LIBRARIES}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")

		# Also install copies of shared libraries that are required by Python modules.
		SET(OVITO_PYTHON_DEPENDENCIES libssl.so libcrypto.so liblapack.so.3gf libblas.so.3gf libquadmath.so.0 libgfortran.so.3 libncurses.so)
		FOREACH(PYTHON_DEPENDENCY ${OVITO_PYTHON_DEPENDENCIES})
			FIND_LIBRARY(OVITO_PYTHON_DEP NAMES ${PYTHON_DEPENDENCY} PATHS /usr/lib NO_DEFAULT_PATH)
			IF(NOT OVITO_PYTHON_DEP)
				MESSAGE(FATAL_ERROR "Could not find shared library ${PYTHON_DEPENDENCY}, which is required by Python interpreter, in system path.")
			ENDIF()
			GET_FILENAME_COMPONENT(OVITO_PYTHON_DEP_REAL ${OVITO_PYTHON_DEP} REALPATH)
			INSTALL(FILES "${OVITO_PYTHON_DEP_REAL}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
			IF(NOT "${OVITO_PYTHON_DEP_REAL}" STREQUAL "${OVITO_PYTHON_DEP}")
				GET_FILENAME_COMPONENT(OVITO_PYTHON_DEP_REAL_NAME ${OVITO_PYTHON_DEP_REAL} NAME)
				EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${OVITO_PYTHON_DEP_REAL_NAME} "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_DEPENDENCY}")
				INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_DEPENDENCY} DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
			ENDIF()
			UNSET(OVITO_PYTHON_DEP CACHE)
		ENDFOREACH()

		# Install a copy of the QScintilla2 library.
		CONFIGURE_FILE("${QScintilla_LIBRARIES}" "${OVITO_LIBRARY_DIRECTORY}" COPYONLY)		
		GET_FILENAME_COMPONENT(real_qscintilla_lib "${QScintilla_LIBRARIES}" REALPATH)
		INSTALL(FILES "${QScintilla_LIBRARIES}" "${real_qscintilla_lib}" DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}/")
	ENDIF()
ELSEIF(WIN32)
	# Install standard Python packages. 
	LIST(GET PYTHON_LIBRARIES 0 PYTHON_FIRST_LIBRARY)
	GET_FILENAME_COMPONENT(PYTHON_LIBRARY_PATH "${PYTHON_FIRST_LIBRARY}" PATH)
	GET_FILENAME_COMPONENT(PYTHON_LIBRARY_NAME "${PYTHON_FIRST_LIBRARY}" NAME_WE)
	INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/../Lib" 
		DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}"
		PATTERN "*.pyc" EXCLUDE 
		PATTERN "*.pyo" EXCLUDE
		PATTERN "__pycache__" EXCLUDE)
	# Also install the compiled modules that are shipped with Python.
	INSTALL(DIRECTORY "${PYTHON_LIBRARY_PATH}/../DLLs"
		DESTINATION "${OVITO_RELATIVE_BINARY_DIRECTORY}")

	# Install Python interpreter dll.
	OVITO_INSTALL_DLL("${PYTHON_LIBRARY_PATH}/../${PYTHON_LIBRARY_NAME}.dll")
	
	# Install QScintilla DLL.
	GET_FILENAME_COMPONENT(QSCINTILLA_BASE_NAME "${QScintilla_LIBRARIES}" NAME_WE)
	GET_FILENAME_COMPONENT(QSCINTILLA_LIB_PATH "${QScintilla_LIBRARIES}" PATH)
	OVITO_INSTALL_DLL("${QSCINTILLA_LIB_PATH}/${QSCINTILLA_BASE_NAME}.dll")
ENDIF()

# Define tests for the scripting interface.
SET(PYTHON_TEST_SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/tests/scripts/test_suite")
FILE(GLOB_RECURSE PYTHON_TEST_SCRIPTS RELATIVE "${PYTHON_TEST_SCRIPTS_DIR}" "${PYTHON_TEST_SCRIPTS_DIR}/*.py")
FOREACH(test_script ${PYTHON_TEST_SCRIPTS})
	ADD_TEST(NAME "${test_script}" WORKING_DIRECTORY "${PYTHON_TEST_SCRIPTS_DIR}" COMMAND "${OVITOS_EXECUTABLE}" "${test_script}")
ENDFOREACH()
