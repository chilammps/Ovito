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

#include <core/Core.h>
#include "OpenGLSharedResource.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Rendering) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

class OpenGLContextInfo
{
public:

	/// Constructor that creates a wrapper object for the given OpenGL context.
	OpenGLContextInfo(QOpenGLContext* ctx, QSurface* surface) : _context(ctx), _resources(nullptr) {
		if(surface->surfaceClass() == QSurface::Window)
			_windowSurface = static_cast<QWindow*>(surface);
		else if(surface->surfaceClass() == QSurface::Offscreen)
			_offscreenSurface = static_cast<QOffscreenSurface*>(surface);
	}

	/// Destructor.
	~OpenGLContextInfo() {
        // Detach this information block from all of the shared resources that used to be owned by it.
        for(OpenGLSharedResource* resource = _resources; resource != nullptr; resource = resource->_next) {
            resource->_contextInfo = nullptr;
        }
    }

	/// The OpenGL context wrapped by this object.
    QOpenGLContext* _context;

    /// The window surface needed to make the OpenGL context current.
    QPointer<QWindow> _windowSurface;

    /// The offscreen surface needed to make the OpenGL context current.
    QPointer<QOffscreenSurface> _offscreenSurface;

    /// Linked list of resources associated with the OpenGL context.
    OpenGLSharedResource* _resources;
};

class OpenGLContextManager : public QObject
{
	Q_OBJECT

public:

	~OpenGLContextManager() {
		qDeleteAll(_contexts);
	}

	OpenGLContextInfo* contextInfo(QOpenGLContext* ctx) {
		// Look for existing context wrapper.
		for(OpenGLContextInfo* info : _contexts) {
			if(info->_context == ctx)
				return info;
		}
		// Create a new context wrapper.
		OpenGLContextInfo* info = new OpenGLContextInfo(ctx, ctx->surface());
		_contexts.append(info);
		// Install listener.
		connect(ctx, &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLContextManager::aboutToDestroyContext, Qt::DirectConnection);
		return info;
	}

private Q_SLOTS:

	/// Is called when an OpenGL context is being destroyed.
    void aboutToDestroyContext();

private:

    QList<OpenGLContextInfo *> _contexts;
};

#ifndef ONLY_FOR_DOXYGEN
	#include "OpenGLSharedResource.moc"
#endif

static QThreadStorage<OpenGLContextManager*> glContextManagerStorage;

static OpenGLContextManager* glresource_context_manager() {
	if(!glContextManagerStorage.hasLocalData()) {
		glContextManagerStorage.setLocalData(new OpenGLContextManager());
	}
	return glContextManagerStorage.localData();
}

void OpenGLContextManager::aboutToDestroyContext()
{
	QOpenGLContext* ctx = qobject_cast<QOpenGLContext*>(sender());
	OVITO_CHECK_POINTER(ctx);
	
	int index = 0;
	while(index < _contexts.size()) {
		OpenGLContextInfo* info = _contexts[index];
		if(info->_context == ctx) {
		    QList<QOpenGLContext*> shares = ctx->shareGroup()->shares();
			if(shares.size() >= 2) {
				// Transfer ownership to another context in the same sharing
				// group.  This may result in multiple OpenGLContextInfo objects
				// for the same context, which is ok.
				info->_context = ((ctx == shares.at(0)) ? shares.at(1) : shares.at(0));
			} else {
				// All contexts in the sharing group have been deleted,
				// so detach all of the shared resources.
				_contexts.removeAt(index);
				delete info;
				continue;
			}
		}
		++index;
	}
}

/// Destructor.
OpenGLSharedResource::~OpenGLSharedResource()
{
	destroyOpenGLResources(); 
}

void OpenGLSharedResource::attachOpenGLResources()
{
    OVITO_ASSERT(_contextInfo == nullptr);

    QOpenGLContext* context = QOpenGLContext::currentContext();
    OVITO_CHECK_POINTER(context);

    OpenGLContextManager* manager = glresource_context_manager();

    _contextInfo = manager->contextInfo(context);
    _next = _contextInfo->_resources;
    _prev = nullptr;
    if(_contextInfo->_resources)
    	_contextInfo->_resources->_prev = this;
    _contextInfo->_resources = this;
}

void OpenGLSharedResource::destroyOpenGLResources()
{
	if(!_contextInfo)
		return;

    OpenGLContextManager* manager = glresource_context_manager();

    // Detach this resource from the context information block.
	if(_next) _next->_prev = _prev;
	if(_prev) _prev->_next = _next;
	else _contextInfo->_resources = _next;

	QOpenGLContext* ownerContext = _contextInfo->_context;
	QSurface* ownerSurface = _contextInfo->_windowSurface.data();
	if(!ownerSurface) ownerSurface = _contextInfo->_offscreenSurface.data();
    _contextInfo = nullptr;
    _next = _prev = nullptr;

    // Switch back to the owning context temporarily and delete the id.
	QOpenGLContext* currentContext = QOpenGLContext::currentContext();

	if(currentContext != ownerContext && (!currentContext || !QOpenGLContext::areSharing(ownerContext, currentContext))) {
		if(ownerSurface != nullptr) {
			QSurface* currentSurface = currentContext ? currentContext->surface() : nullptr;
			ownerContext->makeCurrent(ownerSurface);
			freeOpenGLResources();
			if(currentContext)
				currentContext->makeCurrent(currentSurface);
			else
				ownerContext->doneCurrent();
		}
	}
	else {
		freeOpenGLResources();
	}
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
