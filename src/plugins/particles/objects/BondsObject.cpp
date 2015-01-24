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

#include <plugins/particles/Particles.h>
#include "BondsObject.h"

namespace Ovito { namespace Particles {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, BondsObject, DataObject);

/******************************************************************************
* Default constructor.
******************************************************************************/
BondsObject::BondsObject(DataSet* dataset, BondsStorage* storage) : DataObject(dataset),
	_storage(storage ? storage : new BondsStorage())
{
}

/******************************************************************************
* Replaces the internal storage object with the given one.
******************************************************************************/
void BondsObject::setStorage(BondsStorage* storage)
{
	OVITO_CHECK_POINTER(storage);
	_storage = storage;
	changed();
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void BondsObject::saveToStream(ObjectSaveStream& stream)
{
	DataObject::saveToStream(stream);

	stream.beginChunk(0x01);
	_storage.constData()->saveToStream(stream, !saveWithScene());
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void BondsObject::loadFromStream(ObjectLoadStream& stream)
{
	DataObject::loadFromStream(stream);

	stream.expectChunk(0x01);
	_storage->loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> BondsObject::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<BondsObject> clone = static_object_cast<BondsObject>(DataObject::clone(deepCopy, cloneHelper));

	// Shallow copy storage.
	clone->_storage = this->_storage;

	return clone;
}

/******************************************************************************
* Remaps the bonds after some of the particles have been deleted.
* Dangling bonds are removed too.
******************************************************************************/
void BondsObject::particlesDeleted(const boost::dynamic_bitset<>& deletedParticlesMask)
{
	// Build map that maps old particle indices to new indices.
	std::vector<size_t> indexMap(deletedParticlesMask.size());
	auto index = indexMap.begin();
	size_t oldParticleCount = deletedParticlesMask.size();
	size_t newParticleCount = 0;
	for(size_t i = 0; i < deletedParticlesMask.size(); i++)
		*index++ = deletedParticlesMask.test(i) ? std::numeric_limits<size_t>::max() : newParticleCount++;

	_storage.detach();
	auto result = _storage->bonds().begin();
	auto bond = _storage->bonds().begin();
	auto last = _storage->bonds().end();
	for(; bond != last; ++bond) {
		// Remove invalid bonds.
		if(bond->index1 >= oldParticleCount || bond->index2 >= oldParticleCount)
			continue;

		// Remove dangling bonds whose particles have gone.
		if(deletedParticlesMask.test(bond->index1) || deletedParticlesMask.test(bond->index2))
			continue;

		// Keep but remap particle indices.
		result->pbcShift = bond->pbcShift;
		result->index1 = indexMap[bond->index1];
		result->index2 = indexMap[bond->index2];
		++result;
	}
	_storage->bonds().erase(result, last);
	changed();
}

}	// End of namespace
}	// End of namespace
