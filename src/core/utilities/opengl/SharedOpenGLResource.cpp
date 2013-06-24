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
#include "SharedOpenGLResource.h"

namespace Ovito {

class OpenGLContextInfo
{
public:

	/// Constructor that creates a wrapper object for the given OpenGL context.
	OpenGLContextInfo(QOpenGLContext* ctx, QSurface* surface) : _context(ctx), _surface(surface), _resources(nullptr) {}

	/// Destructor.
	~OpenGLContextInfo() {
        // Detach this information block from all of the shared resources that used to be owned by it.
        for(SharedOpenGLResource* resource = _resources; resource != nullptr; resource = resource->_next) {
            resource->_contextInfo = nullptr;
        }
    }

	/// The OpenGL context wrapped by this object.
    QOpenGLContext* _context;

    /// The surface needed to make the OpenGL context current.
    QSurface* _surface;

    /// Linked list of resources associated with the OpenGL context.
    SharedOpenGLResource* _resources;
};

class OpenGLContextManager : public QObject
{
	Q_OBJECT

public:

	~OpenGLContextManager() { qDeleteAll(_contexts); }

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
		connect(ctx, SIGNAL(aboutToBeDestroyed()), this, SLOT(aboutToDestroyContext()));
		return info;
	}

private Q_SLOTS:

	/// Is called when an OpenGL context is being destroyed.
    void aboutToDestroyContext();

private:

    QList<OpenGLContextInfo *> _contexts;
};

#include "SharedOpenGLResource.moc"

Q_GLOBAL_STATIC(OpenGLContextManager, qt_gl_context_manager);

void OpenGLContextManager::aboutToDestroyContext()
{
	OVITO_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

	QOpenGLContext* ctx = qobject_cast<QOpenGLContext*>(sender());
	OVITO_CHECK_POINTER(ctx);
	int index = 0;
	while(index < _contexts.size()) {
		OpenGLContextInfo* info = _contexts[index];
		if(info->_context == ctx) {
		    QList<QOpenGLContext*> shares = ctx->shareGroup()->shares();
			if(shares.size() >= 2) {
				// Transfer ownership to another context in the same sharing
				// group.  This may result in multiple QGLContextInfo objects
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

void SharedOpenGLResource::attachOpenGLResources()
{
    OVITO_ASSERT(_contextInfo == nullptr);
    OVITO_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    QOpenGLContext* context = QOpenGLContext::currentContext();
    OVITO_CHECK_POINTER(context);

    OpenGLContextManager* manager = qt_gl_context_manager();
    _contextInfo = manager->contextInfo(context);
    _next = _contextInfo->_resources;
    _prev = nullptr;
    if(_contextInfo->_resources)
    	_contextInfo->_resources->_prev = this;
    _contextInfo->_resources = this;
}

void SharedOpenGLResource::destroyOpenGLResources()
{
	if(!_contextInfo)
		return;

	OVITO_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());

    // Detach this resource from the context information block.
	if(_next) _next->_prev = _prev;
	if(_prev) _prev->_next = _next;
	else _contextInfo->_resources = _next;

	QOpenGLContext* ownerContext = _contextInfo->_context;
	QSurface* ownerSurface = _contextInfo->_surface;
    _contextInfo = nullptr;
    _next = _prev = nullptr;

    // Switch back to the owning context temporarily and delete the id.
	QOpenGLContext* currentContext = QOpenGLContext::currentContext();
	if(currentContext != ownerContext && !QOpenGLContext::areSharing(ownerContext, currentContext)) {
		QSurface* currentSurface = currentContext ? currentContext->surface() : nullptr;
		OVITO_ASSERT_MSG(ownerSurface != nullptr, "SharedOpenGLResource::destroyOpenGLResources()", "The QSurface associated with the OpenGL context has already been deleted.");
		ownerContext->makeCurrent(ownerSurface);
		freeOpenGLResources();
		if(currentContext)
			currentContext->makeCurrent(currentSurface);
		else
			ownerContext->doneCurrent();
	}
	else freeOpenGLResources();
}

};
