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

#ifndef __OVITO_REMOTE_AUTHENTICATION_DIALOG_H
#define __OVITO_REMOTE_AUTHENTICATION_DIALOG_H

#include <core/Core.h>
#include <core/dataset/importexport/FileImporter.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui) OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * This dialog asks the user for a username/password for a remote server.
 */
class OVITO_CORE_EXPORT RemoteAuthenticationDialog : public QDialog
{
	Q_OBJECT
	
public:

	/// \brief Constructs the dialog window.
	RemoteAuthenticationDialog(QWidget* parent, const QString& title, const QString& labelText);

	/// \brief Sets the username shown in the dialog.
	void setUsername(const QString& username) { _usernameEdit->setText(username); }

	/// \brief Sets the password shown in the dialog.
	void setPassword(const QString& password) { _passwordEdit->setText(password); }

	/// \brief Returns the username entered by the user.
	QString username() const { return _usernameEdit->text(); }

	/// \brief Returns the password entered by the user.
	QString password() const { return _passwordEdit->text(); }

	/// \brief Displays the dialog.
	virtual int exec() override;

private:

	QLineEdit* _usernameEdit;
	QLineEdit* _passwordEdit;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_REMOTE_AUTHENTICATION_DIALOG_H
