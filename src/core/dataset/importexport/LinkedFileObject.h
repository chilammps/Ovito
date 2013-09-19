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
class OVITO_CORE_EXPORT LinkedFileObject : public SceneObject
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE LinkedFileObject();

	/// \brief Returns the parser that loads the input file.
	LinkedFileImporter* importer() const { return _importer; }

	/// \brief Sets the source location for importing data.
	/// \param sourceUrl The new source location.
	/// \param importerType The type of importer object that will parse the input file.
	/// \return false if the operation has been canceled by the user.
	bool setSource(const QUrl& sourceUrl, const FileImporterDescription* importerType);

	/// \brief Sets the source location for importing data.
	/// \param sourceUrl The new source location.
	/// \param importer The importer object that will parse the input file.
	/// \return false if the operation has been canceled by the user.
	bool setSource(QUrl sourceUrl, const OORef<LinkedFileImporter>& importer);

	/// \brief Returns the source location of the data.
	const QUrl& sourceUrl() const { return _sourceUrl; }

	/// \brief This reloads the input data from the external file.
	/// \param frame The animation frame to reload from the external file.
	void refreshFromSource(int frame = -1);

	/// \brief Returns the status returned by the file parser on its last invocation.
	virtual ObjectStatus status() const override { return _importStatus; }

	/// \brief Scans the inpframeut source for animation frames and updates the internal list of frames.
	bool updateFrames();

	/// \brief Returns the number of animation frames that can be loaded from the data source.
	int numberOfFrames() const { return _frames.size(); }

	/// \brief Returns the index of the animation frame loaded last from the input file.
	int loadedFrame() const { return _loadedFrame; }

	/// \brief Returns the list of animation frames in the input file(s).
	const QVector<LinkedFileImporter::FrameSourceInformation>& frames() const { return _frames; }

	/// \brief Returns whether the scene's animation interval is being adjusted to the number of frames reported by the file parser.
	bool adjustAnimationIntervalEnabled() const { return _adjustAnimationIntervalEnabled; }

	/// \brief Controls whether the scene's animation interval should be adjusted to the number of frames reported by the file parser.
	void setAdjustAnimationIntervalEnabled(bool enabled) { _adjustAnimationIntervalEnabled = enabled; }

	/// \brief Adjusts the animation interval of the current data set to the number of frames in the data source.
	void adjustAnimationInterval(int gotoFrameIndex = -1);

	/// \brief Asks the object for the result of the geometry pipeline at the given time.
	virtual PipelineFlowState evaluate(TimePoint time) override;

	/// \brief Returns the list of imported scene objects.
	const QVector<SceneObject*> sceneObjects() const { return _sceneObjects; }

	/// \brief Inserts a new object into the list of scene objects held by this container object.
	void addSceneObject(SceneObject* obj) {
		if(!_sceneObjects.contains(obj)) {
			obj->setSaveWithScene(saveDataWithScene());
			_sceneObjects.push_back(obj);
		}
	}

	/// \brief Looks for an object of the given type in the list of scene objects and returns it.
	template<class T>
	T* findSceneObject() const {
		for(SceneObject* obj : sceneObjects()) {
			T* castObj = dynamic_object_cast<T>(obj);
			if(castObj) return castObj;
		}
		return nullptr;
	}

	/// \brief Removes all scene objects owned by this LinkedFileObject that are not
	///        listed in the given set of active objects.
	void removeInactiveObjects(const QSet<SceneObject*>& activeObjects) {
		for(int index = _sceneObjects.size() - 1; index >= 0; index--)
			if(!activeObjects.contains(_sceneObjects[index]))
				_sceneObjects.remove(index);
	}

	/// \brief Returns whether the loaded scene objects should be saved in a scene file.
	/// \return \c true if a copy of the external data is stored in the scene file; \c false if the data resides only in the linked file.
	bool saveDataWithScene() const { return _saveDataWithScene; }

	/// \brief Controls whether the imported data is saved along with the scene.
	/// \param on \c true if data should be stored in the scene file; \c false if the data resides only in the external file.
	/// \undoable
	void setSaveDataWithScene(bool on) {
		_saveDataWithScene = on;
		for(SceneObject* sceneObj : sceneObjects())
			sceneObj->setSaveWithScene(on);
	}

	/// Returns the title of this object.
	virtual QString objectTitle() override;

	/// Returns the number of sub-objects that should be displayed in the modifier stack.
	virtual int editableSubObjectCount() override;

	/// Returns a sub-object that should be listed in the modifier stack.
	virtual RefTarget* editableSubObject(int index) override;

