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
	Q_INVOKABLE LinkedFileObject();

	/// \brief Changes the parser associated with this object.
	/// \note After the replacing the parser with this method,
	///       reload() should be called to let the new parser reload the input file.
	virtual void setImporter(LinkedFileImporter* importer) { _importer = importer; }

	/// \brief Returns the parser that loads the input file.
	LinkedFileImporter* importer() const { return _importer; }

	/// \brief This reloads the input data from the external file.
	/// \param frame The animation frame to load from the external file.
	/// \param suppressDialogs Specifies whether any dialogs or message boxes should be suppressed during loading.
	/// \return \a false when the operation has been canceled by the user; \a true on success.
	/// \throws Exception on error.
	virtual bool refreshFromSource(int frame = 0, bool suppressDialogs = false);

	/// \brief Returns the status returned by the file parser on its last invocation.
	const ObjectStatus& status() const { return _importStatus; }

	/// \brief Returns whether the scene's animation interval is being adjusted to the number of frames reported by the file parser.
	bool adjustAnimationInterval() const { return _adjustAnimationInterval; }

	/// \brief Controls whether the scene's animation interval should be adjusted to the number of frames reported by the file parser.
	void setAdjustAnimationInterval(bool enabled) { _adjustAnimationInterval = enabled; }

	/// Asks the object for the result of the geometry pipeline at the given time.
	virtual PipelineFlowState evaluateNow(TimePoint time) override;

	/// Requests the results of a full evaluation of the geometry pipeline at the given time.
	virtual Future<PipelineFlowState> evaluateLater(TimePoint time) override;

#if 0
	/// \brief Returns whether the loaded scene objects should be saved in a scene file.
	/// \return \c true if a copy of the external data is stored in the scene file; \c false if the data resides only in the linked file.
	bool storeDataWithScene() const { return atomsObject() ? atomsObject()->serializeAtoms() : false; }

	/// \brief Controls whether the imported data is saved along with the scene.
	/// \param on \c true if data should be stored in the scene file; \c false if the data resides only in the external file.
	/// \undoable
	void setStoreDataWithScene(bool on) { if(atomsObject()) atomsObject()->setSerializeAtoms(on); }
#endif

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

	/// \brief Returns the number of animation frames per simulation snapshot.
	/// \return The number of animation frames per simulation snapshot. This is always equal or greater than 1.
	int framesPerSnapshot() const { return max((int)_framesPerSnapshot, 1); }

	/// \brief Sets the number of animation frames per simulation snapshot to control the playback speed.
	/// \param fps The number of animation frames per simulation snapshot. This must be equal or greater than 1.
	void setFramesPerSnapshot(int fps) { _framesPerSnapshot = fps; }

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

#endif

protected:

	/// \brief Saves the status returned by the parser object and generates a ReferenceEvent::StatusChanged event.
	void setStatus(const ObjectStatus& status);

	/// \brief Adjusts the animation interval of the current data set to the number of frames reported by the file parser.
	void adjustAnimationInterval();

	/// \brief Call the importer object to load the given frame.
	PipelineFlowState evaluateImplementation(FutureInterface<PipelineFlowState>& futureInterface, int frameIndex);

#if 0
	/// \brief Saves the class' contents to the given stream.
	/// \sa RefTarget::saveToStream()Atoms
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

#endif

private:

	/// The associated importer object that is responsible for parsing the input file.
	ReferenceField<LinkedFileImporter> _importer;

	/// Stores the imported scene objects.
	VectorReferenceField<SceneObject> _sceneObjects;

	/// Controls whether the scene's animation interval is adjusted to the number of frames found in the input file.
	PropertyField<bool> _adjustAnimationInterval;

	/// The status returned by the parser during its last call.
	ObjectStatus _importStatus;

	/// The index of the animation frame loaded last from the input file.
	int _loadedFrame;

	/// The index of the animation frame currently being loaded.
	int _frameBeingLoaded;

	/// The background operation created by evaluateLater().
	Future<PipelineFlowState> _evaluationOperation;

#if 0

	/// Controls the playback speed of simulation snapshots.
	PropertyField<int> _framesPerSnapshot;

#endif

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_importer)
	DECLARE_VECTOR_REFERENCE_FIELD(_sceneObjects)
	//DECLARE_PROPERTY_FIELD(_framesPerSnapshot)
	DECLARE_PROPERTY_FIELD(_adjustAnimationInterval)
};

};

#endif // __OVITO_LINKED_FILE_OBJECT_H
