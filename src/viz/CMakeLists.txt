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
	importer/lammps/LAMMPSTextDumpImporter.cpp
)

# Build library
ADD_LIBRARY(Viz ${OVITO_VIZ_SOURCE_FILES})

TARGET_LINK_LIBRARIES(Viz Base Core)

# Link Qt5.
QT5_USE_MODULES(Viz Widgets Xml)
