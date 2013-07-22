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
#include <core/utilities/concurrent/Future.h>
#include <core/utilities/concurrent/Task.h>
#include <core/utilities/concurrent/ProgressManager.h>
#include <core/gui/dialogs/RemoteAuthenticationDialog.h>

#include <ssh/sshconnection.h>
#include <ssh/sshconnectionmanager.h>
#include <ssh/sftpchannel.h>

#include "FileManager.h"

namespace Ovito {

/// The singleton instance of the class.
FileManager* FileManager::_instance = nullptr;

/******************************************************************************
* Constructor.
******************************************************************************/
FileManager::FileManager() : _mutex(QMutex::Recursive)
{
	OVITO_ASSERT_MSG(!_instance, "FileManager constructor", "Multiple instances of this singleton class have been created.");
}

/******************************************************************************
* Makes a file available on this computer.
******************************************************************************/
Future<QString> FileManager::fetchUrl(const QUrl& url)
{
	if(url.isLocalFile()) {
		// Nothing to do to fetch local files. Simply return a finished Future object.
		return Future<QString>(url.toLocalFile(), tr("Loading file %1").arg(url.toLocalFile()));
	}
	else if(url.scheme() == "sftp") {
		QMutexLocker lock(&_mutex);

		QUrl normalizedUrl = normalizeUrl(url);

		// Check if requested URL is already in the cache.
		auto cacheEntry = _cachedFiles.find(normalizedUrl);
		if(cacheEntry != _cachedFiles.end()) {
			return Future<QString>(cacheEntry.value()->fileName(), tr("Loading URL %1").arg(url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));
		}

		// Check if requested URL is already being loaded.
		auto inProgressEntry = _pendingFiles.find(normalizedUrl);
		if(inProgressEntry != _pendingFiles.end()) {
			qDebug() << "Returning running download job";
			return inProgressEntry.value();
		}

		// Fetch remote file.
		Future<QString> future = FileManager::fetchRemoteFile(url);
		_pendingFiles.insert(normalizedUrl, future);
		return future;
	}
	else throw Exception(tr("URL scheme not supported. The program supports only the sftp:// scheme and loading of local files."));
}

/******************************************************************************
* Removes a cached remote file so that it will be downloaded again next
* time it is requested.
******************************************************************************/
void FileManager::removeFromCache(const QUrl& url)
{
	QMutexLocker lock(&_mutex);

	auto cacheEntry = _cachedFiles.find(normalizeUrl(url));
	if(cacheEntry != _cachedFiles.end()) {
		cacheEntry.value()->deleteLater();
		_cachedFiles.erase(cacheEntry);
	}
}

/******************************************************************************
* Is called when a remote file has been fetched.
******************************************************************************/
void FileManager::fileFetched(QUrl url, QTemporaryFile* localFile)
{
	QMutexLocker lock(&_mutex);

	QUrl normalizedUrl = normalizeUrl(url);

	auto inProgressEntry = _pendingFiles.find(normalizedUrl);
	if(inProgressEntry != _pendingFiles.end())
		_pendingFiles.erase(inProgressEntry);
	else
		OVITO_ASSERT(false);

	if(localFile) {
		// Store downloaded file in local cache.
		auto cacheEntry = _cachedFiles.find(normalizedUrl);
		if(cacheEntry != _cachedFiles.end())
			cacheEntry.value()->deleteLater();
		OVITO_ASSERT(localFile->thread() == this->thread());
		localFile->setParent(this);
		_cachedFiles[normalizedUrl] = localFile;
	}
}

/******************************************************************************
* Future object that is responsible for downloading a file via sftp protocol.
******************************************************************************/
class SftpFileFetcher : public QObject, public FutureInterface<QString>
{
	Q_OBJECT

public:

	/// Constructor.
	SftpFileFetcher(const QUrl& url) : _url(url), _connection(nullptr), _timerId(0) {

		// Run all event handlers of this class in the main thread.
		moveToThread(QApplication::instance()->thread());

		// Start download process in the main thread.
		QMetaObject::invokeMethod(this, "start", Qt::AutoConnection);
	}

