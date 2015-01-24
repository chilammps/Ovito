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

#ifndef __OVITO_INPUT_COLUMN_MAPPING_DIALOG_H
#define __OVITO_INPUT_COLUMN_MAPPING_DIALOG_H

#include <plugins/particles/Particles.h>
#include "InputColumnMapping.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Import)

/**
 * \brief Dialog box that lets the user edit an InputColumnMapping.
 */
class OVITO_PARTICLES_EXPORT InputColumnMappingDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// Constructor.
	InputColumnMappingDialog(const InputColumnMapping& mapping, QWidget* parent);

	/// Fills the editor with the given mapping.
	void setMapping(const InputColumnMapping& mapping);

	/// Returns the user-defined column mapping.
	InputColumnMapping mapping() const;

protected Q_SLOTS:

	/// This is called when the user has pressed the OK button.
	void onOk();

	/// Updates the list of vector components for the given file column.
	void updateVectorComponentList(int columnIndex);

	/// Saves the current mapping as a preset.
	void onSavePreset();

	/// Loads a preset mapping.
	void onLoadPreset();

protected:

	/// \brief Returns the string representation of a property's data type.
	static QString dataTypeToString(int dataType);

	/// The main table widget that contains the entries for each data column of the input file.
	QTableWidget* _tableWidget;

	QVector<QCheckBox*> _fileColumnBoxes;
	QVector<QComboBox*> _propertyBoxes;
	QVector<QComboBox*> _vectorComponentBoxes;
	QVector<int> _propertyDataTypes;

	QSignalMapper* _vectorCmpntSignalMapper;

	QLabel* _fileExcerptLabel;
	QTextEdit* _fileExcerptField;
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_INPUT_COLUMN_MAPPING_DIALOG_H
