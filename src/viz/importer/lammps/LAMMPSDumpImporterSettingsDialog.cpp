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

#include <core/Core.h>
#include "LAMMPSDumpImporterSettingsDialog.h"

namespace Viz {

/******************************************************************************
* Constructor.
******************************************************************************/
LAMMPSDumpImporterSettingsDialog::LAMMPSDumpImporterSettingsDialog(LAMMPSTextDumpImporter* importer, QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(tr("LAMMPS Dump File Import Settings"));
	this->importer = importer;

	QVBoxLayout* layout1 = new QVBoxLayout(this);

	// Time steps group
	QGroupBox* sourceGroupBox = new QGroupBox(tr("Data source location"), this);
	layout1->addWidget(sourceGroupBox);

	QVBoxLayout* layout2 = new QVBoxLayout(sourceGroupBox);

	sourceTextbox = new QLineEdit(importer->sourceUrl().toString(), sourceGroupBox);
	sourceTextbox->setMinimumWidth(600);
	layout2->addWidget(sourceTextbox);

	multiTimestepCheckbox = new QCheckBox(tr("File contains multiple timesteps"), sourceGroupBox);
	multiTimestepCheckbox->setChecked(importer->isMultiTimestepFile());
	layout2->addWidget(multiTimestepCheckbox);

	layout1->addStretch(1);

	// Ok and cancel buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOk()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	layout1->addWidget(buttonBox);
}

/******************************************************************************
* This is called when the user has pressed the OK button.
******************************************************************************/
void LAMMPSDumpImporterSettingsDialog::onOk()
{
	try {
		QUrl url = QUrl::fromUserInput(sourceTextbox->text());
		if(!url.isValid())
			throw Exception(tr("Source URL is not valid."));

		// Write settings back to the parser.
		importer->setMultiTimestepFile(multiTimestepCheckbox->isChecked());
		importer->setSourceUrl(url);

		// Close dialog box.
		accept();
	}
	catch(const Exception& ex) {
		ex.showError();
		return;
	}
}

};	// End of namespace
