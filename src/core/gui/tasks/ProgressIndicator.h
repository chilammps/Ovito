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

/**
 * \file ProgressIndicator.h
 * \brief Contains the definition of the Ovito::ProgressIndicator class.
 */

#ifndef __OVITO_PROGRESS_INDICATOR_H
#define __OVITO_PROGRESS_INDICATOR_H

#include <core/Core.h>

namespace Ovito {

/**
 * \brief This class is used to report the progress of a long-running operation to the user.
 *
 * \author Alexander Stukowski
 */
class CORE_DLLEXPORT ProgressIndicator : public QObject
{
	Q_OBJECT

public:

	/// \brief Constructs and initializes the progress indicator object.
	/// \param labelText The initial status text that describes the operation.
	/// \param maximum The number of work steps that make up the operation.
	/// \param forceBackground Forces the progress dialog to not show up. Instead the progress is
	///                        indicated in the status bar of the main window.
	ProgressIndicator(const QString& labelText = QString(), int maximum = 0, bool forceBackground = false);

	/// \brief Destructor that closes the internal progress dialog if there are no more
	///        ProgressIndicator objects left.
	virtual ~ProgressIndicator();

	/// \brief Returns the number of work steps of the current operation.
	/// \return The number of work steps or 0 if the number is not defined.
	/// \sa setMaximum()
	int maximum() const { return _maximum; }

	/// \brief Sets the number of work steps of the current operation.
	/// \param maximum The number of steps or 0 if the number is not defined.
	/// \sa maximum()
	void setMaximum(int maximum);

	/// \brief Returns the current progress.
	/// \return The number of work steps done so far.
	/// \sa setValue()
	/// \sa maximum()
	int value() const { return _value; }

	/// \brief Returns a description of the current operation.
	/// \return A string that described the ongoing operation in progress.
	QString labelText() const { return _labelText; }

	/// \brief Updates the description string of the current operation.
	/// \param newText The new description.
	/// \sa labelText()
	void setLabelText(const QString& newText);

	/// \brief Checks whether the user has canceled the operation.
	/// \return \c true if the user has hit the "Cancel" button to abort the operation;
	///         \c false if the operation should continue.
	///
	/// The operation routine should call this function from time to time to check whether
	/// the operation should be canceled.
	bool isCanceled() const;

	/// \brief Sets the abort flag for this operation.
	void setCanceled(bool canceled);

	/// \brief Shows the progress of the given QFuture object and waits
	///        until the operation has finished.
	/// \return \c true if the operation was successful; \c false if the operation has been canceled by the user.
	bool waitForFuture(const QFuture<void>& future);

public:

	Q_PROPERTY(bool isCanceled READ isCanceled WRITE setCanceled)
	Q_PROPERTY(int value READ value WRITE setValue)
	Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
	Q_PROPERTY(QString labelText READ labelText WRITE setLabelText)

public Q_SLOTS:

	/// \brief Sets the number of work steps of the current operation.
	void setRange(int minimum, int maximum) { setMaximum(maximum); }

	/// \brief Sets the number of work steps done so far.
	/// \param progress The new number of accomplished steps. This must be between 0 and the maximum().
	/// \sa value()
	void setValue(int progress);

Q_SIGNALS:

	/// \brief This signal is emitted when the maximum number of work steps of this operation has changed.
	/// \param newMaximum The new number of work steps.
	/// \param operation The operation that generated the signal, i.e. this object.
	void maximumChanged(int newMaximum, ProgressIndicator* operation);

	/// \brief This signal is emitted when the current progress this operation has changed.
	/// \param newValue The new number of work steps done so far.
	/// \param operation The operation that generated the signal, i.e. this object.
	void valueChanged(int newValue, ProgressIndicator* operation);

	/// \brief This signal is emitted when the string that describes the ongoing operation has changed.
	/// \param newLabelString The new description string.
	/// \param operation The operation that generated the signal, i.e. this object.
	void labelChanged(const QString& newLabelString, ProgressIndicator* operation);

	/// \brief This signal is emitted when the user has canceled the operation.
	void canceled();

private:

	/// The maximum progress value.
	int _maximum;

	/// The current progress value, which is between zero and the maximum.
	int _value;

	/// The textual representation of the progress.
	QString _progressText;

	/// Flag that indicates that the operation has been canceled.
	bool _isC	anceled;
};


/**
 * \brief Dialog box that displays the current progress of one or more
 *        operations.
 *
 * \note This is a private implementation class and should not be used
 *       by plugin developers.
 *
 * \author Alexander Stukowski
 * \sa ProgressIndicator
 */
class ProgressIndicatorDialog : public QDialog
{
	Q_OBJECT

public:

	/// \brief Registers a new indicator that should be displayed in the dialog.
	/// \param indicator The new indicator.
	/// \param suppressDialog Specifies whether the progress dialog should be shown or not.
	///
	/// This method is called from the constructor of the ProgressIndicator class.
	static void registerIndicator(ProgressIndicator* indicator, bool suppressDialog);

	/// \brief Removes an indicator from the dialog.
	/// \param indicator The indicator to be removed.
	///
	/// This method is called from the destructor of the ProgressIndicator class.
	static void unregisterIndicator(ProgressIndicator* indicator);

	/// \brief Lets the application process user events.
	static void processEvents();

	/// \brief Returns whether the progress dialog is currently visible.
	static bool isDialogActive() { return dialog.isNull() == false && dialog->isVisible(); }

private Q_SLOTS:

	void onIndicatorMaximumChanged(int newMaximum, ProgressIndicator* operation);
	void onIndicatorValueChanged(int newValue, ProgressIndicator* operation);
	void onIndicatorLabelChanged(const QString& newLabelString, ProgressIndicator* operation);
	void onCancel();

	void onIndicatorsChanged();
	void onShowIndicator();

private:

	/// \brief Constructor that shows the dialog.
	ProgressIndicatorDialog(bool hideDialog);

	/// \brief Destructor that closes the dialog.
	~ProgressIndicatorDialog();

	/// The button that lets the user abort the running operation.
	QPushButton* cancelButton;

	/// The main status label.
	QLabel* statusLabel1;

	/// The main progress bar.
	QProgressBar* progressBar1;

	/// The secondary status label.
	QLabel* statusLabel2;

	/// The secondary progress bar.
	QProgressBar* progressBar2;

	/// The external message label shown in the status bar of the main window.
	QPointer<QLabel> externalMessageLabel;

	/// The external progress bar shown in the status bar of the main window.
	QPointer<QProgressBar> externalProgressBar;

	/// The instance of the dialog if it is shown at the moment.
	static QPointer<ProgressIndicatorDialog> dialog;

	/// The list of indicators for ongoing operations.
	static QStack<ProgressIndicator*> indicators;
};

};

#endif // __OVITO_PROGRESS_INDICATOR_H
