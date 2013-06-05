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
 * \file PipelineObject.h
 * \brief Contains the definition of the Ovito::PipelineObject class.
 */

#ifndef __OVITO_PIPELINE_OBJECT_H
#define __OVITO_PIPELINE_OBJECT_H

#include <core/Core.h>
#include "SceneObject.h"
#include "ModifierApplication.h"

namespace Ovito {

/**
 * \brief This scene object type takes an input object and applies a list of modifiers to it.
 */
class PipelineObject : public SceneObject
{
public:

	/// \brief Default constructor that creates an empty object without input.
	Q_INVOKABLE PipelineObject();

	/// \brief Asks the object for the result of the geometry pipeline at the given time
	///        up to a given point in the modifier stack.
	/// \param time The animation at which the geometry pipeline should be evaluated.
	/// \param upToHere If \a upToHere is \c NULL then the complete modifier stack will be evaluated.
	///                 Otherwise only the modifiers in the pipeline before the given point will be applied to the
	///                 input object. \a upToHere must be one of the application objects returned by modifierApplications().
	/// \param including Specifies whether the last modifier given by \a upToHere will also be applied to the input object.
	/// \return The result object.
	PipelineFlowState evalObject(TimePoint time, ModifierApplication* upToHere, bool including);

	/// \brief Returns the input object of this modified object.
	SceneObject* inputObject() const { return _inputObject; }

	/// \brief Sets the input object for the geometry pipeline.
	/// \param inputObject The new input object to which the modifiers will be applied.
	/// \undoable
	void setInputObject(SceneObject* inputObject) { _inputObject = inputObject; }

	/// \brief Sets the input object for the modifiers.
	/// \param inputObject The new input object to which the modifiers will be applied.
	/// \undoable
	void setInputObject(const OORef<SceneObject>& inputObject) { setInputObject(inputObject.get()); }

	/// \brief Returns the list of modifier applications.
	/// \return The list of applications of modifiers that make up the geometry pipeline.
	///         The modifiers in this list are applied to the input object in ascending order.
	const QVector<ModifierApplication*>& modifierApplications() const { return _modApps; }

	/// \brief Inserts a modifier into the geometry pipeline.
	/// \param modifier The modifier to be inserted.
	/// \param atIndex Specifies the position in the geometry pipeline where the modifier should be applied.
	///                It must be between zero and the number of existing modifier applications as returned by
	///                modifierApplications(). Modifiers are applied in ascending order, i.e. the modifier
	///                at index 0 is applied first.
	/// \return The application object that has been created for the usage of the modifier instance in this
	///         geometry pipeline.
	/// \undoable
	ModifierApplication* insertModifier(Modifier* modifier, int atIndex);

	/// \brief Inserts a modifier application into the internal list.
	/// \param modApp The modifier application to be inserted.
	/// \param atIndex Specifies the position in the geometry pipeline where the modifier should be applied.
	///                It must be between zero and the number of existing modifier applications as returned by
	///                modifierApplications(). Modifiers are applied in ascending order, i.e. the modifier
	///                at index 0 is applied first.
	/// \undoable
	void insertModifierApplication(ModifierApplication* modApp, int atIndex);

	/// \brief Removes the given modifier application from the geometry pipeline.
	/// \param app The application of a modifier instance that should be removed. This must be one from the
	///            list returned by modifierApplications().
	/// \undoable
	void removeModifier(ModifierApplication* app);

	/////////////////////////////////////// from SceneObject /////////////////////////////////////////

	/// Asks the object for its validity interval at the given time.
	virtual TimeInterval objectValidity(TimePoint time) override;

#if 0
	/// Render the object into the viewport.
	virtual void renderObject(TimePoint time, ObjectNode* contextNode, Viewport* vp);
#endif

	/// Returns the bounding box of the object in local object coordinates.
	virtual Box3 boundingBox(TimePoint time, ObjectNode* contextNode) override;

	/// Asks the object for the result of the geometry pipeline at the given time.
	virtual PipelineFlowState evalObject(TimePoint time) override {
		return evalObject(time, nullptr, true);
	}

	/// Returns the number of input objects that are referenced by this scene object.
	virtual int inputObjectCount() override { return 1; }

	/// Returns the input object of this scene object.
	virtual SceneObject* inputObject(int index) override { return _inputObject; }

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

	/// Is called when a reference target has been added to a list reference field of this RefMaker.
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

	/// Is called when a reference target has been removed from a list reference field of this RefMaker.
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) override;

private:

	/// Notifies all modifiers from the given index on that their input has changed.
	void notifyModifiersInputChanged(int changedIndex);

	/// This method invalidates the internal geometry pipeline cache of the PipelineObject.
	void invalidatePipelineCache() {
		_pipelineCache = PipelineFlowState();
		_pipelineCacheIndex = -1;
	}

private:

	/// The input object that is modified by the modifiers.
	ReferenceField<SceneObject> _inputObject;

	/// The ordered list of modifiers that are applied to the input object.
	/// The modifiers are applied to the input object in the reverse order of this list.
	VectorReferenceField<ModifierApplication> _modApps;

	/// The cached result from the geometry pipeline evaluation.
	PipelineFlowState _pipelineCache;

	/// The pipeline stage that is in the cache.
	/// If the pipeline cache is empty then this is -1.
	int _pipelineCacheIndex;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_inputObject)
	DECLARE_VECTOR_REFERENCE_FIELD(_modApps)
};

};

#endif // __OVITO_PIPELINE_OBJECT_H
