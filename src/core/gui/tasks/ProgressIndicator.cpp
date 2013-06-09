///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2008) Alexander Stukowski
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
#include <core/utilities/ProgressIndicator.h>
#include <core/gui/ApplicationManager.h>
#include <core/gui/mainwnd/MainFrame.h>
#include <core/viewport/ViewportManager.h>


namespace Core {

/******************************************************************************
* Constructs and initializes the progress indicator object.
******************************************************************************/
ProgressIndicator::ProgressIndicator(const QString& labelText, int maximum, bool forceBackground)
{
	_canceled = false;
	_maximum = maximum;
	_value = 0;
	setLabelText(labelText);

	// Block any viewport updates while the progress bar is shown.
	VIEWPORT_MANAGER.suspendViewportUpdates();

	ProgressIndicatorDialog::registerIndicator(this, forceBackground);
}

/******************************************************************************
* Destructor that closes the internal progress dialog if there are no more
* ProgressIndicator objects left.
******************************************************************************/
ProgressIndicator::~ProgressIndicator()
{
	ProgressIndicatorDialog::unregisterIndicator(this);

	// Update viewports.
	VIEWPORT_MANAGER.resumeViewportUpdates();
}

/******************************************************************************
* Sets the number of work steps of the current operation.
******************************************************************************/
void ProgressIndicator::setMaximum(int maximum)
{
	if(_maximum == maximum) return;
	_maximum = maximum;
	maximumChanged(maximum, this);
}

/******************************************************************************
* Sets the number of work steps done so far.
******************************************************************************/
void ProgressIndicator::setValue(int progress)
{
	if(_value == progress) return;
	_value = progress;
	valueChanged(progress, this);
}

/******************************************************************************
* Updates the description string of the current operation.
******************************************************************************/
void ProgressIndicator::setLabelText(const QString& newText)
{
	if(_labelText == newText) return;
	_labelText = newText;

	// Print the new label text to the console if it has changed.
	if(!_labelText.isEmpty()) {
		// Also print the current progress in percent.
		if(_value != 0 && _maximum != 0)
			MsgLogger() << QString("%1 (%2%)").arg(_labelText).arg(_value * 100 / _maximum).toLocal8Bit().constData() << endl;
		else
			MsgLogger() << _labelText.toLocal8Bit().constData() << endl;
	}

	labelChanged(_labelText, this);
}

/******************************************************************************
* Checks whether the user has canceled the operation.
******************************************************************************/
bool ProgressIndicator::isCanceled() const
{
	ProgressIndicatorDialog::processEvents();
	return _canceled;
}

/******************************************************************************
* Sets the abort flag for this operation.
******************************************************************************/
void ProgressIndicator::setCanceled(bool canceled)
{
	_canceled = canceled;
	if(canceled) {
		this->canceled();
	}
}

/******************************************************************************
* Shows the progress of the given QFuture object and waits
* until the operation has finished.
******************************************************************************/
bool ProgressIndicator::waitForFuture(const QFuture<void>& future)
{
	QFutureWatcher<void> futureWatcher;

    connect(&futureWatcher, SIGNAL(progressRangeChanged(int, int)), this, SLOT(setRange(int, int)));
    connect(&futureWatcher, SIGNAL(progressValueChanged(int)), this, SLOT(setValue(int)));
    setRange(future.progressMinimum(), future.progressMaximum());
    futureWatcher.setFuture(future);

	// Wait for the future to finish and process messages.
	QEventLoop eventLoop;
	connect(&futureWatcher, SIGNAL(finished()), &eventLoop, SLOT(quit()));
	connect(this, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
	if(futureWatcher.isFinished() == false)
		eventLoop.exec();
    // Wait for the future to finish without waiting for messages.
    futureWatcher.waitForFinished();

    return (futureWatcher.isCanceled() == false && !isCanceled());
}

////////////////////////////// ProgressIndicatorDialog ////////////////////////////

QPointer<ProgressIndicatorDialog> ProgressIndicatorDialog::dialog;
QStack<ProgressIndicator*> ProgressIndicatorDialog::indicators;

/******************************************************************************
* Registers a new indicator that should be displayed in the dialog.
******************************************************************************/
void ProgressIndicatorDialog::registerIndicator(ProgressIndicator* indicator, bool suppressDialog)
{
	CHECK_POINTER(indicator);
	OVITO_ASSERT(!indicators.contains(indicator));
	indicators.push(indicator);

	// Create the dialog box.
	if(APPLICATION_MANAGER.guiMode()) {
		if(dialog.isNull())
			dialog = new ProgressIndicatorDialog(suppressDialog);

		connect(indicator, SIGNAL(maximumChanged(int,ProgressIndicator*)), dialog, SLOT(onIndicatorMaximumChanged(int,ProgressIndicator*)));
		connect(indicator, SIGNAL(valueChanged(int,ProgressIndicator*)), dialog, SLOT(onIndicatorValueChanged(int,ProgressIndicator*)));
		connect(indicator, SIGNAL(labelChanged(const QString&,ProgressIndicator*)), dialog, SLOT(onIndicatorLabelChanged(const QString&,ProgressIndicator*)));

		dialog->onIndicatorsChanged();
		if(dialog->cancelButton->isEnabled() == false)
			indicator->setCanceled(true);
	}
}

/******************************************************************************
* Removes an indicator from the dialog.
******************************************************************************/
void ProgressIndicatorDialog::unregisterIndicator(ProgressIndicator* indicator)
{
	CHECK_POINTER(indicator);
	OVITO_ASSERT(indicators.contains(indicator));

	indicators.remove(indicators.indexOf(indicator));
	if(!dialog.isNull())
		dialog->onIndicatorsChanged();

	if(indicators.empty() && dialog.isNull() == false) {
		delete dialog;
		OVITO_ASSERT(dialog.isNull());
	}

	OVITO_ASSERT(!indicators.contains(indicator));
}

/******************************************************************************
* Lets the application process user events.
******************************************************************************/
void ProgressIndicatorDialog::processEvents()
{
	if(isDialogActive())
		QCoreApplication::processEvents();
	else
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

/******************************************************************************
* Constructor that shows the dialog.
******************************************************************************/
ProgressIndicatorDialog::ProgressIndicatorDialog(bool hideDialog)
	: QDialog(MAIN_FRAME)
{
	setWindowTitle(tr("Operation in progress..."));

	QVBoxLayout* mainlayout = new QVBoxLayout(this);
	QGridLayout* sublayout = new QGridLayout();
	statusLabel1 = new QLabel(this);
	statusLabel1->setMinimumWidth(400);
	sublayout->addWidget(statusLabel1, 0, 0, 1, 2);
	progressBar1 = new QProgressBar(this);
	sublayout->addWidget(progressBar1, 1, 0, 1, 2);

	statusLabel2 = new QLabel(this);
	sublayout->addWidget(statusLabel2, 2, 1, 1, 1);
	progressBar2 = new QProgressBar(this);
	sublayout->addWidget(progressBar2, 2, 0, 1, 1);
	sublayout->setColumnStretch(0, 1);
	sublayout->setColumnStretch(1, 3);
	sublayout->setRowStretch(3, 1);

	mainlayout->addLayout(sublayout);

	cancelButton = new QPushButton(tr("&Cancel"), this);
	mainlayout->addWidget(cancelButton, 0, Qt::AlignRight);
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(onCancel()));

	setModal(true);

	if(hideDialog) {
		externalMessageLabel = new QLabel(this);
		externalProgressBar = new QProgressBar(this);
		externalProgressBar->setMaximumHeight(MAIN_FRAME->statusBar()->height()-5);

		// Show the progress indicator at a somewhat later time to prevent
		// flickering during very short operations.
		QTimer::singleShot(200, this, SLOT(onShowIndicator()));
	}
	else {
		show();
	}
}

ProgressIndicatorDialog::~ProgressIndicatorDialog()
{
	if(externalMessageLabel) {
		delete (QLabel*)externalMessageLabel;
		OVITO_ASSERT(externalMessageLabel.isNull());
		delete (QProgressBar*)externalProgressBar;
		OVITO_ASSERT(externalProgressBar.isNull());
		//QApplication::restoreOverrideCursor();
	}
}

void ProgressIndicatorDialog::onShowIndicator()
{
	MAIN_FRAME->statusBar()->addWidget(externalMessageLabel, 1);
	MAIN_FRAME->statusBar()->addPermanentWidget(externalProgressBar, 0);
	//QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void ProgressIndicatorDialog::onIndicatorMaximumChanged(int newMaximum, ProgressIndicator* operation)
{
	if(indicators.size() >= 1 && indicators[0] == operation)
		progressBar1->setMaximum(operation->maximum());
	else if(indicators.size() >= 2 && indicators[1] == operation)
		progressBar2->setMaximum(operation->maximum());

	if(externalProgressBar && !indicators.empty() && indicators.top() == operation)
		externalProgressBar->setMaximum(operation->maximum());
}

void ProgressIndicatorDialog::onIndicatorValueChanged(int newValue, ProgressIndicator* operation)
{
	if(indicators.size() >= 1 && indicators[0] == operation)
		progressBar1->setValue(operation->value());
	else if(indicators.size() >= 2 && indicators[1] == operation)
		progressBar2->setValue(operation->value());

	if(externalProgressBar && !indicators.empty() && indicators.top() == operation)
		externalProgressBar->setValue(operation->value());
}

void ProgressIndicatorDialog::onIndicatorLabelChanged(const QString& newLabelString, ProgressIndicator* operation)
{
	if(indicators.size() >= 1 && indicators[0] == operation)
		statusLabel1->setText(operation->labelText());
	else if(indicators.size() >= 2 && indicators[1] == operation)
		statusLabel2->setText(operation->labelText());

	if(externalMessageLabel && !indicators.empty() && indicators.top() == operation)
		externalMessageLabel->setText(operation->labelText());

	ProgressIndicatorDialog::processEvents();
}

void ProgressIndicatorDialog::onCancel()
{
	Q_FOREACH(ProgressIndicator* indicator, indicators) {
		indicator->setCanceled(true);
	}
	cancelButton->setEnabled(false);
}

void ProgressIndicatorDialog::onIndicatorsChanged()
{
	if(indicators.size() >= 1) {
		statusLabel1->setText(indicators[0]->labelText());
		progressBar1->setMaximum(indicators[0]->maximum());
		progressBar1->setValue(indicators[0]->value());
	}
	else {
		statusLabel1->setText(QString());
		progressBar1->setValue(0);
	}

	if(indicators.size() >= 2) {
		statusLabel2->setText(indicators[1]->labelText());
		progressBar2->setMaximum(indicators[1]->maximum());
		progressBar2->setValue(indicators[1]->value());
		statusLabel2->setVisible(true);
		progressBar2->setVisible(true);
	}
	else {
		statusLabel2->setVisible(false);
		progressBar2->setVisible(false);
		statusLabel2->setText(QString());
		progressBar2->setValue(0);
	}

	if(externalMessageLabel && externalProgressBar) {
		if(!indicators.empty()) {
			externalMessageLabel->setText(indicators.top()->labelText());
			externalProgressBar->setMaximum(indicators.top()->maximum());
			externalProgressBar->setValue(indicators.top()->value());
		}
		else {
			externalMessageLabel->setText(QString());
			externalProgressBar->setValue(0);
		}
	}
}

};
