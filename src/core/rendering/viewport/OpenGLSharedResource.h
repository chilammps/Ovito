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

/** 
 * \file OpenGLSharedResource.h
 * \brief Contains the definition of the Ovito::OpenGLSharedResource class.
 */

#ifndef __OVITO_SHARED_OPENGL_RESOURCE_H
#define __OVITO_SHARED_OPENGL_RESOURCE_H

#include <core/Core.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

class OpenGLContextInfo;
class OpenGLContextManager;

class OVITO_CORE_EXPORT OpenGLSharedResource
{
public:

	/// Destructor.
    ~OpenGLSharedResource();

    /// This should be called after the OpenGL resources have been allocated.
    void attachOpenGLResources();

    /// This will free the OpenGL resources. It is automatically called by the destructor.
    void destroyOpenGLResources();

protected:

    /// This method that takes care of freeing the shared OpenGL resources.
    virtual void freeOpenGLResources() = 0;

private:
    OpenGLContextInfo* _contextInfo = nullptr;
    OpenGLSharedResource* _next = nullptr;
    OpenGLSharedResource* _prev = nullptr;

    friend class OpenGLContextManager;
    friend class OpenGLContextInfo;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_SHARED_OPENGL_RESOURCE_H
