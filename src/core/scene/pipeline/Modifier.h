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

#ifndef __OVITO_MODIFIER_H
#define __OVITO_MODIFIER_H

#include <core/Core.h>
#include <core/reference/RefTarget.h>
#include "PipelineFlowState.h"
#include "PipelineStatus.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene)

/**
 * \brief Base class for algorithms that modify an object or data in some way.
 *
 * A Modifier is inserted into the modification pipeline of a scene node
 * using ObjectNode::applyModifier() or PipelineObject::insertModifier().
 *
 * \sa PipelineObject
 * \sa ModifierApplication
 */
class OVITO_CORE_EXPORT Modifier : public RefTarget
{
protected:

	/// \brief Constructor.
	Modifier(DataSet* dataset);

public:

	/// \brief This modifies the input object in a specific way.
	/// \param[in] time The animation at which the modifier is applied.
	/// \param[in] modApp The application object for this modifier. It describes this particular usage of the
	///               modifier in the geometry pipeline.
	/// \param[in,out] state The object flowing down the geometry pipeline. It contains the input object
	///                      when the method is called and is filled with the resulting object by the method.
	virtual PipelineStatus modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state) = 0;

	/// \brief Asks the modifier for its validity interval at the given time.
	/// \param time The animation at which the validity interval should be computed.
	/// \return The maximum time interval that contains \a time and during which the modifier's
	///         parameters do not change. This does not include the validity interval of the
	///         modifier's input object.
	virtual TimeInterval modifierValidity(TimePoint time);

	/// \brief Returns a structure that describes the current status of the modifier.
	///
	/// The default implementation of this method returns PipelineStatus::StatusType::Success.
	///
	/// A modifier should generate a ReferenceEvent::ObjectStatusChanged event when its status changes.
	virtual PipelineStatus status() const { return PipelineStatus(); }

	/// \brief Lets the modifier render itself into a viewport.
	/// \param time The animation time at which to render the modifier.
	/// \param contextNode The node context used to render the modifier.
	/// \param modApp The modifier application specifies the particular application of this modifier in a geometry pipeline.
	/// \param renderer The scene renderer to use.
	/// \param renderOverlay Specifies the rendering pass. The method is called twice by the system: First with renderOverlay==false
	///                      to render any 3d representation of the modifier, and a second time with renderOverlay==true to render
	///                      any overlay graphics on top of the 3d scene.
	///
	/// The viewport transformation is already set up when this method is called
	/// The default implementation does nothing.
	virtual void render(TimePoint time, ObjectNode* contextNode, ModifierApplication* modApp, SceneRenderer* renderer, bool renderOverlay) {}

	/// \brief Computes the bounding box of the visual representation of the modifier.
	/// \param time The animation time at which the bounding box should be computed.
	/// \param contextNode The scene node to which this modifier was applied.
	/// \param modApp The modifier application specifies the particular application of this modifier in a geometry pipeline.
	/// \return The bounding box of the modifier in local object coordinates.
	///
	/// The default implementation returns an empty bounding box.
	virtual Box3 boundingBox(TimePoint time, ObjectNode* contextNode, ModifierApplication* modApp) { return Box3(); }

	/// \brief Returns the list of applications of this modifier in pipelines.
	/// \return The list of ModifierApplication objects that describe the particular applications of this Modifier.
	///
	/// One and the same modifier instance can be applied in several geometry pipelines.
	/// Each application of the modifier instance is associated with a instance of the ModifierApplication class.
	/// This method can be used to determine all applications of this Modifier instance.
	QVector<ModifierApplication*> modifierApplications() const;

	/// \brief Returns the input object of this modifier for each application of the modifier.
	/// \param time The animation for which the geometry pipelines should be evaluated.
	/// \return A container that contains for each application of this modifier the
	///         state of the geometry pipeline evaluation up to the modifier's application.
	///
	/// This method evaluates the geometry pipeline up this modifier. It can be used to work with
	/// the input objects outside of a normal call to modifyObject().
	///
	/// \note This method might return empty result objects in some cases when the modifier stack
	///       cannot be evaluated because of an invalid modifier.
	QVector<QPair<ModifierApplication*, PipelineFlowState>> getModifierInputs() const;

	/// \brief Returns the input object of the modifier assuming that it has been applied only in a single geometry pipeline.
	/// \return The object that comes out of the geometry pipeline when it is evaluated up the application of this modifier.
	///
	/// This is the same function as above but now using the current animation time as
	/// evaluation time and only returning the input object for the first application
	/// of this modifier.
	///
	/// This method can be used to work with the input object outside of a normal call to modifyObject().
	PipelineFlowState getModifierInput() const;

	/// \brief Returns whether this modifier is currently enabled.
	/// \return \c true if it is currently enabled, i.e. applied.
	///         \c false if it is disabled and skipped in the geometry pipeline.
	bool isEnabled() const { return _isEnabled; }

	/// \brief Enables or disables this modifier.
	/// \param enabled Controls the state of the modifier.
	///
	/// A disabled modifier is skipped in the geometry pipeline
	/// and is not applied to the input object.
	///
	/// \undoable
	void setEnabled(bool enabled) { _isEnabled = enabled; }

	/// \brief Asks the modifier whether it can be applied to the given input data.
	/// \param input The pipeline state at the point of the pipeline where the modifier is going to be inserted.
	/// \return true if the modifier can operate on the provided input data; false otherwise.
	///
	/// This method is used to filter the list of available modifiers. The default implementation returns false.
	virtual bool isApplicableTo(const PipelineFlowState& input) { return false; }

protected:

	/// \brief This method is called by the system when the modifier has been inserted into a PipelineObject.
	/// \param pipeline The PipelineObject into which the modifier has been inserted.
	/// \param modApp The ModifierApplication object that has been created for this modifier.
	virtual void initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp) {}

	/// \brief Informs the modifier that its input has changed.
	/// \param modApp The application of this modifier in the modification pipeline.
	////
	/// This method is called by the system when the upstream modification pipeline has changed.
	/// This allows the modifier to throw away any cached results so that a re-computation is triggered the
	/// next time the modification pipeline is evaluated.
	///
	/// The default implementation does nothing.
	virtual void upstreamPipelineChanged(ModifierApplication* modApp) {}

private:

	/// Flag that indicates whether the modifier is enabled.
	PropertyField<bool, bool, ReferenceEvent::TargetEnabledOrDisabled> _isEnabled;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_isEnabled);

	friend class PipelineObject;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

Q_DECLARE_METATYPE(Ovito::Modifier*);
Q_DECLARE_TYPEINFO(Ovito::Modifier*, Q_MOVABLE_TYPE);

#endif // __OVITO_MODIFIER_H
