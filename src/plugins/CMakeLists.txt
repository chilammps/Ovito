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

# Place all plugin libraries into the plugins directory.
FILE(MAKE_DIRECTORY ${OVITO_PLUGINS_DIRECTORY})
INSTALL(DIRECTORY DESTINATION ${OVITO_RELATIVE_PLUGINS_DIRECTORY})
SET(LIBRARY_OUTPUT_PATH "${OVITO_PLUGINS_DIRECTORY}")

IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	ADD_SUBDIRECTORY(pyscript)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/PyScript.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_PARTICLES)
	ADD_SUBDIRECTORY(particles)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Particles.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_TACHYON)
	ADD_SUBDIRECTORY(tachyon)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Tachyon.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_MESH)
	ADD_SUBDIRECTORY(mesh)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Mesh.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS)
	ADD_SUBDIRECTORY(crystalanalysis)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/CrystalAnalysis.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_NETCDF)
	ADD_SUBDIRECTORY(netcdf)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/NetCDF.manifest.xml)
ENDIF()
