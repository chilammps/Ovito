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

# Place all compiled libraries into OVITO's library directory.
SET(LIBRARY_OUTPUT_PATH "${OVITO_LIBRARY_DIRECTORY}")

################# 3rd-party libraries  ####################

ADD_SUBDIRECTORY(3rdparty/botan)
ADD_SUBDIRECTORY(3rdparty/qssh)
ADD_SUBDIRECTORY(3rdparty/muparser)
ADD_SUBDIRECTORY(3rdparty/qcustomplot)
ADD_SUBDIRECTORY(3rdparty/voro++)

################# Core libraries  ####################

ADD_SUBDIRECTORY(base)
IF(OVITO_VIDEO_OUTPUT_SUPPORT)
	ADD_SUBDIRECTORY(3rdparty/video)
ENDIF()
ADD_SUBDIRECTORY(core)

################# Plugins ####################

# Place all plugin libraries into the plugins directory.
FILE(MAKE_DIRECTORY ${OVITO_PLUGINS_DIRECTORY})
INSTALL(DIRECTORY DESTINATION ${OVITO_RELATIVE_PLUGINS_DIRECTORY})
SET(LIBRARY_OUTPUT_PATH "${OVITO_PLUGINS_DIRECTORY}")

IF(OVITO_BUILD_PLUGIN_SCRIPTING)
	ADD_SUBDIRECTORY(plugins/scripting)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Scripting.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_PARTICLES)
	ADD_SUBDIRECTORY(plugins/particles)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Particles.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_TACHYON)
	ADD_SUBDIRECTORY(3rdparty/tachyon)
	ADD_SUBDIRECTORY(plugins/tachyon)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Tachyon.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_MESH)
	ADD_SUBDIRECTORY(plugins/mesh)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/Mesh.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS)
	ADD_SUBDIRECTORY(plugins/crystalanalysis)
ELSE()
	FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/CrystalAnalysis.manifest.xml)
ENDIF()

IF(OVITO_BUILD_PLUGIN_NETCDF)
	ADD_SUBDIRECTORY(plugins/netcdf)
ELSE()
        FILE(REMOVE ${OVITO_PLUGINS_DIRECTORY}/NetCDF.manifest.xml)
ENDIF()

################# Main executable ####################

# Place the executable into the "bin" output directory.
SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})

ADD_SUBDIRECTORY(main)