	/// Opens the SSH connection.
	Q_INVOKABLE void start() {

		// This background task started to run.
		reportStarted();

		// Check if process has already been canceled.
    	if(isCanceled()) {
    		shutdown();
    		return;
    	}

		QSsh::SshConnectionParameters connectionParams;
		connectionParams.host = _url.host();
		connectionParams.userName = _url.userName();
		connectionParams.password = _url.password();
		if(connectionParams.userName.isEmpty() || connectionParams.password.isEmpty()) {
			auto loginInfo = _loginCache.find(connectionParams.host);
			if(loginInfo != _loginCache.end()) {
				connectionParams.userName = loginInfo.value().first;
				connectionParams.password = loginInfo.value().second;
			}
		}
		connectionParams.port = _url.port(22);
		connectionParams.authenticationType = QSsh::SshConnectionParameters::AuthenticationByPassword;
		connectionParams.timeout = 10;

		setProgressText(tr("Connecting to remote server %1").arg(_url.host()));

		// Open connection
		_connection = QSsh::SshConnectionManager::instance().acquireConnection(connectionParams);
		OVITO_CHECK_POINTER(_connection);

		// Listen for signals of the connection.
		connect(_connection, SIGNAL(error(QSsh::SshError)), this, SLOT(onSshConnectionError(QSsh::SshError)));
		if(_connection->state() == QSsh::SshConnection::Connected) {
			onSshConnectionEstablished();
			return;
	    }
		QObject::connect(_connection, SIGNAL(connected()), this, SLOT(onSshConnectionEstablished()));

		// Start to connect.
		if(_connection->state() == QSsh::SshConnection::Unconnected) {
			_connection->connectToHost();
		}
	}

	/// Destructor.
	virtual ~SftpFileFetcher() {
		OVITO_ASSERT(_sftpChannel == nullptr);
		OVITO_ASSERT(_connection == nullptr);
	}

	void shutdown() {
		if(_timerId)
			killTimer(_timerId);
		if(_sftpChannel) {
			QObject::disconnect(_sftpChannel.data(), 0, this, 0);
			_sftpChannel->closeChannel();
			_sftpChannel.clear();
		}
		if(_connection) {
			QObject::disconnect(_connection, 0, this, 0);
			QSsh::SshConnectionManager::instance().releaseConnection(_connection);
			_connection = nullptr;
		}

		if(_localFile) {
			setResult(_localFile->fileName());
		}
		reportFinished();
		FileManager::instance().fileFetched(_url, _localFile.take());

		// Warning: This object will have been deleted at this point.
	}

private Q_SLOTS:

	/// Handles SSH connection errors.
	void onSshConnectionError(QSsh::SshError error) {
		// If authentication failed, ask the user to re-enter username/password.
		if(error == QSsh::SshAuthenticationError && !isCanceled()) {
			OVITO_ASSERT(!_sftpChannel);

			// Ask for new username/password.
			RemoteAuthenticationDialog dialog(nullptr, tr("Remote authentication"),
					_url.password().isEmpty() ?
					tr("<p>Please enter username and password to access the remote machine</p><p><b>%1</b></p>").arg(_url.host()) :
					tr("<p>Authentication failed. Please enter the correct username and password to access the remote machine</p><p><b>%1</b></p>").arg(_url.host()));
			dialog.setUsername(_url.userName());
			dialog.setPassword(_url.password());
			if(dialog.exec() == QDialog::Accepted) {
				// Start over with new login information.
				QObject::disconnect(_connection, 0, this, 0);
				QSsh::SshConnectionManager::instance().releaseConnection(_connection);
				_connection = nullptr;
				_url.setUserName(dialog.username());
				_url.setPassword(dialog.password());
				start();
				return;
			}
			else {
				cancel();
			}
		}

		try {
			throw Exception(tr("Cannot access URL\n\n%1\n\nSSH connection error: %2").arg(_url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)).
				arg(_connection->errorString()));
		}
		catch(Exception& ex) {
			reportException();
		}
		_localFile.reset();
		shutdown();
	}

	/// Is called when the SSH connection has been established.
    void onSshConnectionEstablished() {
    	if(isCanceled()) {
    		shutdown();
    		return;
    	}

    	// After success login, store login information in cache.
    	QSsh::SshConnectionParameters connectionParams = _connection->connectionParameters();
    	_loginCache.insert(connectionParams.host, qMakePair(connectionParams.userName, connectionParams.password));

    	setProgressText(tr("Opening SFTP file transfer channel."));

    	_sftpChannel = _connection->createSftpChannel();
        connect(_sftpChannel.data(), SIGNAL(initialized()), SLOT(onSftpChannelInitialized()));
        connect(_sftpChannel.data(), SIGNAL(initializationFailed(const QString&)), SLOT(onSftpChannelInitializationFailed(const QString&)));
        _sftpChannel->initialize();
    }

