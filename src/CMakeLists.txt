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

# Place all compiled libraries into OVITO's library directory.
SET(LIBRARY_OUTPUT_PATH "${OVITO_LIBRARY_DIRECTORY}")

################# 3rd-party libraries  ####################

ADD_SUBDIRECTORY(3rdparty/botan)
ADD_SUBDIRECTORY(3rdparty/qssh)
ADD_SUBDIRECTORY(3rdparty/muparser)
ADD_SUBDIRECTORY(3rdparty/qcustomplot)
ADD_SUBDIRECTORY(3rdparty/voro++)
IF(OVITO_BUILD_PLUGIN_TACHYON)
	ADD_SUBDIRECTORY(3rdparty/tachyon)
ENDIF()

################# Core library  ####################

ADD_SUBDIRECTORY(core)

################# Plugins ####################

ADD_SUBDIRECTORY(plugins)

################# Main executable ####################

# Place the executable into the "bin" output directory.
SET(EXECUTABLE_OUTPUT_PATH ${OVITO_BINARY_DIRECTORY})
ADD_SUBDIRECTORY(main)

# Propagate the plugin list to the parent scope.
SET(OVITO_PLUGINS_LIST "${OVITO_PLUGINS_LIST}" PARENT_SCOPE)
