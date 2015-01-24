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

#ifndef __OVITO_BONDS_STORAGE_H
#define __OVITO_BONDS_STORAGE_H

#include <plugins/particles/Particles.h>

namespace Ovito { namespace Particles {

/**
 * \brief Memory storage for bonds between particles.
 */
class OVITO_PARTICLES_EXPORT BondsStorage : public QSharedData
{
public:

	/**
	 * A single bond between two particles.
	 */
	struct Bond {

		/// If the bond crosses a periodic boundary, this tells us
		/// in which direction.
		Vector_3<int8_t> pbcShift;

		/// The index of the first particle.
		/// Note that we are not using size_t here to save memory.
		unsigned int index1;

		/// The index of the second particle.
		/// Note that we are not using size_t here to save memory.
		unsigned int index2;
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

	/// Returns the list of bonds between particles.
	const std::vector<Bond>& bonds() const { return _bonds; }

	/// Returns the list of bonds between particles.
	std::vector<Bond>& bonds() { return _bonds; }

	/// Adds a new bond to the bonds list.
	void addBond(unsigned int index1, unsigned int index2, const Vector_3<int8_t>& pbcShift) {
		_bonds.push_back({ pbcShift, index1, index2 });
	}

protected:

	/// The list of bonds between particles.
	std::vector<Bond> _bonds;
};

}	// End of namespace
}	// End of namespace

#endif // __OVITO_BONDS_STORAGE_H
