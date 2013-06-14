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

#ifndef __OVITO_ATOMS_IMPORTER_H
#define __OVITO_ATOMS_IMPORTER_H

#include <core/Core.h>
#include <core/dataset/importexport/LinkedFileImporter.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief Base class for file parsers that read atom-position datasets.
 */
class AtomsImporter : public LinkedFileImporter
{
protected:

	/**
	 * Data structure that holds the data read by the parseFile() function.
	 */
	class AtomsData : public ImportedData
	{
	public:

		/// Lets the data container insert the data it holds into the scene by creating
		/// appropriate scene objects.
		virtual void insertIntoScene(LinkedFileObject* destination) override;

		/// Returns the current simulation cell matrix.
		const AffineTransformation& simulationCell() const { return _simulationCell; }

		/// Sets the simulation cell matrix.
		void setSimulationCell(const AffineTransformation& cellMatrix) { _simulationCell = cellMatrix; }

	private:

		/// The geometry of the cell.
		AffineTransformation _simulationCell = AffineTransformation::Zero();

		/// PBC flags.
		std::array<bool,3> _pbcFlags = {{ true, true, true }};
	};

public:

	/// \brief Constructs a new instance of this class.
	AtomsImporter() {}

protected:

	/// \brief Reads the data from the input file(s).
	virtual void loadImplementation(FutureInterface<ImportedDataPtr>& futureInterface, FrameSourceInformation frame) override;

	/// \brief Parses the given input file and stores the data in the given container object.
	virtual void parseFile(FutureInterface<ImportedDataPtr>& futureInterface, AtomsData& container, QIODevice& file) = 0;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_ATOMS_IMPORTER_H
