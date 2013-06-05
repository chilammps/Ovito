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
 * \file ObjectNode.h
 * \brief Contains the definition of the Ovito::ObjectNode class.
 */

#ifndef __OVITO_OBJECT_NODE_H
#define __OVITO_OBJECT_NODE_H

#include <core/Core.h>
#include "SceneNode.h"
#include "objects/SceneObject.h"
#if 0
#include "objects/Modifier.h"
#endif

namespace Ovito {

/**
 * \brief A node in the scene tree that represents an object.
 */
class ObjectNode : public SceneNode
{
public:

	/// \brief Constructs an object node that is associated with the given SceneObject.
	/// \param object The scene object that should be placed into the scene.
	Q_INVOKABLE ObjectNode(SceneObject* object = NULL);

	/// \brief Returns whether this is an instance of the ObjectNode class.
	/// \return Always returns \c true.
	virtual bool isObjectNode() const { return true; }

	/// \brief Allows direct access to this node's SceneObject.
	/// \return The object that represented by this node in the scene graph. Can be \c NULL.
	SceneObject* sceneObject() const { return _sceneObject; }

	/// \brief Sets the object of this node.
	/// \param obj The new object that evaluated by the ObjectNode and shown in the viewports.
	/// \undoable
	void setSceneObject(SceneObject* obj) { _sceneObject = obj; }

	/// \brief Sets the object of this node.
	/// \param obj The new object that evaluated by the ObjectNode and shown in the viewports.
	///
	/// Same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void setSceneObject(const SceneObject::SmartPtr& obj) { setSceneObject(obj.get()); }

	/// \brief Gets this node's local object transformation matrix.
	/// \return The transformation matrix that transforms from object space to local node space.
	///
	/// This object transform is applied to the SceneObject after it has been transformed by the
	/// node's transformation matrix.
	const AffineTransformation& objectTransform() const { return _objectTransform; }

	/// \brief Sets this node's local object transformation matrix.
	/// \param tm The new object transformation matrix.
	/// \undoable
	void setObjectTransform(const AffineTransformation& tm) { _objectTransform = tm; }

	/// \brief Evaluates the geometry pipeline of this object node at the given animation time.
	/// \param time The animation time at which the geometry pipeline of the node should be evaluated.
	/// \return The result of the evaluation.
	const PipelineFlowState& evalPipeline(TimePoint time);

	/// \brief Returns the bounding box of the node's object in local coordinates.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the node's local coordinate system that contains
	///         the whole node geometry.
	/// \note The objectTransform() is already applied to the returned box.
	virtual Box3 localBoundingBox(TimePoint time) override;

#if 0
	/// \brief Performs a hit test on this node.
	/// \param time The animation time at which to perform the hit test.
	/// \param vp The viewport where the mouse was clicked.
	/// \param pickRegion The picking region to be used for hit testing.
	/// \return The distance of the hit from the viewer or HIT_TEST_NONE if no hit was found.
	virtual FloatType hitTest(TimePoint time, Viewport* vp, const PickRegion& pickRegion) override;
#endif

#if 0
	/// \brief Applies a modifier to the object node.
	/// \param mod The modifier to be applied.
	///
	/// The modifier is inserted into the geometry pipeline where it is put on top of the modifier stack.
	/// \undoable
	void applyModifier(Modifier* mod);

	/// \brief Applies a modifier to the object node.
	/// \param mod The modifier to be applied.
	///
	/// Same method as above but takes a smart pointer instead of a raw pointer.
	/// \undoable
	void applyModifier(const Modifier::SmartPtr& mod) {  applyModifier(mod.get()); }
#endif

public:

	Q_PROPERTY(AffineTransformation objectTransform READ objectTransform WRITE setObjectTransform)
	Q_PROPERTY(SceneObject* sceneObject READ sceneObject WRITE setSceneObject)

protected:

	/// This method is called when a referenced object has changed.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a reference field of this object changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

private:

	/// The object of this node.
	ReferenceField<SceneObject> _sceneObject;

	/// This node's local object transformation.
	PropertyField<AffineTransformation> _objectTransform;

	/// The cached result from the geometry pipeline evaluation.
	PipelineFlowState _pipelineCache;

	/// This method invalidates the geometry pipeline cache of the object node.
	/// It will automatically be rebuilt on the next call to evalPipeline().
	void invalidatePipelineCache() { _pipelineCache = PipelineFlowState(); }

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_sceneObject);
	DECLARE_PROPERTY_FIELD(_objectTransform);
};


};

#endif // __OVITO_OBJECT_NODE_H
