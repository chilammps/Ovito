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

ADD_SUBDIRECTORY(3rdparty)

##################### Core library  #######################

ADD_SUBDIRECTORY(core)

####################### Plugins ###########################

ADD_SUBDIRECTORY(plugins)

################### Main executable #######################

ADD_SUBDIRECTORY(main)

# Propagate list of plugins to parent scope.
SET(OVITO_PLUGIN_LIST ${OVITO_PLUGIN_LIST} PARENT_SCOPE)


