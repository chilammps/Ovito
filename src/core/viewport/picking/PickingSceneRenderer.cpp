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
#include <core/viewport/ViewportWindow.h>
#include <core/viewport/Viewport.h>
#include <core/rendering/RenderSettings.h>
#include "PickingSceneRenderer.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(View) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

IMPLEMENT_OVITO_OBJECT(Core, PickingSceneRenderer, ViewportSceneRenderer);

/******************************************************************************
* This method is called just before renderFrame() is called.
******************************************************************************/
void PickingSceneRenderer::beginFrame(TimePoint time, const ViewProjectionParameters& params, Viewport* vp)
{
	// Get the viewport's window.
	ViewportWindow* vpWindow = vp->viewportWindow();
	if(!vpWindow)
		throw Exception(tr("Viewport window has not been created."));
	if(!vpWindow->isExposed())
		throw Exception(tr("Viewport window is not exposed."));

	// Get OpenGL context.
	QOpenGLContext* context = vpWindow->glcontext();
	if(!context || !context->isValid())
		throw Exception(tr("Viewport OpenGL context has not been created."));

	// Before making our GL context current, remember the old context that
	// is currently active so we can restore it when we are done.
	_oldContext = QOpenGLContext::currentContext();
	_oldSurface = _oldContext ? _oldContext->surface() : nullptr;

	// Make GL context current.
	if(!context->makeCurrent(vpWindow))
		throw Exception(tr("Failed to make OpenGL context current."));

	// Create OpenGL framebuffer.
	QSize size = vp->size();
	QOpenGLFramebufferObjectFormat framebufferFormat;
	framebufferFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
	_framebufferObject.reset(new QOpenGLFramebufferObject(size.width(), size.height(), framebufferFormat));
	// Clear OpenGL error state.
	while(glGetError() != GL_NO_ERROR);
	if(!_framebufferObject->isValid())
		throw Exception(tr("Failed to create OpenGL framebuffer object for offscreen rendering."));

	// Bind OpenGL buffer.
	if(!_framebufferObject->bind())
		throw Exception(tr("Failed to bind OpenGL framebuffer object for offscreen rendering."));

	ViewportSceneRenderer::beginFrame(time, params, vp);

	// Setup GL viewport.
	OVITO_CHECK_OPENGL(glViewport(0, 0, size.width(), size.height()));
	OVITO_CHECK_OPENGL(glClearColor(0, 0, 0, 0));
}

/******************************************************************************
* Renders the current animation frame.
******************************************************************************/
bool PickingSceneRenderer::renderFrame(FrameBuffer* frameBuffer, QProgressDialog* progress)
{
	// Clear previous object records.
	reset();

	// Let the base class do the main rendering work.
	if(!ViewportSceneRenderer::renderFrame(frameBuffer, progress))
		return false;

	// Clear OpenGL error state, so we start fresh for the glReadPixels() call below.
	while(glGetError() != GL_NO_ERROR);

	// Fetch rendered image from OpenGL framebuffer.
	QSize size = _framebufferObject->size();
	_image = QImage(size, QImage::Format_ARGB32);
	// Try GL_BGRA pixel format first. If not supported, use GL_RGBA instead and convert back to GL_BGRA.
	glReadPixels(0, 0, size.width(), size.height(), GL_BGRA, GL_UNSIGNED_BYTE, _image.bits());
	if(glGetError() != GL_NO_ERROR) {
		glReadPixels(0, 0, size.width(), size.height(), GL_RGBA, GL_UNSIGNED_BYTE, _image.bits());
		_image = _image.rgbSwapped();
	}
	OVITO_REPORT_OPENGL_ERRORS();

	// Also acquire OpenGL depth buffer data.
	// The depth information is used to compute the XYZ coordinate of the point under the mouse cursor.
	_depthBufferBits = glformat().depthBufferSize();
	if(_depthBufferBits == 16) {
		_depthBuffer.reset(new quint8[size.width() * size.height() * sizeof(GLushort)]);
		OVITO_CHECK_OPENGL(glReadPixels(0, 0, size.width(), size.height(), GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, _depthBuffer.get()));
	}
	else if(_depthBufferBits == 24) {
		_depthBuffer.reset(new quint8[size.width() * size.height() * sizeof(GLuint)]);
		while(glGetError() != GL_NO_ERROR);
		glReadPixels(0, 0, size.width(), size.height(), GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, _depthBuffer.get());
		if(glGetError() != GL_NO_ERROR) {
			OVITO_CHECK_OPENGL(glReadPixels(0, 0, size.width(), size.height(), GL_DEPTH_COMPONENT, GL_FLOAT, _depthBuffer.get()));
			_depthBufferBits = 0;
		}
	}
	else if(_depthBufferBits == 32) {
		_depthBuffer.reset(new quint8[size.width() * size.height() * sizeof(GLuint)]);
		OVITO_CHECK_OPENGL(glReadPixels(0, 0, size.width(), size.height(), GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, _depthBuffer.get()));
	}
	else {
		_depthBuffer.reset(new quint8[size.width() * size.height() * sizeof(GLfloat)]);
		OVITO_CHECK_OPENGL(glReadPixels(0, 0, size.width(), size.height(), GL_DEPTH_COMPONENT, GL_FLOAT, _depthBuffer.get()));
		_depthBufferBits = 0;
	}

	return true;
}

