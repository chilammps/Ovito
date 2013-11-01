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

ADD_SUBDIRECTORY(ssh)
ADD_SUBDIRECTORY(muparser)
ADD_SUBDIRECTORY(qcustomplot)

################# Core libraries  ####################

ADD_SUBDIRECTORY(base)
IF(OVITO_VIDEO_OUTPUT_SUPPORT)
	ADD_SUBDIRECTORY(video)
ENDIF()
ADD_SUBDIRECTORY(core)

################# Plugins ####################

# Place all plugins libraries into the plugin directory.
FILE(MAKE_DIRECTORY ${OVITO_PLUGINS_DIRECTORY})
INSTALL(DIRECTORY DESTINATION ${OVITO_RELATIVE_PLUGINS_DIRECTORY})
SET(LIBRARY_OUTPUT_PATH "${OVITO_PLUGINS_DIRECTORY}")

ADD_SUBDIRECTORY(particles)
ADD_SUBDIRECTORY(tachyon)
ADD_SUBDIRECTORY(mesh)

################# Main executable ####################

# Place the executable into the "bin" output directory.
SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})

ADD_SUBDIRECTORY(main)
