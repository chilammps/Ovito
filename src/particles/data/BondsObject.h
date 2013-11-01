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
 * \file BondsObject.h
 * \brief Contains the definition of the Particles::BondsObject class.
 */

#ifndef __OVITO_BONDS_OBJECT_H
#define __OVITO_BONDS_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/SceneObject.h>
#include "BondsStorage.h"

namespace Particles {

using namespace Ovito;

/**
 * \brief Stores the bonds between particles.
 */
class BondsObject : public SceneObject
{
public:

	/// \brief Default constructor.
	Q_INVOKABLE BondsObject(BondsStorage* storage = nullptr);

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Bonds"); }

	/// \brief Replaces the internal storage object with the given one.
	void setStorage(BondsStorage* storage);

	/// \brief Returns the internal storage object.
	BondsStorage* storage() const { return _storage.data(); }

	/// Returns the list of bonds between particles.
	const std::vector<BondsStorage::Bond>& bonds() const { return _storage->bonds(); }

	/// Deletes all bonds.
	void clear() {
		_storage.detach();
		_storage->bonds().clear();
		changed();
	}

	/// Remaps the bonds after some of the particles have been deleted.
	/// Dangling bonds are removed too.
	void particlesDeleted(const std::vector<bool>& deletedParticlesMask);

	/// \brief This method must be called every time the contents of the bonds object are changed.
	///        It generates a ReferenceEvent::TargetChanged event.
	void changed() { notifyDependents(ReferenceEvent::TargetChanged); }

protected:

	/// Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

	/// The internal storage object that holds the bonds data.
	QExplicitlySharedDataPointer<BondsStorage> _storage;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_BONDS_OBJECT_H
