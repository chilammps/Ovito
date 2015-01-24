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

#include <plugins/particles/Particles.h>
#include <core/gui/widgets/general/SpinnerWidget.h>
#include <plugins/particles/objects/ParticlePropertyObject.h>
#include "ParticleExporter.h"
#include "OutputColumnMapping.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Export)

/**
 * \brief This dialog box lets the user adjust the settings of a ParticleExporter.
 */
class OVITO_PARTICLES_EXPORT ParticleExporterSettingsDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// Constructor.
	ParticleExporterSettingsDialog(QWidget* parent, ParticleExporter* exporter, const PipelineFlowState& state, OutputColumnMapping* columnMapping = nullptr);

	/// Extends the dialog by inserting an additional widget (usually a QGroupBox) into the layout.
	void insertWidget(QWidget* widget);

protected Q_SLOTS:

	/// This is called when the user has pressed the OK button.
	virtual void onOk();

protected:

	/// Populates the column mapping list box with an entry.
	void insertPropertyItem(ParticlePropertyReference propRef, const QString& displayName);

	QVBoxLayout* _mainLayout;
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

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_PARTICLE_EXPORTER_SETTINGS_DIALOG_H