public Q_SLOTS:

	/// \brief Displays the file selection dialog and lets the user select a new input file.
	void showFileSelectionDialog(QWidget* parent = nullptr);

	/// \brief Displays the remote file selection dialog and lets the user select a new source URL.
	void showURLSelectionDialog(QWidget* parent = nullptr);

public:

	Q_PROPERTY(QUrl sourceUrl READ sourceUrl)
	Q_PROPERTY(bool saveDataWithScene READ saveDataWithScene WRITE setSaveDataWithScene)

protected Q_SLOTS:

	/// \brief This is called when the background loading operation has finished.
	void loadOperationFinished();

protected:

	/// \brief Saves the status returned by the parser object and generates a ReferenceEvent::StatusChanged event.
	void setStatus(const ObjectStatus& status);

	/// Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
	virtual void referenceInserted(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

	/// Is called when a RefTarget has been added to a VectorReferenceField of this RefMaker.
	virtual void referenceRemoved(const PropertyFieldDescriptor& field, RefTarget* newTarget, int listIndex) override;

	/// \brief Saves the class' contents to the given stream.
	virtual void saveToStream(ObjectSaveStream& stream) override;

	/// \brief Loads the class' contents from the given stream.
	virtual void loadFromStream(ObjectLoadStream& stream) override;

	/// \brief Cancels the current load operation if there is any in progress.
	void cancelLoadOperation();

private:

	/// The associated importer object that is responsible for parsing the input file.
	ReferenceField<LinkedFileImporter> _importer;

	/// Stores the imported scene objects.
	VectorReferenceField<SceneObject> _sceneObjects;

	/// Controls whether the scene's animation interval is adjusted to the number of frames found in the input file.
	PropertyField<bool> _adjustAnimationIntervalEnabled;

	/// Controls whether the imported data is saved along with the scene.
	/// If false, only the metadata is saved while the actual data reside in the external file.
	PropertyField<bool> _saveDataWithScene;

	/// The source file (may include a wild-card pattern).
	PropertyField<QUrl, QUrl, ReferenceEvent::TitleChanged> _sourceUrl;

	/// Stores the list of animation frames in the input file(s).
	QVector<LinkedFileImporter::FrameSourceInformation> _frames;

	/// The index of the animation frame loaded last from the input file.
	int _loadedFrame;

	/// The index of the animation frame currently being loaded.
	int _frameBeingLoaded;

	/// The background file loading task started by evaluate().
	Future<LinkedFileImporter::ImportTaskPtr> _loadFrameOperation;

	/// The watcher object that is used to monitor the background operation.
	FutureWatcher _loadFrameOperationWatcher;

	/// The status returned by the parser during its last call.
	ObjectStatus _importStatus;

private:

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_REFERENCE_FIELD(_importer);
	DECLARE_VECTOR_REFERENCE_FIELD(_sceneObjects);
	DECLARE_PROPERTY_FIELD(_adjustAnimationIntervalEnabled);
	DECLARE_PROPERTY_FIELD(_sourceUrl);
	DECLARE_PROPERTY_FIELD(_saveDataWithScene);
};

};

#endif // __OVITO_LINKED_FILE_OBJECT_H
