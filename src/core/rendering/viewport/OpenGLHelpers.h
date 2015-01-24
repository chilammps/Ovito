///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OVITO_OPENGL_HELPERS_H
#define __OVITO_OPENGL_HELPERS_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

// The minimum OpenGL version required by Ovito:
#define OVITO_OPENGL_MINIMUM_VERSION_MAJOR 			2
#define OVITO_OPENGL_MINIMUM_VERSION_MINOR			1

// The standard OpenGL version used by Ovito:
#define OVITO_OPENGL_REQUESTED_VERSION_MAJOR 		3
#define OVITO_OPENGL_REQUESTED_VERSION_MINOR		2

/// Reports OpenGL error status codes.
extern OVITO_CORE_EXPORT void checkOpenGLErrorStatus(const char* command, const char* sourceFile, int sourceLine);

// OpenGL debugging macro:
#ifdef OVITO_DEBUG
	#define OVITO_CHECK_OPENGL(cmd)									\
	{																\
		cmd;														\
		Ovito::checkOpenGLErrorStatus(#cmd, __FILE__, __LINE__);	\
	}
    #define OVITO_REPORT_OPENGL_ERRORS() Ovito::checkOpenGLErrorStatus("", __FILE__, __LINE__);
#else
	#define OVITO_CHECK_OPENGL(cmd)			cmd
    #define OVITO_REPORT_OPENGL_ERRORS()
#endif

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_HELPERS_H
