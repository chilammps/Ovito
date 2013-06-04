///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
 * \file SceneObject.h
 * \brief Contains the definition of the Core::SceneObject class.
 */

#ifndef __OVITO_SCENEOBJECT_H
#define __OVITO_SCENEOBJECT_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/scene/animation/TimeInterval.h>
#include <core/viewport/PickRegion.h>
#include "PipelineFlowState.h"
#include "AttachedObjectRenderer.h"

namespace Core {

class ObjectNode;				// defined in ObjectNode.h
class Viewport;					// defined in Viewport.h
class Window3D;					// defined in Window3D.h
struct CameraViewDescription;	// defined in Viewport.h

/**
 * \brief Abstract base class for all objects in the scene.
 *
 * SceneObjects can be either procedural objects or modifier objects.
 * A single SceneObject can be shared by multiple ObjectNode objects.
 *
 * \author Alexander Stukowski
 * \sa ObjectNode
 */
class CORE_DLLEXPORT SceneObject : public RefTarget
{
	Q_OBJECT
	DECLARE_ABSTRACT_PLUGIN_CLASS(SceneObject)

protected:

	/// \brief Default constructor.
	/// \param isLoading Indicates whether the object is being loaded from a file.
	///                  This parameter is only used by the object serialization system.
	SceneObject(bool isLoading) : RefTarget(isLoading) {
#if 0
		INIT_PROPERTY_FIELD(SceneObject, _attachedRenderers);
#endif
	}

public:

	/// \brief Asks the object for its validity interval at the given time.
	/// \param time The animation time at which the validity interval should be computed.
	/// \return The maximum time interval that contains \a time and during which the object is valid.
	///
	/// When computing the validity interval of the object, the implementation of this method
	/// should take validity intervals of all sub-objects and sub-controller into account.
	virtual TimeInterval objectValidity(TimeTicks time) = 0;

	/// \brief Makes the object render itself into a viewport.
	/// \param time The animation time at which to render the object
	/// \param contextNode The node context used to render the object.
	/// \param vp The viewport to render in.
	///
	/// The viewport transformation is already set up when this method is called by the
	/// system. The object has to be rendered in the local object coordinate system.
	virtual void renderObject(TimeTicks time, ObjectNode* contextNode, Viewport* vp) = 0;

	/// \brief Asks the object whether it should appear in a rendered image.
	/// \return \c true if the object is renderable.
	///         \c false otherwise.
	///
	/// The default implementation returns \c false.
	virtual bool isRenderable() { return false; }

	/// \brief Renders the object in preview rendering mode using OpenGL.
	/// \param time The animation time at which to render the object
	/// \param view The camera projection.
	/// \param contextNode The node context used to render the object.
	/// \param imageWidth The width of the rendering buffer in pixels.
	/// \param imageHeight The height of the rendering buffer in pixels.
	/// \param glcontext The window that provides the OpenGL rendering context.
	/// \return \c false if the rendering has been aborted by the user; \c true otherwise.
	///
	/// The OpenGL object transformation is already set up when this method is called by the
	/// preview renderer. The object has to be rendered in the local object coordinate system.
	///
	/// \note This method is only called when isRenderable() returns \c true for this object.
	///
	/// \sa isRenderable()
	/// \sa PreviewRenderer
	virtual bool renderPreview(TimeTicks time, const CameraViewDescription& view, ObjectNode* contextNode, int imageWidth, int imageHeight, Window3D* glcontext) { return true; }

	/// \brief Computes the bounding box of the object.
	/// \param time The animation time for which the bounding box should be computed.
	/// \param contextNode The scene node to which this scene object belongs to.
	/// \return The bounding box of the object in local object coordinates.
	virtual Box3 boundingBox(TimeTicks time, ObjectNode* contextNode) = 0;

	/// \brief Performs a hit test on this object.
	/// \param time The animation at which hit testing should be done.
	/// \param vp The viewport in which hit testing should be performed.
	/// \param contextNode The scene nodes to which this scene object belongs to.
	/// \param pickRegion The picking region to be used for hit testing.
	/// \return The distance of the hit from the viewer or HIT_TEST_NONE if no hit was found.
	///
	/// The default implementation of this method uses the standard picking methods of the Window3D class.
	/// It enables picking mode for the Window3D, renders the SceneObject using renderObject(), and lets the
	/// Window3D decide whether the object was picked.
	///
	/// \sa Window3D::setPickingRegion()
	/// \sa intersectRay()
	virtual FloatType hitTest(TimeTicks time, Viewport* vp, ObjectNode* contextNode, const PickRegion& pickRegion);

	/// \brief Performs a ray intersection calculation.
	/// \param ray The ray to test for intersection with the object. Ray is specified in object space.
	/// \param time The animation at which to do the ray-object intersection test.
	/// \param contextNode The scene nodes to which this scene object belongs to (can be NULL).
	/// \param t If an intersection has been found, the method stores the distance of the intersection in this output parameter.
	/// \param normal If an intersection has been found, the method stores the surface normal at the point of intersection in this output parameter.
	/// \return True if the closest intersection has been found. False if no intersection has been found.
	///
	/// The default implementation always returns false.
	///
	/// \sa hitTest()
	virtual bool intersectRay(const Ray3& ray, TimeTicks time, ObjectNode* contextNode, FloatType& t, Vector3& normal) { return false; }

