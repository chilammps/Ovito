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

#ifndef __OVITO_PIPELINE_OBJECT_H
#define __OVITO_PIPELINE_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/DataObject.h>
#include "ModifierApplication.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief This is a data flow pipeline. It has a source object, which provides the input data,
 *        and a list of modifiers that act on the data.
 */
class OVITO_CORE_EXPORT PipelineObject : public DataObject
{
public:

	/// \brief Constructor that creates an empty pipeline.
	Q_INVOKABLE PipelineObject(DataSet* dataset);

	/// \brief Asks the object for the result of the geometry pipeline at the given time
	///        up to a given point in the modifier stack.
	/// \param time The animation at which the geometry pipeline should be evaluated.
	/// \param upToHere If \a upToHere is \c NULL then the complete modifier stack will be evaluated.
	///                 Otherwise only the modifiers in the pipeline before the given point will be applied to the
	///                 input object. \a upToHere must be one of the application objects returned by modifierApplications().
	/// \param including Specifies whether the last modifier given by \a upToHere will also be applied to the input object.
	/// \return The result object.
	PipelineFlowState evaluatePipeline(TimePoint time, ModifierApplication* upToHere, bool including);

	/// \brief Returns the object that provides the input data for this pipeline.
	DataObject* sourceObject() const { return _sourceObject; }

	/// \brief Replaces the object that provides the input data for this pipeline.
	/// \param sourceObject The new source object.
	/// \undoable
	void setSourceObject(DataObject* sourceObject) { _sourceObject = sourceObject; }

	/// \brief Returns the list of modifier applications.
	/// \return The list of applications of modifiers that make up the data flow pipeline.
	///         The modifiers in this list are applied to the input object in ascending order.
	const QVector<ModifierApplication*>& modifierApplications() const { return _modApps; }

	/// \brief Inserts a modifier into the data flow pipeline.
	/// \param modifier The modifier to be inserted.
	/// \param atIndex Specifies the position in the pipeline where the modifier should be inserted.
	///                It must be between zero and the number of existing modifier applications as returned by
	///                modifierApplications(). Modifiers are applied in ascending order, i.e., the modifier
	///                at index 0 is applied first to the input data.
	/// \return The ModifierApplication object that has been created for the usage of the modifier in this pipeline.
	/// \undoable
	ModifierApplication* insertModifier(Modifier* modifier, int atIndex);

	/// \brief Inserts a modifier application into the pipeline.
	/// \param modApp The modifier application to be inserted.
	/// \param atIndex Specifies the position in the pipeline where the modifier should be inserted.
	///                It must be between zero and the number of existing modifier applications as returned by
	///                modifierApplications(). Modifiers are applied in ascending order, i.e., the modifier
	///                at index 0 is applied first.
	/// \undoable
	void insertModifierApplication(ModifierApplication* modApp, int atIndex);

	/// \brief Removes a modifier from the geometry pipeline.
	/// \param app The ModifierApplication that should be removed. This must be one from the
	///            list returned by modifierApplications().
	/// \undoable
	void removeModifier(ModifierApplication* app);

	/////////////////////////////////////// from DataObject /////////////////////////////////////////

	/// Asks the object for the result of the geometry pipeline at the given time.
	virtual PipelineFlowState evaluate(TimePoint time) override {
		return evaluatePipeline(time, nullptr, true);
	}

protected:

	/// This method is called when a reference target changes.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Is called when a reference target has been added to a list reference field of this RefMaker.
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

	/// Is called when a reference target has been removed from a list reference field of this RefMaker.
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* oldTarget, int listIndex) override;

	/// Is called when the value of a reference field of this RefMaker changes.
	virtual void referenceReplaced(const PropertyFieldDescriptor& field, RefTarget* oldTarget, RefTarget* newTarget) override;

private:

	/// This function is called when a part of the pipeline (or its source) have changed.
	/// Notifies all modifiers starting at the given index that their input has changed.
	void modifierChanged(int changedIndex);

private:

	/// The object providing the input data that is processed by the modifiers.
	ReferenceField<DataObject> _sourceObject;

	/// The ordered list of modifiers that are applied to the input object.
	/// The modifiers are applied to the input object in the reverse order of this list.
	VectorReferenceField<ModifierApplication> _modApps;

	/// The state of the input object from the last evaluation of the pipeline.
	PipelineFlowState _lastInput;

	/// The cached results from the last pipeline evaluation.
	PipelineFlowState _cachedState;

	/// Indicates which pipeline stage has been stored in the cache.
	/// If the cache is empty, then this is -1.
	int _cachedIndex;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_sourceObject);
	DECLARE_VECTOR_REFERENCE_FIELD(_modApps);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_PIPELINE_OBJECT_H
