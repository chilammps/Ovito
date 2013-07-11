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
 * \file AsynchronousParticleModifier.h
 * \brief Contains the definition of the Viz::AsynchronousParticleModifier class.
 */

#ifndef __OVITO_ASYNC_PARTICLE_MODIFIER_H
#define __OVITO_ASYNC_PARTICLE_MODIFIER_H

#include <core/Core.h>

#include <viz/modifier/ParticleModifier.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleType.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Base class for modifiers that compute their results in a background thread.
 */
class AsynchronousParticleModifier : public ParticleModifier
{
public:

	/// Default constructor.
	AsynchronousParticleModifier();

	/// This method is called by the system when an item in the modification pipeline located before this modifier has changed.
	virtual void modifierInputChanged(ModifierApplication* modApp) override;

	/// Returns whether the recalculation of the modifier results is performed every time the input data changes.
	bool autoUpdate() const { return _autoUpdate; }

	/// Sets whether the recalculation of the modifier results is performed every time the input data changes.
	void setAutoUpdate(bool on) { _autoUpdate = on; }

public:

	Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate)

protected Q_SLOTS:

	/// \brief This is called when the background job has finished.
	virtual void backgroundJobFinished();

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Modifies the particle object. The time interval passed
	/// to the function is reduced to the interval where the modified object is valid/constant.
	virtual ObjectStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// Cancels any running background job.
	void cancelBackgroundJob();

	void

private:

	/// Controls whether the analysis is performed every time the input data changes.
	PropertyField<bool> _autoUpdate;

	/// The background job.
	Future<QExplicitlySharedDataPointer<ParticleProperty>> _backgroundOperation;

	/// The watcher object that is used to monitor the background operation.
	FutureWatcher _backgroundOperationWatcher;

	/// Indicates if and how long the cached modifier results are valid.
	TimeInterval _cacheValidity;

	/// Indicates whether the modifier's results are currently being computed in the background.
	TimeInterval _computationValidity;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_autoUpdate);
};

};	// End of namespace

#endif // __OVITO_ASYNC_PARTICLE_MODIFIER_H
