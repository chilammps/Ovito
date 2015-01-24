///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#ifndef __OVITO_ASYNC_PARTICLE_MODIFIER_H
#define __OVITO_ASYNC_PARTICLE_MODIFIER_H

#include <plugins/particles/Particles.h>
#include <core/utilities/concurrent/Task.h>
#include <plugins/particles/modifier/ParticleModifier.h>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers)

/**
 * \brief Base class for modifiers that compute their results in a background thread.
 */
class OVITO_PARTICLES_EXPORT AsynchronousParticleModifier : public ParticleModifier
{
public:

	/**
	 * Abstract base class for compute engines of AsynchronousParticleModifier implementations.
	 */
	class ComputeEngine : public AsynchronousTask
	{
	public:

		/// Constructs a new compute engine.
		ComputeEngine(const TimeInterval& validityInterval) : _validityInterval(validityInterval) {}

		/// Returns the validity period of the results computed by this engine.
		const TimeInterval& validityInterval() const { return _validityInterval; }

		/// Changes the stored validity period of the results computed by this engine.
		void setValidityInterval(const TimeInterval& iv) { _validityInterval = iv; }

	private:

		/// The validity period of the results computed by this engine.
		TimeInterval _validityInterval;
	};

	/// Constructor.
	AsynchronousParticleModifier(DataSet* dataset);

	/// Interrupts a running computation engine if there is one for this asynchronous modifier.
	void stopRunningEngine();

	/// Asks this object to delete itself. Calls stopRunningEngine() first.
	virtual void deleteReferenceObject() override;

private Q_SLOTS:

	/// Is called when the modifier's compute engine has finished.
	virtual void computeEngineFinished();

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Is called when a RefTarget referenced by this object has generated an event.
	virtual bool referenceEvent(RefTarget* source, ReferenceEvent* event) override;

	/// Modifies the particle object. The time interval passed
	/// to the function is reduced to the interval where the modified object is valid/constant.
	virtual PipelineStatus modifyParticles(TimePoint time, TimeInterval& validityInterval) override;

	/// This method is called by the system when the upstream modification pipeline has changed.
	virtual void upstreamPipelineChanged(ModifierApplication* modApp) override;

	/// Invalidates the modifier's result cache so that the results will be recomputed
	/// next time the modifier is evaluated.
	virtual void invalidateCachedResults();

	/// Creates a computation engine that will compute the modifier's results.
	virtual std::shared_ptr<ComputeEngine> createEngine(TimePoint time, TimeInterval validityInterval) = 0;

	/// Unpacks the results of the computation engine and stores them in the modifier.
	virtual void transferComputationResults(ComputeEngine* engine) = 0;

	/// Lets the modifier insert the cached computation results into the modification pipeline.
	virtual PipelineStatus applyComputationResults(TimePoint time, TimeInterval& validityInterval) = 0;

private:

	/// The currently running compute engine.
	std::shared_ptr<ComputeEngine> _runningEngine;

	/// The watcher that is used to monitor the currently running compute engine.
	FutureWatcher _engineWatcher;

	/// The validity interval of the cached computation results.
	TimeInterval _cacheValidity;

	/// The status returned by the compute engine.
	PipelineStatus _computationStatus;

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_ASYNC_PARTICLE_MODIFIER_H
