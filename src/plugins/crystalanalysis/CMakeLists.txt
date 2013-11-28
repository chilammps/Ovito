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

SET(OVITO_CRYSTALANALYSIS_SOURCE_FILES
	importer/CAImporter.cpp
	data/patterns/BurgersVectorFamily.cpp
	data/patterns/StructurePattern.cpp
	data/patterns/PatternCatalog.cpp
	data/surface/DefectSurface.cpp
	data/surface/DefectSurfaceDisplay.cpp
	data/clusters/Cluster.cpp
	data/clusters/ClusterGraph.cpp
	data/dislocations/DislocationSegment.cpp
	data/dislocations/DislocationNetwork.cpp
	data/dislocations/DislocationDisplay.cpp
	data/dislocations/DislocationInspector.cpp
	modifier/ShiftModifier.cpp
	modifier/SmoothSurfaceModifier.cpp
	modifier/SmoothDislocationsModifier.cpp
)

# List of source files of the GLU library used for polygon tessellation.
SET(GLUSourceFiles
	data/surface/polytess/dict.c
	data/surface/polytess/geom.c
	data/surface/polytess/memalloc.c
	data/surface/polytess/mesh.c
	data/surface/polytess/normal.c
	data/surface/polytess/priorityq.c
	data/surface/polytess/render.c
	data/surface/polytess/sweep.c
	data/surface/polytess/tess.c
	data/surface/polytess/tessmono.c
)

# Generate plugin module.
OVITO_PLUGIN(CrystalAnalysis
	SOURCES ${OVITO_CRYSTALANALYSIS_SOURCE_FILES} ${GLUSourceFiles}
	PLUGIN_DEPENDENCIES Particles
)
