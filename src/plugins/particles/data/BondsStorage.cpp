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
#include "BondsStorage.h"

namespace Ovito { namespace Particles {

/******************************************************************************
* Default constructor.
******************************************************************************/
BondsStorage::BondsStorage()
{
}

/******************************************************************************
* Copy constructor.
******************************************************************************/
BondsStorage::BondsStorage(const BondsStorage& other) : _bonds(other._bonds)
{
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void BondsStorage::saveToStream(SaveStream& stream, bool onlyMetadata) const
{
	stream.beginChunk(0x01);
	if(!onlyMetadata) {
		stream.writeSizeT(_bonds.size());
		stream.write(_bonds.data(), _bonds.size() * sizeof(Bond));
	}
	else {
		stream.writeSizeT(0);
	}
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void BondsStorage::loadFromStream(LoadStream& stream)
{
	stream.expectChunk(0x01);
	size_t bondCount;
	stream.readSizeT(bondCount);
	_bonds.resize(bondCount);
	stream.read(_bonds.data(), _bonds.size() * sizeof(Bond));
	stream.closeChunk();
}

}	// End of namespace
}	// End of namespace
