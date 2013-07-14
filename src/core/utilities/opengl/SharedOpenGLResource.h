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
 * \file SharedOpenGLResource.h
 * \brief Contains the definition of the Ovito::SharedOpenGLResource class.
 */

#ifndef __OVITO_SHARED_OPENGL_RESOURCE_H
#define __OVITO_SHARED_OPENGL_RESOURCE_H

#include <core/Core.h>

namespace Ovito {

class OpenGLContextInfo;
class OpenGLContextManager;

class SharedOpenGLResource
{
public:

	/// Destructor.
    ~SharedOpenGLResource() { destroyOpenGLResources(); }

    /// This should be called after the OpenGL resources have been allocated.
    void attachOpenGLResources();

    /// This will free the OpenGL resources. It is automatically called by the destructor.
    void destroyOpenGLResources();

protected:

    /// This method that takes care of freeing the shared OpenGL resources.
    virtual void freeOpenGLResources() = 0;

private:
    OpenGLContextInfo* _contextInfo = nullptr;
    SharedOpenGLResource* _next = nullptr;
    SharedOpenGLResource* _prev = nullptr;

    friend class OpenGLContextManager;
    friend class OpenGLContextInfo;
};

};

#endif // __OVITO_SHARED_OPENGL_RESOURCE_H
