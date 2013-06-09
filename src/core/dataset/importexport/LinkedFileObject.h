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

#ifndef __OVITO_LINKED_FILE_OBJECT_H
#define __OVITO_LINKED_FILE_OBJECT_H

#include <core/Core.h>
#include <core/scene/objects/SceneObject.h>
#include "LinkedFileImporter.h"

namespace Ovito {

/**
 * \brief A place holder object that feeds data read from an external file into the scene.
 *
 * This class is always used in conjunction with a LinkedFileImporter class.
 */
class LinkedFileObject : public SceneObject
{
public:

	/// \brief Constructs a new instance of this class.
	LinkedFileObject();

	/// \brief Changes the parser associated with this object.
	/// \note After the replacing the parser with this method,
	///       reload() should be called to let the new parser reload the input file.
	virtual void setImporter(LinkedFileImporter* importer);

	/// \brief Returns the parser that loads the input file.
	LinkedFileImporter* importer() const { return _importer; }

	/// \brief This reloads the input data from the external file.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	/// \throws Exception on error.
	virtual bool refreshFromSource();

#if 0
	/// \brief Returns the movie frame that is currently loaded.
	///
	/// Usually this is the animation frame that was requested from evalObject() the last time.
	/// The atomic configuration for the currently loaded movie frame can be accessed via atomsObject().
	int loadedMovieFrame() const { return _loadedMovieFrame; }

	/// \brief Returns the path to the input file.
	/// \return The path to the input file. This is the same as AtomsFileParser::inputFile() returns.
	/// \sa AtomsFileParser::inputFile()
	QString inputFile() const { return parser() ? parser()->inputFile() : QString(); }

	/// \brief Returns the name of the current source file.
	/// \return The name/path of the file from which the current snapshot was loaded. This is the same as AtomsFileParser::sourceFile() returns.
	/// \sa AtomsFileParser::sourceFile()
	QString sourceFile() const { return parser() ? parser()->sourceFile() : QString(); }


	/// \brief Return the status returned by the parser during its last call.
	const EvaluationStatus& status() const { return _loadStatus; }

	/// \brief Returns whether atomic coordinates are saved along with the scene.
	/// \return \c true if data is stored in the scene file; \c false if the data resides in an external file.
	bool storeAtomsWithScene() const { return atomsObject() ? atomsObject()->serializeAtoms() : false; }

	/// \brief Returns whether atomic coordinates are saved along with the scene.
	/// \param on \c true if data should be stored in the scene file; \c false if the data resides in an external file.
	/// \undoable
	void setStoreAtomsWithScene(bool on) { if(atomsObject()) atomsObject()->setSerializeAtoms(on); }

	/// \brief Returns the number of animation frames per simulation snapshot.
	/// \return The number of animation frames per simulation snapshot. This is always equal or greater than 1.
	int framesPerSnapshot() const { return max((int)_framesPerSnapshot, 1); }

	/// \brief Sets the number of animation frames per simulation snapshot to control the playback speed.
	/// \param fps The number of animation frames per simulation snapshot. This must be equal or greater than 1.
	void setFramesPerSnapshot(int fps) { _framesPerSnapshot = fps; }

	/// Returns whether the scene's animation interval is being adjusted to the number of frames stored in the input file.
	bool adjustAnimationInterval() const { return _adjustAnimationInterval; }

	/// Sets whether the scene's animation interval should be adjusted to the number of frames stored in the input file.
	void setAdjustAnimationInterval(bool enabled) { _adjustAnimationInterval = enabled; }

	// From SceneObject:

	/// Asks the object for its validity interval at the given time.
	virtual TimeInterval objectValidity(TimeTicks time);
	/// Makes the object render itself into the viewport.
	virtual void renderObject(TimeTicks time, ObjectNode* contextNode, Viewport* vp) {}
	/// Returns the bounding box of the object in local object coordinates.
	virtual Box3 boundingBox(TimeTicks time, ObjectNode* contextNode) { return Box3(); }

	/// Asks the object for the result of the geometry pipeline at the given time.
	virtual PipelineFlowState evalObject(TimeTicks time);

	// From RefTarget:

	/// Returns the title of this object.
	virtual QString schematicTitle();

	/// Returns the number of sub-objects that should be displayed in the modifier stack.
	virtual int editableSubObjectCount();

	/// Returns a sub-object that should be listed in the modifier stack.
	virtual RefTarget* editableSubObject(int index);


public Q_SLOTS:

	/// \brief Displays the file selection dialog and lets the user select a new input file.
	void showSelectionDialog(QWidget* parent = NULL);

public:

	Q_PROPERTY(QString inputFile READ inputFile)
	Q_PROPERTY(bool storeAtomsWithScene READ storeAtomsWithScene WRITE setStoreAtomsWithScene)

protected:

	/// \brief Stores the parser status and sends a notification message.
	void setStatus(const EvaluationStatus& status);

	/// \brief Saves the class' contents to the given stream.
	/// \sa RefTarget::saveToStream()
	virtual void saveToStream(ObjectSaveStream& stream);

	/// \brief Loads the class' contents from the given stream.
	/// \sa RefTarget::loadFromStream()
	virtual void loadFromStream(ObjectLoadStream& stream);

	/// \brief This method is called once for this object after it has been loaded from the input stream
	virtual void loadFromStreamComplete();

	/// \brief Creates a copy of this object.
	/// \sa RefTarget::clone()
	virtual RefTarget::SmartPtr clone(bool deepCopy, CloneHelper& cloneHelper);

	/// This method is called when an object referenced by this object
	/// sends a notification message.
	virtual bool onRefTargetMessage(RefTarget* source, RefTargetMessage* msg);

private:

#endif

	/// The associated importer object that is responsible for parsing the input file.
	ReferenceField<LinkedFileImporter> _importer;

	/// Stores the imported scene objects.
	VectorReferenceField<SceneObject> _sceneObjects;

#if 0
	/// The status returned by the parser during its last call.
	EvaluationStatus _loadStatus;

	/// Controls the playback speed of simulation snapshots.
	PropertyField<int> _framesPerSnapshot;

	/// Controls whether the scene's animation interval is adjusted to the number of frames stored in the input file.
	PropertyField<bool> _adjustAnimationInterval;
#endif

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_importer)
	DECLARE_VECTOR_REFERENCE_FIELD(_sceneObjects)
	//DECLARE_PROPERTY_FIELD(_framesPerSnapshot)
	//DECLARE_PROPERTY_FIELD(_adjustAnimationInterval)
};

};

#endif // __OVITO_LINKED_FILE_OBJECT_H