/******************************************************************************
* This method is called after renderFrame() has been called.
******************************************************************************/
void PickingSceneRenderer::endFrame()
{
	endPickObject();
	_framebufferObject.reset();
	ViewportSceneRenderer::endFrame();

	// Reactivate old GL context.
	if(_oldSurface && _oldContext)
		_oldContext->makeCurrent(_oldSurface);
	else {
		QOpenGLContext* context = QOpenGLContext::currentContext();
		if(context) context->doneCurrent();
	}
	_oldContext = nullptr;
	_oldSurface = nullptr;
}

/******************************************************************************
* Resets the internal state of the picking renderer and clears the stored object records.
******************************************************************************/
void PickingSceneRenderer::reset()
{
	_objects.clear();
	endPickObject();
#if 1
	_currentObject.baseObjectID = 1;
#else
	// This can be enabled during debugging to avoid alpha!=1 pixels in the picking render buffer.
	_currentObject.baseObjectID = 0xEF000000;
#endif
	_image = QImage();
}

/******************************************************************************
* When picking mode is active, this registers an object being rendered.
******************************************************************************/
quint32 PickingSceneRenderer::beginPickObject(ObjectNode* objNode, ObjectPickInfo* pickInfo)
{
	OVITO_ASSERT(objNode != nullptr);
	OVITO_ASSERT(isPicking());

	_currentObject.objectNode = objNode;
	_currentObject.pickInfo = pickInfo;
	return _currentObject.baseObjectID;
}

/******************************************************************************
* Registers a range of sub-IDs belonging to the current object being rendered.
******************************************************************************/
quint32 PickingSceneRenderer::registerSubObjectIDs(quint32 subObjectCount)
{
	OVITO_ASSERT_MSG(_currentObject.objectNode, "PickingSceneRenderer::registerSubObjectIDs()", "You forgot to register the current object via beginPickObject().");

	quint32 baseObjectID = _currentObject.baseObjectID;
	_objects.push_back(_currentObject);
	_currentObject.baseObjectID += subObjectCount;
	return baseObjectID;
}

/******************************************************************************
* Call this when rendering of a pickable object is finished.
******************************************************************************/
void PickingSceneRenderer::endPickObject()
{
	_currentObject.objectNode = nullptr;
	_currentObject.pickInfo = nullptr;
}