    /// Is called when the SFTP channel could not be created.
    void onSftpChannelInitializationFailed(const QString& reason) {
		try {
			throw Exception(tr("Cannot access URL\n\n%1\n\nSFTP error: %2").arg(_url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)).arg(reason));
		}
		catch(Exception& ex) {
			reportException();
		}
		shutdown();
    }

    /// Is called when the SFTP channel has been created.
    void onSftpChannelInitialized() {
    	if(isCanceled()) {
    		shutdown();
    		return;
    	}

		connect(_sftpChannel.data(), SIGNAL(finished(QSsh::SftpJobId, QString)), this, SLOT(onSftpJobFinished(QSsh::SftpJobId, QString)));
		connect(_sftpChannel.data(), SIGNAL(fileInfoAvailable(QSsh::SftpJobId, const QList<QSsh::SftpFileInfo>&)), this, SLOT(onFileInfoAvailable(QSsh::SftpJobId, const QList<QSsh::SftpFileInfo>&)));
    	try {

    		// Set progress text.
    		setProgressText(tr("Fetching remote file %1").arg(_url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

			// Create temporary file.
			_localFile.reset(new QTemporaryFile());
			if(!_localFile->open())
				throw Exception(tr("Failed to create temporary file: %1").arg(_localFile->errorString()));
			_localFile->close();

			// Request file info.
			_sftpChannel->statFile(_url.path());

			// Start to download file.
			_downloadJob = _sftpChannel->downloadFile(_url.path(), _localFile->fileName(), QSsh::SftpOverwriteExisting);
			if(_downloadJob == QSsh::SftpInvalidJob)
				throw Exception(tr("Failed to download remote file %1.").arg(_url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

			// Start timer to monitor download progress.
			_timerId = startTimer(500);
    	}
		catch(Exception& ex) {
			_localFile.reset();
			reportException();
			shutdown();
			return;
		}
    }

    /// Is called after the file has been downloaded.
    void onSftpJobFinished(QSsh::SftpJobId jobId, const QString& errorMessage) {
    	if(jobId != _downloadJob)
    		return;

    	if(isCanceled()) {
    		_localFile.reset();
    		shutdown();
    		return;
    	}
        if(!errorMessage.isEmpty()) {
        	try {
    			throw Exception(tr("Cannot access URL\n\n%1\n\nSFTP error: %2")
    					.arg(_url.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded))
    					.arg(errorMessage));
        	}
			catch(Exception& ex) {
				reportException();
			}
			_localFile.reset();
			shutdown();
			return;
        }
        shutdown();
    }

    /// Is called when the file info for the requested file arrived.
    void onFileInfoAvailable(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo>& fileInfoList) {
    	if(fileInfoList.empty() == false ) {
    		if(fileInfoList[0].sizeValid) {
    			setProgressRange(fileInfoList[0].size / 1000);
    		}
    	}
    }

protected:

    void timerEvent(QTimerEvent* event) override {
    	if(_localFile) {
    		qint64 size = _localFile->size();
    		if(size >= 0 && progressMaximum() > 0) {
    			setProgressValue(size / 1000);
    		}
        	if(isCanceled()) {
        		QScopedPointer<QTemporaryFile> file(_localFile.take());
        		shutdown();
        	}
    	}
    }

private:

	/// The URL of the file to be downloaded.
	QUrl _url;

	/// The SSH connection.
	QSsh::SshConnection* _connection;

	/// The SFTP channel.
    QSsh::SftpChannel::Ptr _sftpChannel;

    /// The local copy of the file.
    QScopedPointer<QTemporaryFile> _localFile;

    /// The SFTP file download job.
    QSsh::SftpJobId _downloadJob;

    /// The progress monitor timer.
    int _timerId;

	/// Cache of login/password information for remote machines.
	static QMap<QString, QPair<QString,QString>> _loginCache;
};
QMap<QString, QPair<QString,QString>> SftpFileFetcher::_loginCache;

#include "FileManager.moc"

/******************************************************************************
* Creates a future that downloads the given remote file.
******************************************************************************/
Future<QString> FileManager::fetchRemoteFile(const QUrl& url)
{
	if(url.scheme() != "sftp")
		throw Exception(tr("Invalid or unsupported URL scheme %1 in URL %2").arg(url.scheme()).arg(url.toString()));

	std::shared_ptr<SftpFileFetcher> futureInterface(new SftpFileFetcher(url));
	return Future<QString>(futureInterface);
}

};
