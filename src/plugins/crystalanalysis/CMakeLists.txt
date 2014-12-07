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

# Find the CGAL library.
FIND_PACKAGE(CGAL REQUIRED)
IF(NOT CGAL_FOUND)
	MESSAGE(FATAL_ERROR "Could not locate CGAL library, which is required by CrystalAnalysis plugin.")
ENDIF()
# Link CGAL library.
INCLUDE(${CGAL_USE_FILE})

SET(SourceFiles
	importer/CAImporter.cpp
	data/patterns/BurgersVectorFamily.cpp
	data/patterns/StructurePattern.cpp
	data/patterns/PatternCatalog.cpp
	data/clusters/Cluster.cpp
	data/clusters/ClusterGraph.cpp
	data/dislocations/DislocationSegment.cpp
	data/dislocations/DislocationNetwork.cpp
	data/dislocations/DislocationDisplay.cpp
	data/dislocations/DislocationInspector.cpp
	modifier/ShiftModifier.cpp
	modifier/SmoothSurfaceModifier.cpp
	modifier/SmoothDislocationsModifier.cpp
	modifier/ConstructSurfaceModifier.cpp
	util/DelaunayTessellation.cpp
)

IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	LIST(APPEND SourceFiles scripting/PythonInterface.cpp)
ENDIF()

OVITO_STANDARD_PLUGIN(CrystalAnalysis
	SOURCES ${SourceFiles}
	PLUGIN_DEPENDENCIES Particles
	LIB_DEPENDENCIES ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${OPENGL_gl_LIBRARY}
	OPTIONAL_PLUGIN_DEPENDENCIES PyScript
	PYTHON_WRAPPERS "${CMAKE_CURRENT_SOURCE_DIR}/resources/python/"
)

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
	TARGET_COMPILE_DEFINITIONS(CrystalAnalysis PRIVATE CGAL_CFG_ARRAY_MEMBER_INITIALIZATION_BUG)
ENDIF()

# Install CGAL dll and its dependencies (GMP, MPFR, and Boost DLLs).
IF(WIN32)
	FILE(GLOB CGAL_LIBRARY_DLLS "${CGAL_LIBRARIES_DIR}/../bin/CGAL*.dll")
	FOREACH(dll ${CGAL_LIBRARY_DLLS})
		OVITO_INSTALL_DLL("${dll}")
	ENDFOREACH()

	GET_FILENAME_COMPONENT(GMP_BASE_NAME "${GMP_LIBRARIES}" NAME_WE)
	GET_FILENAME_COMPONENT(GMP_LIB_PATH "${GMP_LIBRARIES}" PATH)
	OVITO_INSTALL_DLL("${GMP_LIB_PATH}/${GMP_BASE_NAME}.dll")

	GET_FILENAME_COMPONENT(MPFR_BASE_NAME "${MPFR_LIBRARIES}" NAME_WE)
	GET_FILENAME_COMPONENT(MPFR_LIB_PATH "${MPFR_LIBRARIES}" PATH)
	OVITO_INSTALL_DLL("${MPFR_LIB_PATH}/${MPFR_BASE_NAME}.dll")

	IF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		GET_FILENAME_COMPONENT(BOOST_SYSTEM_BASE_NAME "${Boost_SYSTEM_LIBRARY_RELEASE}" NAME_WE)
		GET_FILENAME_COMPONENT(BOOST_SYSTEM_LIB_PATH "${Boost_SYSTEM_LIBRARY_RELEASE}" PATH)
	ELSE()
		GET_FILENAME_COMPONENT(BOOST_SYSTEM_BASE_NAME "${Boost_SYSTEM_LIBRARY_DEBUG}" NAME_WE)
		GET_FILENAME_COMPONENT(BOOST_SYSTEM_LIB_PATH "${Boost_SYSTEM_LIBRARY_DEBUG}" PATH)
	ENDIF()
	OVITO_INSTALL_DLL("${BOOST_SYSTEM_LIB_PATH}/${BOOST_SYSTEM_BASE_NAME}.dll")
ENDIF()
