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

#ifndef __OVITO_OBJECT_NODE_H
#define __OVITO_OBJECT_NODE_H

#include <core/Core.h>
#include "SceneNode.h"
#include "objects/DataObject.h"
#include "objects/DisplayObject.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief A node in the scene that represents an object.
 */
class OVITO_CORE_EXPORT ObjectNode : public SceneNode
{
public:

	/// \brief Constructs an object node.
	Q_INVOKABLE ObjectNode(DataSet* dataset);

	/// \brief Returns this node's data provider, i.e. the object
	///        that generates the data displayed by this scene node.
	DataObject* dataProvider() const { return _dataProvider; }

	/// \brief Sets the data provider object of this node.
	/// \param pipeline An object that generates data, which will be displayed by this ObjectNode.
	/// \undoable
	void setDataProvider(DataObject* dataProvider) { _dataProvider = dataProvider; }

	/// \brief Returns the data source of this node's pipeline, i.e., the object that provides the
	///        input data that enters the pipeline.
	DataObject* sourceObject() const;

	/// \brief Evaluates the data flow pipeline of this object node at the given animation time.
	/// \param time The animation time at which the pipeline of the node should be evaluated.
	/// \return The output of the pipeline.
	const PipelineFlowState& evalPipeline(TimePoint time);

	/// \brief This function blocks execution until the node's modification
	///        pipeline has been fully evaluated.
	/// \param time The animation time at which the modification pipeline should be evaluated.
	/// \param message The text to be shown to the user while waiting.
	/// \param progressDialog An existing progress dialog to use to show the message.
	///                       If NULL, the function will show its own dialog box.
	/// \return true on success; false if the operation has been canceled by the user.
	bool waitUntilReady(TimePoint time, const QString& message, QProgressDialog* progressDialog = nullptr);

	/// \brief Applies a modifier by appending it to the end of the node's data pipeline.
	/// \param mod The modifier to be inserted into the data flow pipeline.
	/// \undoable
	Q_INVOKABLE void applyModifier(Modifier* mod);

	/// \brief Returns the list of display objects that are responsible for displaying
	///        the node's data in the viewports.
	const QVector<DisplayObject*>& displayObjects() const { return _displayObjects; }

	/// \brief Returns the bounding box of the node's object in local coordinates.
	/// \param time The time at which the bounding box should be computed.
	/// \return An axis-aligned box in the node's local coordinate system that fully contains
	///         the node's object.
	virtual Box3 localBoundingBox(TimePoint time) override;

	/// \brief Renders the node's object.
	/// \param time Specifies the animation frame to render.
	/// \param renderer The renderer that should be used to render graphics primitives.
	void render(TimePoint time, SceneRenderer* renderer);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override;

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

	/// The object which generates the data to be displayed by this ObjectNode.
	ReferenceField<DataObject> _dataProvider;

	/// The cached results from the last data pipeline evaluation.
	PipelineFlowState _pipelineCache;

	/// The list of display objects that are responsible for displaying
	/// the node's data in the viewports.
	VectorReferenceField<DisplayObject> _displayObjects;

	/// This method invalidates the data pipeline cache of the object node.
	void invalidatePipelineCache() {
		// Reset data cache.
		_pipelineCache.clear();
		// Also mark the cached bounding box of this scene node as invalid.
		invalidateBoundingBox();
	}

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_dataProvider);
	DECLARE_VECTOR_REFERENCE_FIELD(_displayObjects);
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_OBJECT_NODE_H
