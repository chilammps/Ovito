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

SET(OVITO_SCRIPTING_SOURCE_FILES
	engine/ScriptEngine.cpp
	engine/ScriptAutostarter.cpp
	ui/ScriptingApplet.cpp
	bindings/BasicTypesBinding.cpp
	bindings/InputOutputBinding.cpp
	bindings/ObjectNodeBinding.cpp
	bindings/ViewportBinding.cpp
)

# Generate plugin module.
OVITO_PLUGIN(Scripting
	SOURCES ${OVITO_SCRIPTING_SOURCE_FILES}
)
