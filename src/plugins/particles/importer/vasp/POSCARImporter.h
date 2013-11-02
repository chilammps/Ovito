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

#ifndef __OVITO_POSCAR_IMPORTER_H
#define __OVITO_POSCAR_IMPORTER_H

#include <core/Core.h>
#include "../ParticleImporter.h"

namespace Particles {

using namespace Ovito;

/**
 * \brief File parser for POSCAR files as used by the VASP DFT code.
 */
class POSCARImporter : public ParticleImporter
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE POSCARImporter() {}

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this import class.
	virtual QString fileFilter() override { return "*"; }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() override { return tr("POSCAR Files"); }

	/// \brief Checks if the given file has format that can be read by this importer.
	virtual bool checkFileFormat(QIODevice& input, const QUrl& sourceLocation) override;

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("POSCAR"); }

protected:

	/// The format-specific task object that is responsible for reading an input file in the background.
	class POSCARImportTask : public ParticleImportTask
	{
	public:

		/// Constructor.
		POSCARImportTask(const LinkedFileImporter::FrameSourceInformation& frame) : ParticleImportTask(frame) {}

	protected:

		/// Parses the given input file and stores the data in this container object.
		virtual void parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream) override;
	};

	/// \brief Creates an import task object to read the given frame.
	virtual ImportTaskPtr createImportTask(const FrameSourceInformation& frame) override {
		return std::make_shared<POSCARImportTask>(frame);
	}

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_POSCAR_IMPORTER_H
