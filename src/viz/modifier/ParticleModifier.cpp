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

#include <core/Core.h>
#include <core/scene/pipeline/ModifierApplication.h>
#include "ParticleModifier.h"

namespace Viz {

IMPLEMENT_OVITO_OBJECT(Viz, ParticleModifier, Modifier)
IMPLEMENT_OVITO_OBJECT(Viz, ParticleModifierEditor, PropertiesEditor)

/******************************************************************************
* This modifies the input object.
******************************************************************************/
ObjectStatus ParticleModifier::modifyObject(TimePoint time, ModifierApplication* modApp, PipelineFlowState& state)
{
#if 0
	// This method is not re-entrant. If the modifier is currently being evaluated and
	// this method is called again then we are not able to process the request.
	if(inputAtoms)
		return EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, tr("Cannot handle re-entrant modifier calls."));

	// Prepare internal fields.
	outputAtoms = NULL;
	this->modApp = modApp;
	EvaluationStatus status;

	try {
		inputAtoms = dynamic_object_cast<AtomsObject>(state.result());
		if(!inputAtoms)
			throw Exception(tr("This modifier cannot be evaluated because the input object does not contain any atoms."));
		TimeInterval validityInterval = state.stateValidity();

		// Let the virtual function of the derived class do the actual work.
		status = modifyAtomsObject(time, validityInterval);

		// Put result into geometry pipeline.
		if(outputAtoms) {
			state.setResult(outputAtoms);
		}
		state.intersectStateValidity(validityInterval);
	}
	catch(const Exception& ex) {
		ex.logError();
		// Transfer exception message to evaluation status.
		QString msg = ex.message();
		for(int i=1; i<ex.messages().size(); i++) {
			msg += "\n";
			msg += ex.messages()[i];
		}
		status = EvaluationStatus(EvaluationStatus::EVALUATION_ERROR, msg);
	}

	// Cleanup
	_cloneHelper.reset();
	inputAtoms = NULL;
	outputAtoms = NULL;
	this->modApp = NULL;

	return status;
#endif

	return ObjectStatus();
}

#if 0
/******************************************************************************
* Creates a shallow copy of the given input object on demand and returns it.
******************************************************************************/
AtomsObject* ParticleModifier::output()
{
	OVITO_ASSERT(inputAtoms != NULL);
	if(outputAtoms) return outputAtoms.get();

	// Make a shallow copy.
	outputAtoms = cloneHelper()->cloneObject(inputAtoms, false);
	return outputAtoms.get();
}

/******************************************************************************
* Returns the standard channel with the given identifier from the input object.
* The returned channel may be NULL if it does not exist. Its contents
* may not be modified.
******************************************************************************/
DataChannel* ParticleModifier::inputStandardChannel(DataChannel::DataChannelIdentifier which) const
{
	return input()->getStandardDataChannel(which);
}

/******************************************************************************
* Returns the channel with the given identifier from the input object.
* The returned channel may not be modified. If they input object does
* not contain a channel with the given identifier then an exception is thrown.
* If there is a channel with the given identifier but with another data type then
* an exception is thrown too.
******************************************************************************/
DataChannel* ParticleModifier::expectCustomChannel(const QString& channelName, int channelDataType, size_t componentCount) const
{
	DataChannel* channel = input()->findDataChannelByName(channelName);
	if(!channel)
		throw Exception(tr("The modifier cannot be evaluated because the input object does not contain the required data channel (name: %1).").arg(channelName));
	if(channel->type() != channelDataType)
		throw Exception(tr("The modifier cannot be evaluated because the data channel '%1' in the input object has not the required data type.").arg(channel->name()));
	if(channel->componentCount() != componentCount)
		throw Exception(tr("The modifier cannot be evaluated because the data channel '%1' in the input object has not the required number of components per atom.").arg(channel->name()));
	return channel;
}

