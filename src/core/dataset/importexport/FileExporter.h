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

namespace Ovito { namespace DataIO {

/**
 * \brief Abstract base class for file export services.
 */
class OVITO_CORE_EXPORT FileExporter : public RefTarget
{
protected:
	
	/// \brief The constructor.
	FileExporter(DataSet* dataset) : RefTarget(dataset) {}

public:

	/// \brief Returns the file filter that specifies the files that can be exported by this service.
	/// \return A wild-card pattern that specifies the file types that can be handled by this export class.
	virtual QString fileFilter() = 0;

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	/// \return A string that describes the file format.
	virtual QString fileFilterDescription() = 0;

	/// \brief Exports scene nodes to a file.
	/// \param nodes The list of scene nodes to be exported.
	/// \param filePath The path of the output file.
	/// \param noninteractive Specifies whether the export operation should be performed non-interactively,
	///                       i.e. without showing any dialogs etc.
	/// \return \c true if the output file has been successfully written.
	///         \c false if the export operation has been canceled by the user.
	/// \throw Exception when the export has failed or an error has occurred.
	Q_INVOKABLE virtual bool exportToFile(const QVector<SceneNode*>& nodes, const QString& filePath, bool noninteractive = true) = 0;

	/// Returns a list of all available exporter types.
	static QVector<OvitoObjectType*> availableExporters();

private:

	Q_OBJECT
	OVITO_OBJECT
};

}}	// End of namespace

#endif // __OVITO_FILE_EXPORTER_H
