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

SET(OVITO_VIZ_SOURCE_FILES 
	data/SimulationCell.cpp
	data/SimulationCellDisplay.cpp
	data/ParticlePropertyObject.cpp
	data/ParticleProperty.cpp
	data/ParticleDisplay.cpp
	data/ParticleType.cpp
	data/ParticleTypeProperty.cpp
	modifier/ParticleModifier.cpp
	modifier/AsynchronousParticleModifier.cpp
	modifier/coloring/AssignColorModifier.cpp
	modifier/slice/SliceModifier.cpp
	modifier/analysis/StructureIdentificationModifier.cpp
	modifier/analysis/bondangle/BondAngleAnalysisModifier.cpp
	modifier/analysis/cna/CommonNeighborAnalysisModifier.cpp
	modifier/lighting/AmbientOcclusionModifier.cpp
	modifier/lighting/AmbientOcclusionRenderer.cpp
	importer/ParticleImporter.cpp
	importer/ParticleImportData.cpp
	importer/InputColumnMapping.cpp
	importer/lammps/LAMMPSTextDumpImporter.cpp
	importer/lammps/LAMMPSDumpImporterSettingsDialog.cpp
	util/TreeNeighborListBuilder.cpp
)

# Generate plugin module.
OVITO_PLUGIN(Viz
	SOURCES ${OVITO_VIZ_SOURCE_FILES}
	RESOURCE viz.rcc resources/viz.qrc
)

# Link Qt5.
QT5_USE_MODULES(Viz Widgets Xml Concurrent)
