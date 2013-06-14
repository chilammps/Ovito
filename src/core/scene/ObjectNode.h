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
#include "display/DisplayObject.h"

namespace Ovito {

class Modifier;			// defined in Modifier.h

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
	void setSceneObject(const OORef<SceneObject>& obj) { setSceneObject(obj.get()); }

	/// \brief Evaluates the geometry pipeline of this object node at the given animation time.
	/// \param time The animation time at which the geometry pipeline of the node should be evaluated.
	/// \return The result of the evaluation.
	const PipelineFlowState& evalPipeline(TimePoint time);

	/// \brief Returns the list of display objects that are responsible for displaying
	///        the node's scene object in the viewports.
	const QVector<DisplayObject*>& displayObjects() const { return _displayObjects; }

	/// \brief Returns the bounding box of the node's object in local coordinates.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the node's local coordinate system that contains
	///         the whole node geometry.
	virtual Box3 localBoundingBox(TimePoint time) override;

	/// \brief Renders the node's scene objects.
	/// \param time Specifies the animation frame to render.
	/// \param renderer The renderer that should be called by this method to display geometry.
	void render(TimePoint time, SceneRenderer* renderer);

#if 0
	/// \brief Performs a hit test on this node.
	/// \param time The animation time at which to perform the hit test.
	/// \param vp The viewport where the mouse was clicked.
	/// \param pickRegion The picking region to be used for hit testing.
	/// \return The distance of the hit from the viewer or HIT_TEST_NONE if no hit was found.
	virtual FloatType hitTest(TimePoint time, Viewport* vp, const PickRegion& pickRegion) override;
#endif

	/// \brief Applies a modifier to the object node.
	/// \param mod The modifier to be applied.
	///
	/// The modifier is inserted into the geometry pipeline where it is put on top of the modifier stack.
	/// \undoable
	void applyModifier(Modifier* mod);

	/// \brief Applies a modifier to the object node.
	/// \param mod The modifier to be applied.
	///
	/// Same method as above, but this one accepts a smart pointer.
	/// \undoable
	void applyModifier(const OORef<Modifier>& mod) {  applyModifier(mod.get()); }

public:

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

	/// The cached result from the last geometry pipeline evaluation.
	PipelineFlowState _pipelineCache;

	/// The list of display objects that are responsible for displaying
	/// the node's scene object in the viewports.
	VectorReferenceField<DisplayObject> _displayObjects;

	/// This method invalidates the geometry pipeline cache of the object node.
	/// This will automatically rebuild the cache on the next call to evalPipeline();
	void invalidatePipelineCache();

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_sceneObject);
	DECLARE_VECTOR_REFERENCE_FIELD(_displayObjects);
};


};

#endif // __OVITO_OBJECT_NODE_H
