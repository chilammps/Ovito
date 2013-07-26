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
 * \file BondsStorage.h
 * \brief Contains the definition of the Viz::BondsStorage class.
 */

#ifndef __OVITO_BONDS_STORAGE_H
#define __OVITO_BONDS_STORAGE_H

#include <core/Core.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Memory storage for bonds between particles.
 */
class BondsStorage : public QSharedData
{
public:

	/**
	 * A single bond between two particles.
	 */
	struct Bond {

		/// The index of the first particle.
		size_t index1;

		/// The index of the second particle.
		size_t index2;

		/// The
		 index1;
	};

public:

	/// \brief Default constructor that creates an empty storage.
	BondsStorage();

	/// \brief Copy constructor.
	BondsStorage(const BondsStorage& other);

	/// Writes the stored data to an output stream.
	void saveToStream(SaveStream& stream, bool onlyMetadata = false) const;

	/// Reads the stored data from an input stream.
	void loadFromStream(LoadStream& stream);

protected:

};

};	// End of namespace

#endif // __OVITO_BONDS_STORAGE_H
