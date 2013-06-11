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

#ifndef __OVITO_LAMMPS_TEXT_DUMP_IMPORTER_H
#define __OVITO_LAMMPS_TEXT_DUMP_IMPORTER_H

#include <core/Core.h>
#include <core/dataset/importexport/LinkedFileImporter.h>

namespace Viz {

using namespace Ovito;

/**
 * \brief File parser for text-based LAMMPS dump simulation files.
 */
class LAMMPSTextDumpImporter : public LinkedFileImporter
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE LAMMPSTextDumpImporter() {}

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this import class.
	virtual QString fileFilter() override { return "*"; }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() override { return tr("LAMMPS Text Dump Files"); }

protected:

	/// \brief Reads the data from the input file(s).
	virtual void loadImplementation(QFutureInterface<OORef<SceneObject>>& futureInterface, FrameSourceInformation frame, bool suppressDialogs) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_LAMMPS_TEXT_DUMP_IMPORTER_H
