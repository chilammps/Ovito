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

#ifndef __OVITO_FILE_MANAGER_H
#define __OVITO_FILE_MANAGER_H

#include <core/Core.h>
#include <core/utilities/concurrent/Future.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Util) OVITO_BEGIN_INLINE_NAMESPACE(IO)

/**
 * \brief The file manager provides transparent access to remote files.
 */
class OVITO_CORE_EXPORT FileManager : public QObject
{
	Q_OBJECT

public:

	/// \brief Returns the one and only instance of this class.
	/// \return The predefined instance of the UnitsManager singleton class.
	inline static FileManager& instance() {
		OVITO_ASSERT_MSG(_instance != nullptr, "FileManager::instance", "Singleton object is not initialized yet.");
		return *_instance;
	}

	/// \brief Makes a file available on this computer.
	/// \return A QFuture that will provide the local file name of the downloaded file.
	Future<QString> fetchUrl(DataSetContainer& container, const QUrl& url);

	/// \brief Removes a cached remote file so that it will be downloaded again next
	///        time it is requested.
	void removeFromCache(const QUrl& url);

	/// \brief Lists all files in a remote directory.
	/// \return A QFuture that will provide the list of file names.
	Future<QStringList> listDirectoryContents(const QUrl& url);

	/// \brief Looks up login name and password for the given host in the credential cache.
	QPair<QString,QString> findCredentials(const QString& host);

	/// \brief Saves the login name and password for the given host in the credential cache.
	void cacheCredentials(const QString& host, const QString& username, const QString& password);

	/// \brief Constructs a URL from a path entered by the user.
	QUrl urlFromUserInput(const QString& path);

private:

	/// Is called when a remote file has been fetched.
	void fileFetched(QUrl url, QTemporaryFile* localFile);

	/// Strips a URL from username and password information.
	QUrl normalizeUrl(const QUrl& url) const {
		QUrl strippedUrl = url;
		strippedUrl.setUserName(QString());
		strippedUrl.setPassword(QString());
		return strippedUrl;
	}

private:

	/// The remote files that are currently being fetched.
	QMap<QUrl, Future<QString>> _pendingFiles;

	/// The remote files that have already been downloaded to the local cache.
	QMap<QUrl, QTemporaryFile*> _cachedFiles;

	/// The mutex to synchronize access to above data structures.
	QMutex _mutex;

	/// Cache of login/password information for remote machines.
	QMap<QString, QPair<QString,QString>> _credentialCache;

private:
    
	/// This is a singleton class. No public instances allowed.
	FileManager();

	/// Create the singleton instance of this class.
	static void initialize() { _instance = new FileManager(); }

	/// Deletes the singleton instance of this class.
	static void shutdown() { delete _instance; _instance = nullptr; }

	/// The singleton instance of this class.
	static FileManager* _instance;

	friend class Application;
	friend class SftpDownloadJob;
};

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

#endif // __OVITO_FILE_MANAGER_H
