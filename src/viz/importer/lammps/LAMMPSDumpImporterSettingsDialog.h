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

#ifndef __OVITO_LAMMPS_DUMP_IMPORTER_SETTINGS_DIALOG_H
#define __OVITO_LAMMPS_DUMP_IMPORTER_SETTINGS_DIALOG_H

#include <core/Core.h>
#include "LAMMPSTextDumpImporter.h"

namespace Viz {

/******************************************************************************
* This dialog box lets the user adjust the settings of the LAMMPS dump importer.
******************************************************************************/
class LAMMPSDumpImporterSettingsDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// Constructor.
	LAMMPSDumpImporterSettingsDialog(LAMMPSTextDumpImporter* importer, QWidget* parent = 0);

protected Q_SLOTS:

	/// This is called when the user has pressed the OK button.
	void onOk();

protected:

	/// The parser whose settings are being edited.
	OORef<LAMMPSTextDumpImporter> importer;
	QLineEdit* sourceTextbox;
	QCheckBox* multiTimestepCheckbox;
};

};

#endif // __OVITO_LAMMPS_DUMP_IMPORTER_SETTINGS_DIALOG_H
