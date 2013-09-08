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

#ifndef __OVITO_PARTICLE_EXPORTER_SETTINGS_DIALOG_H
#define __OVITO_PARTICLE_EXPORTER_SETTINGS_DIALOG_H

#include <core/Core.h>
#include <core/gui/widgets/SpinnerWidget.h>
#include "ParticleExporter.h"

namespace Viz {

class OutputColumnMapping;

/******************************************************************************
* This dialog box lets the user adjust the export settings.
******************************************************************************/
class ParticleExporterSettingsDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// Constructor.
	ParticleExporterSettingsDialog(QWidget* parent, ParticleExporter* exporter, DataSet* dataset, const PipelineFlowState& state, OutputColumnMapping* columnMapping = nullptr);

protected Q_SLOTS:

	/// This is called when the user has pressed the OK button.
	virtual void onOk();

protected:

	OORef<ParticleExporter> _exporter;
	SpinnerWidget* _startTimeSpinner;
	SpinnerWidget* _endTimeSpinner;
	SpinnerWidget* _nthFrameSpinner;
	QLineEdit* _wildcardTextbox;
	QButtonGroup* _fileGroupButtonGroup;
	QButtonGroup* _rangeButtonGroup;
	OutputColumnMapping* _columnMapping;
	QListWidget* _columnMappingWidget;
};

};	// End of namespace

#endif // __OVITO_PARTICLE_EXPORTER_SETTINGS_DIALOG_H
