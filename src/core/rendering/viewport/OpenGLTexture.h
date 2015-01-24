///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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
 * \file OpenGLTexture.h
 * \brief Contains the definition of the Ovito::OpenGLTexture class.
 */

#ifndef __OVITO_OPENGL_TEXTURE_H
#define __OVITO_OPENGL_TEXTURE_H

#include <core/Core.h>
#include "OpenGLSharedResource.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A wrapper class for OpenGL textures.
 */
class OVITO_CORE_EXPORT OpenGLTexture : private OpenGLSharedResource
{
public:

	/// Constructor.
	OpenGLTexture() : _id(0) {}

	/// Destructor.
	~OpenGLTexture() { destroyOpenGLResources(); }

	/// Create the texture object.
	void create() {
		if(_id != 0) return;

		QOpenGLContext::currentContext()->functions()->glActiveTexture(GL_TEXTURE0);

		// Create OpenGL texture.
		glGenTextures(1, &_id);

		// Make sure texture gets deleted when this object is destroyed.
		attachOpenGLResources();
	}

	/// Returns true if the texture has been created; false otherwise.
	bool isCreated() const { return _id != 0; }

	/// Makes this the active texture.
	void bind() {
		QOpenGLContext::currentContext()->functions()->glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _id);
	}

protected:

    /// This method that takes care of freeing the shared OpenGL resources owned by this class.
    virtual void freeOpenGLResources() override {
    	if(_id) {
			glDeleteTextures(1, &_id);
			_id = 0;
    	}
    }

private:

	/// Resource identifier of the OpenGL texture.
	GLuint _id;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OPENGL_TEXTURE_H
