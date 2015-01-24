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

#ifndef __OVITO_FILE_EXPORTER_H
#define __OVITO_FILE_EXPORTER_H

#include <core/Core.h>
#include <core/object/OvitoObject.h>
#include <core/dataset/DataSet.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(DataIO)

/**
 * \brief Abstract base class for file exporters that export data from OVITO to an external file.
 *
 * To add an exporter for a new file format to OVITO you should derive a new class from FileExporter or
 * one of its specializations and implement the abstract methods fileFilter(), fileFilterDescription(),
 * and exportToFile().
 *
 * A list of all available exporters can be obtained via FileExporter::availableExporters().
 */
class OVITO_CORE_EXPORT FileExporter : public RefTarget
{
protected:
	
	/// Initializes the object.
	FileExporter(DataSet* dataset) : RefTarget(dataset) {}

public:

	/// \brief Returns the filename filter that specifies the file extension that can be exported by this service.
	/// \return A wild-card pattern for the file types that can be produced by this export class (e.g. \c "*.xyz" or \c "*").
	virtual QString fileFilter() = 0;

	/// \brief Returns the file type description that is displayed in the drop-down box of the export file dialog.
	/// \return A human-readable string describing the file format written by this FileExporter.
	virtual QString fileFilterDescription() = 0;

	/// \brief Exports a set of scene nodes to a file.
	/// \param nodes The list of scene nodes to be exported.
	/// \param filePath The output file path selected by the user.
	/// \param noninteractive Controls whether the export operation should be performed non-interactively,
	///                       i.e. without showing any dialogs that require user interaction.
	///                       This will be \c true when called from a Python script.
	/// \return \c true if the output file has been successfully written;
	///         \c false if the export operation has been canceled by the user.
	/// \throws Util::Exception if the export operation has failed due to an error.
	virtual bool exportToFile(const QVector<SceneNode*>& nodes, const QString& filePath, bool noninteractive = true) = 0;

	/// Returns the list of all available exporter types installed in the system.
	static QVector<OvitoObjectType*> availableExporters();

private:

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FILE_EXPORTER_H
