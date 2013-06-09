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

namespace Ovito {

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

protected:

	/// \brief Constructs a new instance of this class.
	LinkedFileImporter() {
		INIT_PROPERTY_FIELD(LinkedFileImporter::_sourceUrl);
		INIT_PROPERTY_FIELD(LinkedFileImporter::_loadedUrl);
	}

public:

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

	/// \brief Changes the source location for the imported data.
	/// \param sourceUrl The new source location.
	/// \throws Exception if the new input file is invalid.
	/// \return \a true on success; or \a false when the operation has been aborted by the user.
	virtual bool setSourceUrl(const QUrl& sourceUrl) {
		_sourceUrl = sourceUrl;
		return true;
	}

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

#if 0
	/// \brief Reads an atomic data set from the input file.
	/// \param destination Destination for the atomic data. The parser should store all atoms in the data channels of this AtomsObject.
	/// \param movieFrame If the input file contains more than one movie frame then this parameter specifies
	///                   the index of the movie frame to load (starting at 0). It must be less then the number of available frames
	///                   reported by numberOfMovieFrames().
	/// \param suppressDialogs Specifies whether any dialogs or message boxes shown by the parser should be suppressed during loading.
	///                        This parameter will is set to true in non-GUI mode or when the parser is invoked from a script to not
	///                        interrupt the parsing process.
	/// \return A status object with EvaluationStatus::EVALUATION_ERROR when the operation has been canceled by the user.
	/// \throws Exception on error.
	///
	/// Before the atoms file can be loaded the method setInputFile() and prepareInputFile() must have been called at least
	/// once.
	///
	/// \sa numberOfMovieFrames()
	virtual ObjectStatus loadAtomsFile(AtomsObject* destination, int movieFrame = 0, bool suppressDialogs = false) = 0;

	/// \brief Returns the number of movie frames in the input file.
	/// \return The number of movie frames stored in the input file or 0 if the input filename has not been set.
	///
	/// This method should be overridden by sub-classes if they support multi-frame atoms files. The
	/// default implementation of this method returns 1.
	///
	/// \note prepareInputFile() must be called before the number of movie frames can be queried.
	///
	/// \sa loadAtomsFile()
	virtual int numberOfMovieFrames() { return inputFile().isEmpty() ? 0 : 1; }
#endif

	/// \brief Scans the input source (which can be a directory or a single file) to discover all animation frames.
	/// \param suppressDialogs Specifies whether any dialogs or message boxes should be suppressed during this operation.
	///                        This parameter is set to true in non-GUI mode or when the parser is invoked from a script to not
	///                        interrupt the process.
	/// \throws Exception when an error occurred.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	///
	/// The default implementation of this method checks if the source URL contains a wild-card pattern.
	/// If yes, it scans the directory to find all matching files.
	virtual bool discoverFrames(bool suppressDialogs = false);

	/// \brief Clears the list of animation frames.
	void resetFrames() { _frames.clear(); }

	/// \brief Records the storage location of an animation frame in the input file(s).
	void registerFrame(const FrameSourceInformation& frame) { _frames.push_back(frame); }

public:

	Q_PROPERTY(QUrl sourceUrl READ sourceUrl WRITE setSourceUrl)
	Q_PROPERTY(QUrl loadedUrl READ loadedUrl)

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