	/// \brief Indicates whether this object should be surrounded by a selection marker in the viewports when it is selected.
	/// \return \c true to let the system render a selection marker around the object when it is selected.
	///
	/// The default implementation just returns \c true and can be overridden by sub-classes.
	virtual bool showSelectionMarker() { return true; }

#if 0
	/// \brief Returns the list of renderer objects attached to this SceneObject.
	/// \return The list of attached renderer objects. They can renderer additional
	///         content into the viewports based on the data stored in the SceneObject.
	const QVector<AttachedObjectRenderer*>& attachedRenderers() const { return _attachedRenderers; }

	/// \brief Returns a specific renderer objects attached to this SceneObject.
	/// \return The first attached renderer object that can be cast to the given type (or NULL if no
	///         such renderer is attached).
	template<class RendererClass>
	RendererClass* attachedRenderer() const { return _attachedRenderers.firstOf<RendererClass>(); }

	/// \brief Attaches a renderer object to this SceneObject.
	/// \param renderer The renderer object to be attached.
	/// \undoable
	/// \sa attachedRenderers()
	/// \sa detachRenderer()
	void attachRenderer(AttachedObjectRenderer* renderObj) { CHECK_OBJECT_POINTER(renderObj); _attachedRenderers.push_back(renderObj); }

	/// \brief Removes a renderer object from this SceneObject's list of attached renderers.
	/// \param renderer The renderer object to be detached.
	/// \undoable
	/// \sa attachedRenderers()
	/// \sa attachRenderer()
	void detachRenderer(AttachedObjectRenderer* renderObj) {
		CHECK_OBJECT_POINTER(renderObj);
		int index = _attachedRenderers.indexOf(renderObj);
		OVITO_ASSERT(index >= 0);
		_attachedRenderers.remove(index);
	}

#endif

	/// \brief This asks the object whether it supports the conversion to another object type.
	/// \param objectClass The destination type. This must be a SceneObject derived class.
	/// \return \c true if this object can be converted to the requested type given by \a objectClass or any sub-class thereof.
	///         \c false if the conversion is not possible.
	///
	/// The default implementation returns \c true if the class \a objectClass is the source object type or any derived type.
	/// This is the trivial case: It requires no real conversion at all.
	///
	/// Sub-classes should override this method to allow the conversion to a MeshObject for example.
	/// When overriding, the base implementation of this method should always be called.
	///
	/// \sa convertTo()
	virtual bool canConvertTo(PluginClassDescriptor* objectClass) {
		// Can always convert to itself.
		if(this->pluginClassDescriptor()->isKindOf(objectClass))
			return true;
		else
			return false;
	}

	/// \brief Lets the object convert itself to another object type.
	/// \param objectClass The destination type. This must be a SceneObject derived class.
	/// \param time The time at which to convert the object.
	/// \return The newly created object or \c NULL if no conversion is possible.
	///
	/// Whether the object can be converted to the desired destination type can be checked in advance using
	/// the canConvertTo() method.
	///
	/// Sub-classes should override this method to allow the conversion to a MeshObject for example.
	/// When overriding, the base implementation of this method should always be called.
	///
	/// \sa canConvertTo()
	virtual SceneObject::SmartPtr convertTo(PluginClassDescriptor* objectClass, TimeTicks time) {
		// Try trivial conversion.
		if(this->pluginClassDescriptor()->isKindOf(objectClass))
			return this;
		else
			return NULL;
	}

	/// \brief Lets the object convert itself to another object type.
	/// \param time The time at which to convert the object.
	///
	/// This is the templated version of the function above.
	/// It just casts the conversion result to the given template parameter.
	template<class T>
	typename T::SmartPtr convertTo(TimeTicks time) {
		return static_object_cast<T>(convertTo(PLUGINCLASSINFO(T), time));
	}

	/// \brief Asks the object for the result of the geometry pipeline at the given time.
	/// \param time The animation time at which the geometry pipeline is being evaluated.
	/// \return The pipeline flow state generated by this object.
	///
	/// The default implementation just returns the object itself as the evaluation result.
	virtual PipelineFlowState evalObject(TimeTicks time) {
		return PipelineFlowState(this, objectValidity(time));
	}

	/// \brief Returns the number of input objects that are referenced by this scene object.
	/// \return The number of input objects that this object relies on.
	///
	/// The default implementation of this method returns 0.
	///
	/// \sa inputObject()
	virtual int inputObjectCount() { return 0; }

	/// \brief Returns an input object of this scene object.
	/// \param index The index of the input object. This must be between 0 and inputObjectCount()-1.
	/// \return The requested input object. Can be \c NULL.
	virtual SceneObject* inputObject(int index) {
		OVITO_ASSERT_MSG(false, "SceneObject::inputObject", "This type of scene object has no input object.");
		return NULL;
	}

private:

#if 0
	/// List of renderer objects attached to this SceneObject.
	/// They will be invoked when the SceneObject is being rendered.
	VectorReferenceField<AttachedObjectRenderer> _attachedRenderers;

	DECLARE_VECTOR_REFERENCE_FIELD(_attachedRenderers)
#endif
};


};

#endif // __OVITO_SCENEOBJECT_H
