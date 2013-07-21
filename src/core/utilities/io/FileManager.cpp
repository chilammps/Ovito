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
#include <ssh/SshConnection.h>
#include <ssh/SshConnectionManager.h>
#include "FileManager.h"

namespace Ovito {

/// The singleton instance of the class.
FileManager* FileManager::_instance = nullptr;

/******************************************************************************
* Constructor.
******************************************************************************/
FileManager::FileManager()
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
		return Future<QString>(url.toLocalFile(), tr("Loading URL %1").arg(url.toString()));
	}
	else if(url.scheme() == "sftp") {
		QMutexLocker lock(&_mutex);

		// Check if requested URL is already in the cache.
		auto cacheEntry = _cachedFiles.find(url);
		if(cacheEntry != _cachedFiles.end())
			return Future<QString>(cacheEntry.value()->fileName(), tr("Loading URL %1").arg(url.toString()));

		// Check if requested URL is already being loaded.
		auto inProgressEntry = _pendingFiles.find(url);
		if(inProgressEntry != _pendingFiles.end())
			return inProgressEntry.value();

		// Fetch remote file.
		Future<QString> future = FileManager::fetchRemoteFile(url);
		ProgressManager::instance().addTask(future);
		_pendingFiles.insert(url, future);
		return future;
	}
	else throw Exception(tr("URL scheme not supported. The program supports only the sftp:// scheme and loading of local files."));
}

/******************************************************************************
* Future object that is responsible for downloading a file via sftp protocol.
******************************************************************************/
class SftpFileFetcher : public QObject, public FutureInterface<QString>
{
public:

	/// Constructor.
	SftpFileFetcher(const QUrl& url) : _url(url), _connection(nullptr) {

		reportStarted();

		QSsh::SshConnectionParameters connectionParams;
		connectionParams.host = url.host();
		connectionParams.userName = url.userName();
		connectionParams.password = url.password();
		connectionParams.port = url.port(22);

		// Open connection
		_connection = QSsh::SshConnectionManager::instance().acquireConnection(connectionParams);
		OVITO_CHECK_POINTER(_connection);

		// Listen for signals of the connection.
		connect(_connection, SIGNAL(error(QSsh::SshError)), this, SLOT(onSshConnectionError(QSsh::SshError))));
		if(_connection->state() == QSsh::SshConnection::Connected) {
			onSshConnectionEstablished();
			return;
	    }
		QObject::connect(_connection, SIGNAL(connected()), this, SLOT(onSshConnectionEstablished()));
		if(_connection->state() == QSsh::SshConnection::Unconnected)
			_connection->connectToHost();
	}

	/// Destructor.
	virtual ~SftpFileFetcher() {
		shutdown();
	}

	void shutdown() {
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
		reportFinished();
	}

private Q_SLOTS:

	/// Handles SSH connection errors.
	void onSshConnectionError(QSsh::SshError error) {
		try { throw Exception(_connection->errorString()); }
		catch(Exception& ex) {
			reportException();
		}
		shutdown();
	}

	/// Is called when the SSH connection has been established.
    void onSshConnectionEstablished() {
    	if(isCanceled()) {
    		shutdown();
    		return;
    	}

    	_sftpChannel = _connection->createSftpChannel();
        connect(_sftpChannel.data(), SIGNAL(initialized()), SLOT(onSftpChannelInitialized()));
        connect(_sftpChannel.data(), SIGNAL(initializationFailed(const QString&)), SLOT(onSftpChannelInitializationFailed(const QString&)));
        _sftpChannel->initialize();
    }

    /// Is called when the SFTP channel could not be created.
    void onSftpChannelInitializationFailed(const QString& reason) {
		try { throw Exception(reason); }
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


    }

private:

	/// The URL of the file to be downloaded.
	QUrl _url;

	/// The SSH connection.
	QSsh::SshConnection* _connection;

	/// The SFTP channel.
    QSsh::SftpChannel::Ptr _sftpChannel;
};

/******************************************************************************
* Creates a future that downloads the given remote file.
******************************************************************************/
Future<QString> FileManager::fetchRemoteFile(const QUrl& url)
{
	if(url.scheme() != "sftp")
		throw Exception(tr("Invalid or unsupported URL scheme %1 in URL %2").arg(url.scheme()).arg(url.toString()));

	std::shared_ptr<FutureInterface<QString>> futureInterface(new SftpFileFetcher(url));
	return Future<QString>(futureInterface);
}


};
