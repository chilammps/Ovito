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
 * \file ObjectNode.h
 * \brief Contains the definition of the Core::ObjectNode class.
 */

#ifndef __OVITO_OBJECTNODE_H
#define __OVITO_OBJECTNODE_H

#include <core/Core.h>
#include "SceneNode.h"
#include "objects/SceneObject.h"
#include "objects/Modifier.h"
#include "material/Material.h"

namespace Core {

/**
 * \brief A node in the scene tree that represents an object.
 *
 * \author Alexander Stukowski
 */
class CORE_DLLEXPORT ObjectNode : public SceneNode
{
	Q_OBJECT
public:

	/// \brief Constructs an object node that has no SceneObject associated with it.
	/// \param isLoading Indicates whether the object is being loaded from a file.
	///                  This parameter is only used by the object serialization system.
	Q_INVOKABLE ObjectNode(bool isLoading = false);

	/// \brief Constructs an object node that is associated with the given SceneObject.
	/// \param object The scene object that should be placed into the scene.
	explicit ObjectNode(SceneObject* object);

	/// \brief Returns whether this is an instance of the ObjectNode class.
	/// \return Returns alwyas \c true.
	virtual bool isObjectNode() const { return true; }

	/// \brief Allows direct access to this node's SceneObject.
	/// \return The object that represented by this node in the scene graph. Can be \c NULL.
	/// \sa setSceneObject()
	SceneObject* sceneObject() const { return _sceneObject; }

	/// \brief Sets the object of this node.
	/// \param obj The new object that evaluated by the ObjectNode and shown in the viewports.
	/// \undoable
	/// \sa sceneObject()
	void setSceneObject(SceneObject* obj) { _sceneObject = obj; }

	/// \brief Sets the object of this node.
	/// \param obj The new object that evaluated by the ObjectNode and shown in the viewports.
	///
	/// Same method as above but takes a smart pointer instead of a raw pointer.
	///
	/// \undoable
	/// \sa sceneObject()
	void setSceneObject(const SceneObject::SmartPtr& obj) { setSceneObject(obj.get()); }

	/// \brief Gets this node's local object transformation matrix.
	/// \return The transformation matrix that transforms from object space to local node space.
	///
	/// This object transform is applied to the SceneObject after it has been transformed by the
	/// node's transformation matrix.
	/// \sa setObjectTransform()
	const AffineTransformation& objectTransform() const { return _objectTransform; }

	/// \brief Sets this node's local object transformation matrix.
	/// \param tm The new object transformation matrix.
	/// \undoable
	/// \sa objectTransform()
	void setObjectTransform(const AffineTransformation& tm) { _objectTransform = tm; }

	/// \brief Evaluates the geometry pipeline of this object node at the given animation time.
	/// \param time The animation time at which the geoemtry pipeline of the node should be evaluated.
	/// \return The result of the evaluation.
	const PipelineFlowState& evalPipeline(TimeTicks time);

	/// \brief Returns the bounding box of the node's object in local coordinates.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the node's local coordinate system that contains
	///         the whole node geometry.
	/// \note The objectTransform() is already applied to the returned box.
	virtual Box3 localBoundingBox(TimeTicks time);

	/// \brief Performs a hit test on this node.
	/// \param time The animation time at which to perform the hit test.
	/// \param vp The viewport where the mouse was clicked.
	/// \param pickRegion The picking region to be used for hit testing.
	/// \return The distance of the hit from the viewer or HIT_TEST_NONE if no hit was found.
	virtual FloatType hitTest(TimeTicks time, Viewport* vp, const PickRegion& pickRegion);

	/// \brief Returns the material assigned to this node.
	/// \return The node's material or \c NULL if this node has no material.
	/// \sa setMaterial()
	Material* material() const { return _material; }

	/// \brief Assignes the given material to this node.
	/// \param m The new material to be assigned to the node. Can be \c NULL.
	/// \undoable
	/// \sa material()
	void setMaterial(Material* m) { _material = m; }

	/// \brief Assignes the given material to this node.
	/// \param m The new material to be assigned to the node. Can be \c NULL.
	///
	/// Same method as above but takes a smart pointer instead of a raw pointer.
	///
	/// \undoable
	/// \sa material()
	void setMaterial(const Material::SmartPtr& m) { setMaterial(m.get()); }

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

public:

	Q_PROPERTY(AffineTransformation objectTransform READ objectTransform WRITE setObjectTransform)
	Q_PROPERTY(Material* material READ material WRITE setMaterial)
	Q_PROPERTY(SceneObject* sceneObject READ sceneObject WRITE setSceneObject)

protected:

	/// This method is called when a referenced object has changed.
	virtual bool onRefTargetMessage(RefTarget* source, RefTargetMessage* msg);

	/// Is called when the value of a reference field of this object changes.
	virtual void onRefTargetReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget);

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream);

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream);

private:

	/// The object of this node.
	ReferenceField<SceneObject> _sceneObject;

	/// This node's local object transformation.
	PropertyField<AffineTransformation> _objectTransform;

	/// The material assigned to this node.
	ReferenceField<Material> _material;

	/// The cached result from the geometry pipeline evaluation.
	PipelineFlowState pipelineCache;

	/// This method invalidates the geometry pipeline cache of the object node.
	/// It will automatically be rebuilt on the next call to evalPipeline().
	void invalidatePipelineCache() { pipelineCache = PipelineFlowState(); }

	DECLARE_SERIALIZABLE_PLUGIN_CLASS(ObjectNode)
	DECLARE_REFERENCE_FIELD(_sceneObject)
	DECLARE_REFERENCE_FIELD(_material)
	DECLARE_PROPERTY_FIELD(_objectTransform)
};


};

#endif // __OVITO_OBJECTNODE_H
