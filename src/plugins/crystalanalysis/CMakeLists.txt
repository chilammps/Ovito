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

# Find CGAL library.
FIND_PACKAGE(CGAL REQUIRED)
IF(NOT CGAL_FOUND)
	MESSAGE(FATAL_ERROR "CGAL library, which is required by the CrystalAnalysis plugin, not found.")
ENDIF()
INCLUDE(${CGAL_USE_FILE})
IF(NOT CMAKE_COMPILER_IS_GNUCXX)
	ADD_DEFINITIONS(-DCGAL_CFG_ARRAY_MEMBER_INITIALIZATION_BUG)
ENDIF()

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

# Generate plugin module.
OVITO_PLUGIN(CrystalAnalysis
	SOURCES ${OVITO_CRYSTALANALYSIS_SOURCE_FILES}
	PLUGIN_DEPENDENCIES Particles
	LIB_DEPENDENCIES ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES}
)
