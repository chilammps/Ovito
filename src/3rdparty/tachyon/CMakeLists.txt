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

# Build the library.
SET(TACHYON_SOURCE_FILES
	tachyon/api.c
	tachyon/apigeom.c
	tachyon/apitrigeom.c
	tachyon/box.c
	tachyon/camera.c
	tachyon/cone.c
	tachyon/coordsys.c
	tachyon/cylinder.c
	tachyon/extvol.c
	tachyon/global.c
	tachyon/grid.c
	tachyon/hash.c
	tachyon/imap.c
	tachyon/imageio.c
	tachyon/intersect.c	
	tachyon/light.c
	tachyon/parallel.c
	tachyon/plane.c
	tachyon/quadric.c
	tachyon/render.c
	tachyon/ring.c
	tachyon/shade.c
	tachyon/sphere.c
	tachyon/texture.c
	tachyon/threads.c
	tachyon/tgafile.c
	tachyon/sgirgb.c
	tachyon/ppm.c
	tachyon/trace.c
	tachyon/triangle.c
	tachyon/ui.c
	tachyon/util.c
	tachyon/vector.c
	tachyon/vol.c
	tachyon/pngfile.c
	tachyon/jpeg.c
	tachyon/psd.c
	tachyon/winbmp.c
)

# Build library.
ADD_LIBRARY(TachyonLib STATIC ${TACHYON_SOURCE_FILES})

TARGET_COMPILE_DEFINITIONS(TachyonLib PRIVATE TACHYON_NO_DEPRECATED THR _REENTRANT)
IF(APPLE)
	TARGET_COMPILE_DEFINITIONS(TachyonLib PRIVATE Bsd)
ELSEIF(UNIX)
	TARGET_COMPILE_DEFINITIONS(TachyonLib PRIVATE Linux)
ELSEIF(WIN32)
	TARGET_COMPILE_DEFINITIONS(TachyonLib PRIVATE WIN32)
ENDIF()

# Make header files of this library available to dependent targets.
TARGET_INCLUDE_DIRECTORIES(TachyonLib INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

IF(APPLE)
	# Link to Pthreads multi-threading library on MacOS.
	TARGET_LINK_LIBRARIES(TachyonLib pthread)
ELSEIF(WIN32)
	# Link to ws2_32 library on Windows.
	TARGET_LINK_LIBRARIES(TachyonLib ws2_32)
ENDIF()

# Since we will link this into the Tachyon plugin, we need to use the same setting for the fPIC flag.
SET_PROPERTY(TARGET TachyonLib PROPERTY POSITION_INDEPENDENT_CODE ON)
# Export this target.
#INSTALL(TARGETS TachyonLib EXPORT OVITO 
#    ARCHIVE DESTINATION "${OVITO_RELATIVE_LIBRARY_DIRECTORY}" 
#    CONFIGURATIONS "")
