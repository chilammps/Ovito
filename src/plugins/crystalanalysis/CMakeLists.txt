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
# Link CGAL library.
INCLUDE(${CGAL_USE_FILE})

SET(OVITO_CRYSTALANALYSIS_SOURCE_FILES
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
	LIST(APPEND OVITO_CRYSTALANALYSIS_SOURCE_FILES 
		scripting/PythonInterface.cpp
	)
ENDIF()

# Generate plugin module.
OVITO_PLUGIN(CrystalAnalysis
	SOURCES ${OVITO_CRYSTALANALYSIS_SOURCE_FILES}
	PLUGIN_DEPENDENCIES Particles
	LIB_DEPENDENCIES ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES} ${OPENGL_gl_LIBRARY}
	OPTIONAL_PLUGIN_DEPENDENCIES PyScript
)

IF(NOT CMAKE_COMPILER_IS_GNUCXX)
	TARGET_COMPILE_DEFINITIONS(CrystalAnalysis PRIVATE CGAL_CFG_ARRAY_MEMBER_INITIALIZATION_BUG)
ENDIF()

# Install CGAL dll and its dependencies (GMP, MPFR, and Boost DLLs).
IF(WIN32)
	OVITO_INSTALL_DLL("${CGAL_LIBRARY}")
	IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
		OVITO_INSTALL_DLL("${GMP_LIBRARIES}")
		OVITO_INSTALL_DLL("${MPFR_LIBRARIES}")
	ENDIF()
	OVITO_INSTALL_DLL("${CGAL_3RD_PARTY_LIBRARIES_DIRS}/libboost_thread-mt.dll")
	OVITO_INSTALL_DLL("${CGAL_3RD_PARTY_LIBRARIES_DIRS}/libboost_system-mt.dll")
ENDIF()

# Install the Python source files that belong to the plugin and which provide the scripting interface.
ADD_CUSTOM_COMMAND(TARGET CrystalAnalysis POST_BUILD COMMAND ${CMAKE_COMMAND} "-E" copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/python/" "${OVITO_PLUGINS_DIRECTORY}/python/")
