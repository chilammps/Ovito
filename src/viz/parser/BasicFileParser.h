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

#ifndef __OVITO_BASIC_FILE_PARSER_H
#define __OVITO_BASIC_FILE_PARSER_H

#include <core/Core.h>
#include <core/dataset/importexport/FileImporter.h>

namespace Viz {

/**
 * \brief Base class for .
 */
class BasicFileParser : public FileImporter
{
protected:

	/// \brief Constructs a new instance of this class.
	BasicFileParser() {
		INIT_PROPERTY_FIELD(BasicFileParser::_sourceUrl);
		INIT_PROPERTY_FIELD(BasicFileParser::_loadedUrl);
	}

public:

	////////////////////////// from ImporterExporter //////////////////////////

	/// \brief Imports the given file into the scene.
	/// \return \a true if the file has been imported; \a false if the import has been aborted by the user.
	/// \throws Exception when the import has failed.
	///
	/// This is the high-level method to load an atoms file into the scene.
	///
	/// This method sets the input file path, lets the user adjust the parser settings in a dialog box,
	/// creates an ObjectNode in the scene, links it to a new AtomsImportObject, links this
	/// object to this parser and finally calls loadAtomsFile() on this parser.
	virtual bool importFile(const QString& filePath, DataSet* scene, bool suppressDialogs = false);

	///////////////////////////// from RefTarget //////////////////////////////

	/// Returns the title of this object.
	virtual QString schematicTitle();

	/////////////////////////// Specific methods ///////////////////////////////

	/// \brief Sets the name of the input file for this parser.
	/// \param filename The path to the new input file.
	/// \throws Exception if the input file is invalid.
	/// \return \a true on success; or \a false when the operation has been aborted by the user.
	/// \sa inputFile()
	virtual bool setInputFile(const QString& filename) {
		_inputFilename = filename;
		setSourceFile(filename);
		return true;
	}

	/// \brief Returns the path to the input file set by the method setInputFile().
	/// \sa setInputFile()
	const QString& inputFile() const { return _inputFilename; }

	/// \brief Sets the name of the current source file.
	/// \param filename The path to the current source file.
	///
	/// The source file path is shown in the UI and has no other meaning.
	/// Use the setInputFile() function to actually set the file to be loaded.
	void setSourceFile(const QString& filename) { _sourceFilename = filename;  }

	/// \brief Returns the name/path of the source file that is currently loaded.
	const QString& sourceFile() const { return _sourceFilename; }

	/// \brief Opens the settings dialog for this parser.
	/// \return \a true if the dialog has been approved by the user; \a false when the user has canceled the operation.
	///
	/// The default implementation of this method does not show any dialog and always returns \a true.
	/// If this method is overridden to show a dialog box then the method hasSettingsDialog() should
	/// be overriden too.
	///
	/// \note The input file name has to be set via setInputFile() before this method may be called.
	/// \sa hasSettingsDialog()
	virtual bool showSettingsDialog(QWidget* parent) { return true; }

	/// \brief Returns whether this import filter provides a settings dialog box.
	/// \return \c true if a call to showSettingsDialog() will show a dialog box; \c otherwise.
	///
	/// The default implementation returns \c false.
	/// \sa showSettingsDialog
	virtual bool hasSettingsDialog() { return false; }

	/// \brief Prepares or scans the input file before it is actually loaded.
	/// \param suppressDialogs Specifies whether any dialogs or message boxes shown by the parser should be suppressed during preparation.
	///                        This parameter is set to true in non-GUI mode or when the parser is invoked from a script to not
	///                        interrupt the parsing process.
	/// \throws Exception on error.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	///
	/// The default implementation of this method just returns \a true.
	virtual bool prepareInputFile(bool suppressDialogs = false) { return true; }

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
	virtual EvaluationStatus loadAtomsFile(AtomsObject* destination, int movieFrame = 0, bool suppressDialogs = false) = 0;

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

public:

	Q_PROPERTY(QUrl sourceUrl READ sourceUrl WRITE setSourceUrl)
	Q_PROPERTY(QUrl loadedUrl READ loadedUrl)

private:

	/// The source file (may be a wild-card pattern).
	PropertyField<QUrl> _sourceUrl;

	/// The path of the currently loaded file.
	PropertyField<QUrl, QUrl, ReferenceField::TitleChanged> _loadedUrl;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_sourceUrl);
	DECLARE_PROPERTY_FIELD(_loadedUrl);
};

};

#endif // __OVITO_BASIC_FILE_PARSER_H
