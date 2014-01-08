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

SET(OVITO_PARTICLES_SOURCE_FILES 
	data/SimulationCell.cpp
	data/SimulationCellDisplay.cpp
	data/ParticlePropertyObject.cpp
	data/ParticleProperty.cpp
	data/ParticleDisplay.cpp
	data/ParticleType.cpp
	data/ParticleTypeProperty.cpp
	data/VectorDisplay.cpp
	data/BondsObject.cpp
	data/BondsStorage.cpp
	data/BondsDisplay.cpp
	data/ParticleSelectionSet.cpp
	data/SurfaceMesh.cpp
	data/SurfaceMeshDisplay.cpp
	modifier/ParticleModifier.cpp
	modifier/AsynchronousParticleModifier.cpp
	modifier/coloring/AssignColorModifier.cpp
	modifier/coloring/ColorCodingModifier.cpp
	modifier/slice/SliceModifier.cpp
	modifier/delete/DeleteParticlesModifier.cpp
	modifier/analysis/StructureIdentificationModifier.cpp
	modifier/analysis/bondangle/BondAngleAnalysisModifier.cpp
	modifier/analysis/cna/CommonNeighborAnalysisModifier.cpp
	modifier/analysis/displacements/CalculateDisplacementsModifier.cpp
	modifier/analysis/centrosymmetry/CentroSymmetryModifier.cpp
	modifier/analysis/strain/AtomicStrainModifier.cpp
	modifier/analysis/bonds/CreateBondsModifier.cpp
	modifier/analysis/coordination/CoordinationNumberModifier.cpp
	modifier/analysis/histogram/HistogramModifier.cpp
	modifier/analysis/cluster/ClusterAnalysisModifier.cpp
	modifier/lighting/AmbientOcclusionModifier.cpp
	modifier/lighting/AmbientOcclusionRenderer.cpp
	modifier/transformation/AffineTransformationModifier.cpp
	modifier/pbc/ShowPeriodicImagesModifier.cpp
	modifier/pbc/WrapPeriodicImagesModifier.cpp
	modifier/selection/InvertSelectionModifier.cpp
	modifier/selection/SelectParticleTypeModifier.cpp
	modifier/selection/SelectExpressionModifier.cpp
	modifier/selection/FreezeSelectionModifier.cpp
	modifier/selection/ManualSelectionModifier.cpp
	modifier/selection/ClearSelectionModifier.cpp
	modifier/properties/CreateExpressionPropertyModifier.cpp
	importer/ParticleImporter.cpp
	importer/ParticleImportTask.cpp
	importer/InputColumnMapping.cpp
	importer/InputColumnMappingDialog.cpp
	importer/lammps/LAMMPSTextDumpImporter.cpp
	importer/lammps/LAMMPSBinaryDumpImporter.cpp
	importer/lammps/LAMMPSDataImporter.cpp
	importer/vasp/POSCARImporter.cpp
	importer/xyz/XYZImporter.cpp
	importer/imd/IMDImporter.cpp
	importer/parcas/ParcasFileImporter.cpp
	importer/cfg/CFGImporter.cpp
	exporter/ParticleExporter.cpp
	exporter/ParticleExporterSettingsDialog.cpp
	exporter/OutputColumnMapping.cpp
	exporter/vasp/POSCARExporter.cpp
	exporter/lammps/LAMMPSDataExporter.cpp
	exporter/lammps/LAMMPSDumpExporter.cpp
	exporter/xyz/XYZExporter.cpp
	exporter/imd/IMDExporter.cpp
	util/TreeNeighborListBuilder.cpp
	util/OnTheFlyNeighborListBuilder.cpp
	util/ParticlePropertyComboBox.cpp
	util/CutoffRadiusPresetsUI.cpp
	util/ParticleInformationApplet.cpp
	util/ParticlePickingHelper.cpp
	util/ParticlePropertyParameterUI.cpp
)

IF(OVITO_BUILD_PLUGIN_SCRIPTING)
	LIST(APPEND OVITO_PARTICLES_SOURCE_FILES
		scripting/ParticlesBinding.cpp
	)
ENDIF()

# List of source files of the GLU library used for polygon tessellation.
SET(GLUSourceFiles
	util/polytess/dict.c
	util/polytess/geom.c
	util/polytess/memalloc.c
	util/polytess/mesh.c
	util/polytess/normal.c
	util/polytess/priorityq.c
	util/polytess/render.c
	util/polytess/sweep.c
	util/polytess/tess.c
	util/polytess/tessmono.c
)

# Generate plugin module.
OVITO_PLUGIN(Particles
	SOURCES ${OVITO_PARTICLES_SOURCE_FILES} ${GLUSourceFiles}
	RESOURCE particles.rcc resources/particles.qrc
	LIB_DEPENDENCIES muParser qcustomplot
	OPTIONAL_PLUGIN_DEPENDENCIES Scripting
)
