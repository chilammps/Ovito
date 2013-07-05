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

#ifndef __OVITO_PARTICLE_MODIFIER_H
#define __OVITO_PARTICLE_MODIFIER_H

#include <core/Core.h>
#include <core/scene/pipeline/Modifier.h>
#include <core/scene/objects/SceneObject.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/reference/CloneHelper.h>
#include <core/reference/RefTargetListener.h>

namespace Viz {

using namespace Ovito;

/*
 * Abstract base class for modifiers that operate on a system of particles.
 */
class ParticleModifier : public Modifier
{
protected:

	/// Constructor.
	ParticleModifier() {}

public:

	/// This modifies the input object.
	virtual ObjectStatus modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state) override;

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// Modifies the particle object. This function must be implemented by sub-classes
	/// do the modifier specific work. The time interval passed
	/// to the function should be reduced to the interval where the returned object is valid/constant.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) = 0;

#if 0
	/// Returns the input atoms. The returned object may not be modified.
	/// If you want to modify the AtomsObject then use the result() method instead.
	/// It will return a copy of the input object that may be modified.
	AtomsObject* input() const { OVITO_ASSERT(inputAtoms != NULL); return inputAtoms; }

	/// Creates a shallow copy of the input object on demand and returns it.
	AtomsObject* output();

	/// Completely replaces the output object with a new AtomsObject.
	void setOutput(AtomsObject* newOutput) { outputAtoms = newOutput; }

	/// Returns the standard channel with the given identifier from the output object.
	/// The requested data channel will be created or deep copied as needed.
	DataChannel* outputStandardChannel(DataChannel::DataChannelIdentifier which);

	/// Returns the standard channel with the given identifier from the input object.
	/// The returned channel may be NULL if it does not exist. Its contents
	/// may not be modified.
	DataChannel* inputStandardChannel(DataChannel::DataChannelIdentifier which) const;

	/// Returns the given standard channel from the input object.
	/// The returned channel may not be modified. If they input object does
	/// not contain the standard channel then an exception is thrown.
	DataChannel* expectStandardChannel(DataChannel::DataChannelIdentifier which) const;

	/// Returns the channel with the given name from the input object.
	/// The returned channel may not be modified. If they input object does
	/// not contain a channel with the given name then an exception is thrown.
	/// If there is a channel with the given name but with another data type then
	/// an exception is thrown too.
	DataChannel* expectCustomChannel(const QString& channelName, int channelDataType, size_t componentCount = 1) const;

	/// Returns the current ModifierApplication object for this modifier.
	ModifierApplication* modifierApplication() const { OVITO_ASSERT(modApp != NULL); return modApp; }

	/// Returns a clone helper object that should be used to create shallow and deep copies
	/// of the atoms object and its data.
	CloneHelper* cloneHelper() {
		if(!_cloneHelper) _cloneHelper.reset(new CloneHelper());
		return _cloneHelper.get();
	}

#endif

protected:

	/// The clone helper object that is used to create shallow and deep copies
	/// of the atoms object and its channels.
	QScopedPointer<CloneHelper> _cloneHelper;

	/// The current ModifierApplication object.
	ModifierApplication* _modApp;

private:

	Q_OBJECT
	OVITO_OBJECT
};

/**
 * Base class for properties editors for ParticleModifier derived classes.
 */
class ParticleModifierEditor : public PropertiesEditor
{
public:

	/// Constructor.
	ParticleModifierEditor() :
		_modifierStatusInfoIcon(":/atomviz/icons/modifier_status_info.png"),
		_modifierStatusWarningIcon(":/atomviz/icons/modifier_status_warning.png"),
		_modifierStatusErrorIcon(":/atomviz/icons/modifier_status_error.png")
	{
		connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(onContentsReplaced(RefTarget*)));
		connect(&_modAppListener, SIGNAL(notificationEvent(ReferenceEvent*)), this, SLOT(onModAppNotificationEvent(ReferenceEvent*)));
	}

	/// Returns a widget that displays a message sent by the modifier that
	/// states the outcome of the modifier evaluation. Derived classes of this
	/// editor base class can add the widget to their user interface.
	QWidget* statusLabel();

private Q_SLOTS:

	/// This handler is called when a new edit object has been loaded into the editor.
	void onContentsReplaced(RefTarget* newEditObject);

	/// This handler is called when the current ModifierApplication sends a notification event.
	void onModAppNotificationEvent(ReferenceEvent* event);

private:

	/// Updates the text of the result label.
	void updateStatusLabel(ModifierApplication* modApp);

	RefTargetListener _modAppListener;
	QPointer<QWidget> _statusLabel;
	QPointer<QLabel> _statusTextLabel;
	QPointer<QLabel> _statusIconLabel;

	QPixmap _modifierStatusInfoIcon;
	QPixmap _modifierStatusWarningIcon;
	QPixmap _modifierStatusErrorIcon;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_PARTICLE_MODIFIER_H
