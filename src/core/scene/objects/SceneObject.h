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
 * \file SceneObject.h
 * \brief Contains the definition of the Ovito::SceneObject class.
 */

#ifndef __OVITO_SCENE_OBJECT_H
#define __OVITO_SCENE_OBJECT_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include <core/animation/TimeInterval.h>
#include <core/scene/pipeline/PipelineFlowState.h>

namespace Ovito {

class ObjectNode;				// defined in ObjectNode.h
class Viewport;					// defined in Viewport.h

/**
 * \brief Abstract base class for all objects in the scene.

 * A single SceneObject can be shared by multiple ObjectNode objects.
 */
class SceneObject : public RefTarget
{
protected:

	/// \brief Default constructor.
	SceneObject() {}

public:

	/// \brief Asks the object for its validity interval at the given time.
	/// \param time The animation time at which the validity interval should be computed.
	/// \return The maximum time interval that contains \a time and during which the object is valid.
	///
	/// When computing the validity interval of the object, an implementation of this method
	/// should take validity intervals of all sub-objects and sub-controller into account.
	virtual TimeInterval objectValidity(TimePoint time) = 0;

#if 0
	/// \brief Makes the object render itself into a viewport.
	/// \param time The animation time at which to render the object
	/// \param contextNode The node context used to render the object.
	/// \param vp The viewport to render in.
	///
	/// The viewport transformation is already set up when this method is called by the
	/// system. The object has to be rendered in the local object coordinate system.
	virtual void renderObject(TimeTicks time, ObjectNode* contextNode, Viewport* vp) = 0;
#endif

	/// \brief Asks the object whether it should appear in a rendered output image.
	/// \return \c true if the object is renderable.
	///         \c false otherwise.
	///
	/// The default implementation returns \c false.
	virtual bool isRenderable() { return false; }

	/// \brief Computes the bounding box of the object.
	/// \param time The animation time for which the bounding box should be computed.
	/// \param contextNode The scene node to which this scene object belongs to.
	/// \return The bounding box of the object in local object coordinates.
	virtual Box3 boundingBox(TimePoint time, ObjectNode* contextNode) = 0;

#if 0
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
#endif

	/// \brief Indicates whether this object should be surrounded by a selection marker in the viewports when it is selected.
	/// \return \c true to let the system render a selection marker around the object when it is selected.
	///
	/// The default implementation returns \c true.
	virtual bool showSelectionMarker() { return true; }

	/// \brief This asks the object whether it supports the conversion to another object type.
	/// \param objectClass The destination type. This must be a SceneObject derived class.
	/// \return \c true if this object can be converted to the requested type given by \a objectClass or any sub-class thereof.
	///         \c false if the conversion is not possible.
	///
	/// The default implementation returns \c true if the class \a objectClass is the source object type or any derived type.
	/// This is the trivial case: It requires no real conversion at all.
	///
	/// Sub-classes should override this method to allow the conversion to a MeshObject, for example.
	/// When overriding, the base implementation of this method should always be called.
	virtual bool canConvertTo(const OvitoObjectType& objectClass) {
		// Can always convert to itself.
		return this->getOOType().isDerivedFrom(objectClass);
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
	virtual OORef<SceneObject> convertTo(const OvitoObjectType& objectClass, TimePoint time) {
		// Trivial conversion.
		if(this->getOOType().isDerivedFrom(objectClass))
			return this;
		else
			return NULL;
	}

	/// \brief Lets the object convert itself to another object type.
	/// \param time The time at which to convert the object.
	///
	/// This is a wrapper of the function above using C++ templates.
	/// It just casts the conversion result to the given class.
	template<class T>
	OORef<T> convertTo(TimePoint time) {
		return static_object_cast<T>(convertTo(T::OOType, time));
	}

	/// \brief Asks the object for the result of the geometry pipeline at the given time.
	/// \param time The animation time at which the geometry pipeline is being evaluated.
	/// \return The pipeline flow state generated by this object.
	///
	/// The default implementation just returns the object itself as the evaluation result.
	virtual PipelineFlowState evalObject(TimePoint time) {
		return PipelineFlowState(this, objectValidity(time));
	}

	/// \brief Returns the number of input objects that are referenced by this scene object.
	/// \return The number of input objects that this object relies on.
	///
	/// The default implementation of this method returns 0.
	virtual int inputObjectCount() { return 0; }

	/// \brief Returns an input object of this scene object.
	/// \param index The index of the input object. This must be between 0 and inputObjectCount()-1.
	/// \return The requested input object. Can be \c NULL.
	virtual SceneObject* inputObject(int index) {
		OVITO_ASSERT_MSG(false, "SceneObject::inputObject", "This type of scene object has no input objects.");
		return nullptr;
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_SCENE_OBJECT_H