/******************************************************************************
* Returns the given standard channel from the input object.
* The returned channel may not be modified. If they input object does
* not contain the standard channel then an exception is thrown.
******************************************************************************/
DataChannel* ParticleModifier::expectStandardChannel(DataChannel::DataChannelIdentifier which) const
{
	DataChannel* channel = input()->getStandardDataChannel(which);
	if(!channel)
		throw Exception(tr("The modifier cannot be evaluated because the input object does not contain the required data channel '%1'.").arg(DataChannel::standardChannelName(which)));
	return channel;
}

/******************************************************************************
* Returns the standard channel with the given identifier from the output object.
* The requested data channel will be created or deep copied as needed.
******************************************************************************/
DataChannel* ParticleModifier::outputStandardChannel(DataChannel::DataChannelIdentifier which)
{
	DataChannel* channel = output()->copyShallowChannel(output()->createStandardDataChannel(which));
	output()->invalidate();
	return channel;
}

#endif

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void ParticleModifier::saveToStream(ObjectSaveStream& stream)
{
	Modifier::saveToStream(stream);
	stream.beginChunk(0x01);
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void ParticleModifier::loadFromStream(ObjectLoadStream& stream)
{
	Modifier::loadFromStream(stream);
	stream.expectChunk(0x01);
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> ParticleModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<ParticleModifier> clone = static_object_cast<ParticleModifier>(Modifier::clone(deepCopy, cloneHelper));
	return clone;
}

/******************************************************************************
* This handler is called when a new edit object has been loaded into the editor.
******************************************************************************/
void ParticleModifierEditor::onContentsReplaced(RefTarget* newEditObject)
{
	ModifierApplication* modApp = nullptr;
	Modifier* mod = dynamic_object_cast<Modifier>(newEditObject);
	if(mod && mod->modifierApplications().empty() == false)
		modApp = mod->modifierApplications().front();
	_modAppListener.setTarget(modApp);

	updateStatusLabel(modApp);
}

/******************************************************************************
* This handler is called when the current ModifierApplication sends a
* notification event.
******************************************************************************/
void ParticleModifierEditor::onModAppNotificationEvent(ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::StatusChanged) {
		ModifierApplication* modApp = dynamic_object_cast<ModifierApplication>(event->sender());
		updateStatusLabel(modApp);
	}
}

/******************************************************************************
* Updates the text of the result label.
******************************************************************************/
void ParticleModifierEditor::updateStatusLabel(ModifierApplication* modApp)
{
	if(!_statusTextLabel || !_statusIconLabel) return;

	if(modApp != NULL) {
		_statusTextLabel->setText(modApp->status().longText());
		if(modApp->status().type() == ObjectStatus::Success) {
			if(modApp->status().longText().isEmpty() == false)
				_statusIconLabel->setPixmap(_modifierStatusInfoIcon);
			else
				_statusIconLabel->clear();
		}
		else if(modApp->status().type() == ObjectStatus::Warning)
			_statusIconLabel->setPixmap(_modifierStatusWarningIcon);
		else if(modApp->status().type() == ObjectStatus::Error)
			_statusIconLabel->setPixmap(_modifierStatusErrorIcon);
		else
			_statusIconLabel->clear();
	}
	else {
		_statusTextLabel->clear();
		_statusIconLabel->clear();
	}
}

/******************************************************************************
* Returns a widget that displays a message sent by the modifier that
* states the outcome of the modifier evaluation. Derived classes of this
* editor base class can add the widget to their user interface.
******************************************************************************/
QWidget* ParticleModifierEditor::statusLabel()
{
	if(_statusLabel) return _statusLabel;

	_statusLabel = new QWidget();
	QGridLayout* layout = new QGridLayout(_statusLabel);
	layout->setContentsMargins(0,0,0,0);
	layout->setColumnStretch(1, 1);
	//_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	_statusIconLabel = new QLabel(_statusLabel);
	_statusIconLabel->setAlignment(Qt::AlignTop);
	layout->addWidget(_statusIconLabel, 0, 0, Qt::AlignTop);

	_statusTextLabel = new QLabel(_statusLabel);
	_statusTextLabel->setAlignment(Qt::AlignTop);
	//_statusTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_statusTextLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
	_statusTextLabel->setWordWrap(true);
	layout->addWidget(_statusTextLabel, 0, 1);

	return _statusLabel;
}

};	// End of namespace

