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

SET(OVITO_MESH_SOURCE_FILES 
	import/TriMeshImportData.cpp
	import/VTKFileImporter.cpp
)

IF(OVITO_BUILD_PLUGIN_PYSCRIPT)
	LIST(APPEND OVITO_MESH_SOURCE_FILES 
		scripting/PythonInterface.cpp
	)
ENDIF()

# Generate plugin module.
OVITO_PLUGIN(Mesh
	SOURCES ${OVITO_MESH_SOURCE_FILES}
	OPTIONAL_PLUGIN_DEPENDENCIES PyScript	
)

# Install the Python source files that belong to the plugin and which provide the scripting interface.
ADD_CUSTOM_COMMAND(TARGET Mesh POST_BUILD COMMAND ${CMAKE_COMMAND} "-E" copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/python/" "${OVITO_PLUGINS_DIRECTORY}/python/")