/******************************************************************************
* Returns the object record and the sub-object ID for the object at the given pixel coordinates.
******************************************************************************/
std::tuple<const PickingSceneRenderer::ObjectRecord*, quint32> PickingSceneRenderer::objectAtLocation(const QPoint& pos) const
{
	if(!_image.isNull()) {
		if(pos.x() >= 0 && pos.x() < _image.width() && pos.y() >= 0 && pos.y() < _image.height()) {
			QPoint mirroredPos(pos.x(), _image.height() - 1 - pos.y());
			QRgb pixel = _image.pixel(mirroredPos);
			quint32 red = qRed(pixel);
			quint32 green = qGreen(pixel);
			quint32 blue = qBlue(pixel);
			quint32 alpha = qAlpha(pixel);
			quint32 objectID = red + (green << 8) + (blue << 16) + (alpha << 24);
			const ObjectRecord* objRecord = lookupObjectRecord(objectID);
			if(objRecord)
				return std::make_tuple(objRecord, objectID - objRecord->baseObjectID);
		}
	}
	return std::tuple<const PickingSceneRenderer::ObjectRecord*, quint32>(nullptr, 0);
}

/******************************************************************************
* Given an object ID, looks up the corresponding record.
******************************************************************************/
const PickingSceneRenderer::ObjectRecord* PickingSceneRenderer::lookupObjectRecord(quint32 objectID) const
{
	if(objectID == 0 || _objects.empty())
		return nullptr;

	for(auto iter = _objects.begin(); iter != _objects.end(); iter++) {
		if(iter->baseObjectID > objectID) {
			OVITO_ASSERT(iter != _objects.begin());
			OVITO_ASSERT(objectID >= (iter-1)->baseObjectID);
			return &*(iter-1);
		}
	}

	OVITO_ASSERT(objectID >= _objects.back().baseObjectID);
	return &_objects.back();
}

/******************************************************************************
* Returns the Z-value at the given window position.
******************************************************************************/
FloatType PickingSceneRenderer::depthAtPixel(const QPoint& pos) const
{
	if(!_image.isNull() && _depthBuffer) {
		int w = _image.width();
		int h = _image.height();
		if(pos.x() >= 0 && pos.x() < w && pos.y() >= 0 && pos.y() < h) {
			QPoint mirroredPos(pos.x(), _image.height() - 1 - pos.y());
			if(_image.pixel(mirroredPos) != 0) {
				if(_depthBufferBits == 16) {
					GLushort bval = reinterpret_cast<const GLushort*>(_depthBuffer.get())[(mirroredPos.y()) * w + pos.x()];
					return (FloatType)bval / FloatType(65535.0);
				}
				else if(_depthBufferBits == 24) {
					GLuint bval = reinterpret_cast<const GLuint*>(_depthBuffer.get())[(mirroredPos.y()) * w + pos.x()];
					return (FloatType)((bval>>8) & 0x00FFFFFF) / FloatType(16777215.0);
				}
				else if(_depthBufferBits == 32) {
					GLuint bval = reinterpret_cast<const GLuint*>(_depthBuffer.get())[(mirroredPos.y()) * w + pos.x()];
					return (FloatType)bval / FloatType(4294967295.0);
				}
				else if(_depthBufferBits == 0) {
					return reinterpret_cast<const GLfloat*>(_depthBuffer.get())[(mirroredPos.y()) * w + pos.x()];
				}
			}
		}
	}
	return 0;
}

/******************************************************************************
* Returns the world space position corresponding to the given screen position.
******************************************************************************/
Point3 PickingSceneRenderer::worldPositionFromLocation(const QPoint& pos) const
{
	FloatType zvalue = depthAtPixel(pos);
	if(zvalue != 0) {
		Point3 ndc(
				(FloatType)pos.x() / _image.width() * 2.0 - 1.0,
				1.0 - (FloatType)pos.y() / _image.height() * 2.0,
				zvalue * 2.0 - 1.0);
		Point3 worldPos = projParams().inverseViewMatrix * (projParams().inverseProjectionMatrix * ndc);
		return worldPos;
	}
	return Point3::Origin();
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
