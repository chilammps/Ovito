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

#ifndef __OVITO_LINKED_FILE_IMPORTER_H
#define __OVITO_LINKED_FILE_IMPORTER_H

#include <core/Core.h>
#include <core/dataset/importexport/FileImporter.h>
#include <core/utilities/ObjectStatus.h>
#include <core/utilities/concurrent/Future.h>

namespace Ovito {

class LinkedFileObject;		// defined in LinkedFileObject.h

/**
 * \brief Base class for file parsers that can reload a file that has been imported into the scene.
 */
class LinkedFileImporter : public FileImporter
{
public:

	/// \brief This structures stores source information about an imported animation frame.
	struct FrameSourceInformation {

		/// The source file that contains the data of the animation frame.
		QUrl sourceFile;

		/// The byte offset into the source file where the data is stored.
		qint64 byteOffset;

		/// The line number in the source file where the data is stored, if the file has a text-based format.
		int lineNumber;

		/// The last modification time of the source file.
		/// This is used to detect changes of the source file, which let the stored byte offset become invalid.
		QDateTime lastModificationTime;
	};

	class ImportedData {
	public:
		/// Base destructor of virtual class.
		virtual ~ImportedData() {}

		/// Lets the data container insert the data it holds into the scene by creating
		/// appropriate scene objects.
		virtual void insertIntoScene(LinkedFileObject* destination) = 0;
	};

	typedef std::shared_ptr<ImportedData> ImportedDataPtr;

public:

	/// \brief Constructs a new instance of this class.
	LinkedFileImporter() {
		INIT_PROPERTY_FIELD(LinkedFileImporter::_sourceUrl);
		INIT_PROPERTY_FIELD(LinkedFileImporter::_loadedUrl);
	}

	///////////////////////////// from FileImporter /////////////////////////////

	/// \brief Imports the given file into the scene.
	/// \return \a true if the file has been imported; \a false if the import has been aborted by the user.
	/// \throws Exception when the import has failed.
	///
	/// This is the high-level method to import a file into the scene.
	///
	/// This method sets the input file path, lets the user adjust the parser settings in a dialog box,
	/// creates an ObjectNode in the scene, links it to a new AtomsImportObject, links this
	/// object to this parser and finally calls loadAtomsFile() on this parser.
	virtual bool importFile(const QUrl& sourceUrl, DataSet* scene, bool suppressDialogs = false) override;

	/////////////////////////////// from RefTarget ///////////////////////////////

	/// Returns the title of this object.
	virtual QString objectTitle() override;

	//////////////////////////// Specific methods ////////////////////////////////

	/// \brief Sets the source location for importing data.
	/// \param sourceUrl The new source location.
	void setSourceUrl(const QUrl& sourceUrl) { _sourceUrl = sourceUrl; }

	/// \brief Returns the source location of the import data.
	const QUrl& sourceUrl() const { return _sourceUrl; }

	/// \brief Returns the path to the file that has been loaded last.
	const QUrl& loadedUrl() const { return _loadedUrl; }

	/// \brief Opens the settings dialog for this importer.
	/// \param parent The parent window for the dialog box.
	/// \return \c true if the dialog has been approved by the user; \c false when the user has canceled the operation.
	///
	/// The default implementation of this method does not show any dialog and always returns \a true.
	/// If this method is overridden to show a dialog box then the method hasSettingsDialog() should
	/// be overridden too.
	///
	/// \note The source location has to be set via setSourceUrl() before calling this method.
	virtual bool showSettingsDialog(QWidget* parent) { return true; }

	/// \brief Returns whether this importer has a settings dialog box to let the user configure the import settings.
	/// \return \c true if a call to showSettingsDialog() will show a dialog box; \c false otherwise.
	///
	/// The default implementation returns \c false.
	virtual bool hasSettingsDialog() { return false; }

	/// \brief Reads the data from the input file(s).
	/// \param frame If the input file contains more than one animation frame then this parameter specifies
	///              the index of the frame to load (starting at 0). It must be less then the number of available frames
	///              reported by numberOfFrames().
	/// \return A future that will give access to the loaded data.
	virtual Future<ImportedDataPtr> load(int frame = 0);

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	/// \param suppressDialogs Specifies whether any dialogs or message boxes should be suppressed during this operation.
	///                        This parameter is set to true in non-GUI mode or when the parser is invoked from a script to not
	///                        interrupt the process.
	/// \throws Exception when an error occurred.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	///
	/// The default implementation of this method checks if the source URL contains a wild-card pattern.
	/// If yes, it scans the directory to find all matching files.
	virtual bool registerFrames(bool suppressDialogs = false);

	/// \brief Clears the list of animation frames.
	void resetFrames() { _frames.clear(); }

	/// \brief Records the storage location of an animation frame in the input file(s).
	void registerFrame(const FrameSourceInformation& frame) { _frames.push_back(frame); }

	/// \brief Returns the number of animation frames in the input file.
	/// \return The number of frames stored in the input file or 0 if the input filename has not been set.
	/// \note registerFrames() must be called first before the number of movie frames can be queried.
	virtual int numberOfFrames() { return sourceUrl().isEmpty() ? 0 : _frames.size(); }

public:

	Q_PROPERTY(QUrl sourceUrl READ sourceUrl WRITE setSourceUrl)
	Q_PROPERTY(QUrl loadedUrl READ loadedUrl)

protected:

	/// \brief Reads the data from the input file(s).
	virtual void loadImplementation(FutureInterface<ImportedDataPtr>& futureInterface, FrameSourceInformation frame) = 0;

private:

	/// The source file (may be a wild-card pattern).
	PropertyField<QUrl> _sourceUrl;

	/// The path of the currently loaded file.
	PropertyField<QUrl, QUrl, ReferenceEvent::TitleChanged> _loadedUrl;

	/// Stores the list of animation frames in the input file(s).
	QVector<FrameSourceInformation> _frames;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_sourceUrl);
	DECLARE_PROPERTY_FIELD(_loadedUrl);
};

};

#endif // __OVITO_LINKED_FILE_IMPORTER_H
