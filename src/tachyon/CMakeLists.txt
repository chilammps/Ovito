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

# The source files of this plugin.
SET(Tachyon_SRCS
	renderer/TachyonRenderer.cpp
	renderer/TachyonRendererEditor.cpp
)

# The Tachyon rednering library source files.
SET(TachyonLib_SRCS
	tachyonlib/api.c
	tachyonlib/apigeom.c
	tachyonlib/apitrigeom.c
	tachyonlib/box.c
	tachyonlib/camera.c
	tachyonlib/coordsys.c
	tachyonlib/cone.c
	tachyonlib/cylinder.c
	tachyonlib/extvol.c
	tachyonlib/global.c
	tachyonlib/grid.c
	tachyonlib/hash.c
	tachyonlib/imap.c
	tachyonlib/imageio.c
	tachyonlib/intersect.c	
	tachyonlib/light.c
	tachyonlib/parallel.c
	tachyonlib/plane.c
	tachyonlib/quadric.c
	tachyonlib/render.c
	tachyonlib/ring.c
	tachyonlib/shade.c
	tachyonlib/sphere.c
	tachyonlib/texture.c
	tachyonlib/threads.c
	tachyonlib/tgafile.c
	tachyonlib/sgirgb.c
	tachyonlib/ppm.c
	tachyonlib/trace.c
	tachyonlib/triangle.c
	tachyonlib/ui.c
	tachyonlib/util.c
	tachyonlib/vector.c
	tachyonlib/vol.c
	tachyonlib/pngfile.c
	tachyonlib/jpeg.c
	tachyonlib/psd.c
	tachyonlib/winbmp.c
)

# Compile Tachyon source files into static library, which will be linked into the Ovito plugin.
ADD_LIBRARY(tachyonlib STATIC ${TachyonLib_SRCS})
ADD_DEFINITIONS(-DTACHYON_NO_DEPRECATED -DTHR -D_REENTRANT)
IF(APPLE)
	ADD_DEFINITIONS(-DBsd)
ELSEIF(UNIX)
	ADD_DEFINITIONS(-DLinux)
	SET_TARGET_PROPERTIES(tachyonlib PROPERTIES COMPILE_FLAGS -fPIC)
ELSEIF(WIN32)
	ADD_DEFINITIONS(-DWIN32)
ENDIF()

# Generate plugin module.
OVITO_PLUGIN(Tachyon
	SOURCES ${Tachyon_SRCS}
	LIB_DEPENDENCIES tachyonlib
)

# Link to Pthreads multi-threading library.
IF(APPLE)
	TARGET_LINK_LIBRARIES(Tachyon pthread)
ENDIF()

