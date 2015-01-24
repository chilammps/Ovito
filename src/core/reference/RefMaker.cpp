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
#include <core/reference/RefMaker.h>
#include <core/reference/RefTarget.h>
#include <core/dataset/UndoStack.h>
#include <core/plugins/Plugin.h>
#include <core/utilities/io/ObjectSaveStream.h>
#include <core/utilities/io/ObjectLoadStream.h>
#include <core/animation/controller/Controller.h>
#include <core/animation/AnimationSettings.h>

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem)

// Gives the class run-time type information.
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, RefMaker, OvitoObject);

/******************************************************************************
* This method is called when the reference counter of this OvitoObject
* has reached zero.
******************************************************************************/
void RefMaker::aboutToBeDeleted()
{
	OVITO_CHECK_OBJECT_POINTER(this);

	// Make sure undo recording is not active while deleting a RefTarget.
	OVITO_ASSERT_MSG(!isRefTarget() || dataset()->undoStack().isRecording() == false, "RefMaker::aboutToBeDeleted()", "Cannot delete object from memory while undo recording is active.");

	// Clear all references this object has to other objects.
	clearAllReferences();

	OvitoObject::aboutToBeDeleted();
}

/******************************************************************************
* Returns the value stored in a non-animatable property field of this RefMaker object.
******************************************************************************/
QVariant RefMaker::getPropertyFieldValue(const PropertyFieldDescriptor& field) const
{
	OVITO_ASSERT_MSG(!field.isReferenceField(), "RefMaker::getPropertyFieldValue", "This function may be used only to access property fields and not reference fields."); 
	OVITO_ASSERT_MSG(getOOType().isDerivedFrom(*field.definingClass()), "RefMaker::getPropertyFieldValue", "The property field has not been defined in this class or its base classes.");
	OVITO_ASSERT(field.propertyStorageReadFunc != nullptr);
	return field.propertyStorageReadFunc(const_cast<RefMaker*>(this));
}

/******************************************************************************
* Sets the value stored in a non-animatable property field of this RefMaker object.
******************************************************************************/
void RefMaker::setPropertyFieldValue(const PropertyFieldDescriptor& field, const QVariant& newValue)
{
	OVITO_ASSERT_MSG(!field.isReferenceField(), "RefMaker::setPropertyFieldValue", "This function may be used only to access property fields and not reference fields."); 
	OVITO_ASSERT_MSG(getOOType().isDerivedFrom(*field.definingClass()), "RefMaker::setPropertyFieldValue", "The property field has not been defined in this class or its base classes.");
	OVITO_ASSERT(field.propertyStorageWriteFunc != nullptr);
	field.propertyStorageWriteFunc(this, newValue);
}

/******************************************************************************
* Looks up the reference field.
******************************************************************************/
const SingleReferenceFieldBase& RefMaker::getReferenceField(const PropertyFieldDescriptor& field) const
{
	OVITO_ASSERT_MSG(field.isReferenceField(), "RefMaker::getReferenceField", "This function may not be used to retrieve property fields."); 
	OVITO_ASSERT_MSG(field.isVector() == false, "RefMaker::getReferenceField", "This function may not be used to retrieve vector reference fields."); 
	OVITO_ASSERT_MSG(getOOType().isDerivedFrom(*field.definingClass()), "RefMaker::getReferenceField", "The reference field has not been defined in this class or its base classes.");
	OVITO_ASSERT(field.singleStorageAccessFunc != nullptr);
	return field.singleStorageAccessFunc(const_cast<RefMaker*>(this));
}

/******************************************************************************
* Looks up the vector reference field.
******************************************************************************/
const VectorReferenceFieldBase& RefMaker::getVectorReferenceField(const PropertyFieldDescriptor& field) const
{
	OVITO_ASSERT_MSG(field.isReferenceField(), "RefMaker::getVectorReferenceField", "This function may not be used to retrieve property fields."); 
	OVITO_ASSERT_MSG(field.isVector() == true, "RefMaker::getVectorReferenceField", "This function may not be used to retrieve single reference fields."); 
	OVITO_ASSERT_MSG(getOOType().isDerivedFrom(*field.definingClass()), "RefMaker::getVectorReferenceField", "The reference field has not been defined in this class or its base classes.");
	OVITO_ASSERT(field.vectorStorageAccessFunc != nullptr);
	return field.vectorStorageAccessFunc(const_cast<RefMaker*>(this));
}

/******************************************************************************
* Handles a notification event from a RefTarget referenced by this object.
******************************************************************************/
bool RefMaker::handleReferenceEvent(RefTarget* source, ReferenceEvent* event)
{
	OVITO_CHECK_OBJECT_POINTER(this);
	
	// Handle delete messages.
	if(event->type() ==  ReferenceEvent::TargetDeleted) {
		OVITO_ASSERT(source == event->sender());
		referenceEvent(source, event);
		OVITO_CHECK_OBJECT_POINTER(this);
		clearReferencesTo(event->sender());
		return false;
	}

	// Let the RefMaker-derived class process the message.
	return referenceEvent(source, event);
}

/******************************************************************************
* Checks if this RefMaker has any reference to the given RefTarget.
******************************************************************************/
bool RefMaker::hasReferenceTo(RefTarget* target) const
{
	if(!target) return false;
	OVITO_CHECK_OBJECT_POINTER(target);
	
	for(const OvitoObjectType* clazz = &getOOType(); clazz; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
			if(!field->isReferenceField()) continue;
			if(field->isVector() == false) {
				if(getReferenceField(*field) == target) 
					return true;
			}
			else {
				if(getVectorReferenceField(*field).contains(target)) 
					return true;
			}
		}
	}
	return false;
}

/******************************************************************************
* Replaces all references of this RefMaker to the old RefTarget with 
* the new RefTarget.
******************************************************************************/
void RefMaker::replaceReferencesTo(RefTarget* oldTarget, RefTarget* newTarget)
{
	if(oldTarget == NULL) return;
	OVITO_CHECK_OBJECT_POINTER(oldTarget);

	// Check for cyclic references first.
	if(newTarget && isReferencedBy(newTarget))
		throw CyclicReferenceError();

	// Iterate over all reference fields in the class hierarchy.
	for(const OvitoObjectType* clazz = &getOOType(); clazz; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
			if(!field->isReferenceField()) continue;
			if(field->isVector() == false) {
				SingleReferenceFieldBase& singleField = field->singleStorageAccessFunc(this);
				if(singleField == oldTarget)
					singleField.setValue(newTarget);
			}
			else {
				VectorReferenceFieldBase& vectorField = field->vectorStorageAccessFunc(this);
				for(int i=vectorField.size(); i--;) {
					if(vectorField[i] == oldTarget) {
						vectorField.remove(i);
						vectorField.insertInternal(newTarget, i);
					}
				}
			}
		}
	}	
}

/******************************************************************************
* Stops observing a RefTarget object. 
* All single reference fields containing the RefTarget will be reset to NULL.
* If the target is referenced in a vector reference field then the item is
* removed from the vector.
******************************************************************************/
void RefMaker::clearReferencesTo(RefTarget* target) 
{ 
	if(target == NULL) return;
	OVITO_CHECK_OBJECT_POINTER(target);

	// Iterate over all reference fields in the class hierarchy.
	for(const OvitoObjectType* clazz = &getOOType(); clazz; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
			if(!field->isReferenceField()) continue;
			if(field->isVector() == false) {
				SingleReferenceFieldBase& singleField = field->singleStorageAccessFunc(this);
				if(singleField == target)
					singleField.setValue(NULL);
			}
			else {
				VectorReferenceFieldBase& vectorField = field->vectorStorageAccessFunc(this);
				for(int i=vectorField.size(); i--;) {
					if(vectorField[i] == target)
						vectorField.remove(i);
				}
			}
		}
	}
}

/******************************************************************************
* Clears all references held by this RefMarker. 
******************************************************************************/
void RefMaker::clearAllReferences()
{
	OVITO_CHECK_OBJECT_POINTER(this);
	OVITO_ASSERT_MSG(getOOType() != RefMaker::OOType, "RefMaker::clearAllReferences", "clearAllReferences() must not be called from the RefMaker destructor.");

	// Iterate over all reference fields in the class hierarchy.
	for(const OvitoObjectType* clazz = &getOOType(); clazz; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field; field = field->next()) {
			if(field->isReferenceField())
				clearReferenceField(*field);
		}
	}
}

/******************************************************************************
* Clears the given reference field. 
* If this is a single reference field then it is set to NULL. 
* If it is a list reference field the all references are removed.
******************************************************************************/
void RefMaker::clearReferenceField(const PropertyFieldDescriptor& field)
{
	OVITO_ASSERT_MSG(field.isReferenceField(), "RefMaker::clearReferenceField", "This function may not be used for property fields."); 
	OVITO_ASSERT_MSG(getOOType().isDerivedFrom(*field.definingClass()), "RefMaker::clearReferenceField()", "The reference field has not been defined in this class or its base classes.");

	if(field.isVector() == false)
		field.singleStorageAccessFunc(this).setValue(nullptr);
	else
		field.vectorStorageAccessFunc(this).clear();
}

/******************************************************************************
* Saves the class' contents to the given stream. 
******************************************************************************/
void RefMaker::saveToStream(ObjectSaveStream& stream)
{
	OvitoObject::saveToStream(stream);

#if 0
	qDebug() << "Saving object" << this;
#endif

	// Iterate over all property fields in the class hierarchy.
	for(const OvitoObjectType* clazz = &getOOType(); clazz != nullptr; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field != nullptr; field = field->next()) {

			if(field->isReferenceField()) {
				// Write the object pointed to by the reference field to the stream.
				
				if(field->targetClass()->isSerializable()) {
					// Write reference target object to stream.
					stream.beginChunk(0x02);
					try {
						if(field->isVector() == false) {
							stream.saveObject(getReferenceField(*field));
						}
						else {
							const QVector<RefTarget*>& list = getVectorReferenceField(*field);
							stream << (qint32)list.size();
							Q_FOREACH(RefTarget* target, list)
								stream.saveObject(target);
						}
					}
					catch(Exception& ex) {
						throw ex.prependGeneralMessage(tr("Failed to serialize contents of reference field %1 of class %2.").arg(field->identifier()).arg(field->definingClass()->name()));
					}
				}
				else {
					// Write special chunk for non-serializable objects.
					stream.beginChunk(0x03);
				}
				stream.endChunk();
			}
			else {
				// Write the primitive value stored in the property field to the stream.
				OVITO_ASSERT(field->propertyStorageSaveFunc != nullptr);
				stream.beginChunk(0x04);
				field->propertyStorageSaveFunc(this, stream);
#if 0
				qDebug() << "  Property field" << field->identifier() << " contains" << field->propertyStorageReadFunc(this);
#endif
				stream.endChunk();
			}
		}
	}
}

/******************************************************************************
* Loads the class' contents from the given stream. 
******************************************************************************/
void RefMaker::loadFromStream(ObjectLoadStream& stream)
{
	OvitoObject::loadFromStream(stream);
	OVITO_ASSERT(!dataset()->undoStack().isRecording());
	OVITO_ASSERT(stream._currentObject && stream._currentObject->object == this);

#if 0
	qDebug() << "Loading object" << this;
#endif

	// Read property field from the stream.
	Q_FOREACH(const ObjectLoadStream::SerializedPropertyField& fieldEntry, stream._currentObject->pluginClass->propertyFields) {
		if(fieldEntry.isReferenceField) {
			OVITO_ASSERT(fieldEntry.targetClass != nullptr);
	
			// Parse target object(s).
			int chunkId = stream.openChunk();
			if(fieldEntry.targetClass->isSerializable() && chunkId == 0x02) {

				// Parse target object chunk.
				if(fieldEntry.field != NULL) {
					OVITO_CHECK_POINTER(fieldEntry.field);
					OVITO_ASSERT(fieldEntry.field->isVector() == ((fieldEntry.field->flags() & PROPERTY_FIELD_VECTOR) != 0));
					OVITO_ASSERT(fieldEntry.targetClass->isDerivedFrom(*fieldEntry.field->targetClass()));
					if(fieldEntry.field->isVector() == false) {
						OORef<RefTarget> target = stream.loadObject<RefTarget>();
						if(target && !target->getOOType().isDerivedFrom(*fieldEntry.targetClass)) {
							throw Exception(tr("Incompatible object stored in reference field %1 of class %2. Expected class %3 but found class %4 in file.")
								.arg(QString(fieldEntry.identifier)).arg(fieldEntry.definingClass->name()).arg(fieldEntry.targetClass->name()).arg(target->getOOType().name()));
						}
#if 0
						qDebug() << "  Reference field" << fieldEntry.identifier << " contains" << target;
#endif
						fieldEntry.field->singleStorageAccessFunc(this).setValue(target);
					}
					else {
						// Get storage address of member variable.
						VectorReferenceFieldBase& refField = fieldEntry.field->vectorStorageAccessFunc(this);
						refField.clear();

						// Load each target object and store it in the list reference field.
						qint32 numEntries;
						stream >> numEntries;
						OVITO_ASSERT(numEntries >= 0);
						for(qint32 i=0; i<numEntries; i++) {
							OORef<RefTarget> target = stream.loadObject<RefTarget>();
							if(target && !target->getOOType().isDerivedFrom(*fieldEntry.targetClass)) {
								throw Exception(tr("Incompatible object stored in reference field %1 of class %2. Expected class %3 but found class %4 in file.")
									.arg(QString(fieldEntry.identifier)).arg(fieldEntry.definingClass->name(), fieldEntry.targetClass->name(), target->getOOType().name()));
							}
#if 0
							qDebug() << "  Vector reference field" << fieldEntry.identifier << " contains" << target;
#endif
							refField.insertInternal(target);
						}
					}
				}
				else {
#if 0
					qDebug() << "  Reference field" << fieldEntry.identifier << " no longer exists.";
#endif
					// The serialized reference field no longer exists in the current program version.
					// Load object from stream and release it immediately.
					if(fieldEntry.flags & PROPERTY_FIELD_VECTOR) {
						qint32 numEntries;
						stream >> numEntries;
						for(qint32 i = 0; i < numEntries; i++)
							stream.loadObject<RefTarget>();
					}
					else {
						stream.loadObject<RefTarget>();
					}
				}
			}
			else if(chunkId != 0x03) {
				throw Exception(tr("Expected non-serializable reference field '%1' in object %2").arg(QString(fieldEntry.identifier)).arg(fieldEntry.definingClass->name()));
			}
			stream.closeChunk();
		}
		else {
			// Read the primitive value of the property field from the stream.
			OVITO_ASSERT(fieldEntry.targetClass == nullptr);
			stream.expectChunk(0x04);
			if(!loadPropertyFieldFromStream(stream, fieldEntry)) {
				if(fieldEntry.field) {
					OVITO_ASSERT(fieldEntry.field->propertyStorageLoadFunc != nullptr);
					fieldEntry.field->propertyStorageLoadFunc(this, stream);
				}
				else {
					// The property field no longer exists.
					// Ignore chunk contents.
				}
			}
			stream.closeChunk();
		}
	}

#if 0
	qDebug() << "Done loading automatic fields of " << this;
#endif
}

/******************************************************************************
* Returns a list of all targets this RefMaker depends on (both
* directly and indirectly).
******************************************************************************/
QSet<RefTarget*> RefMaker::getAllDependencies() const
{
	QSet<RefTarget*> nodes;
	walkNode(nodes, this);
	return nodes;
}

/******************************************************************************
* Recursive gathering function.
******************************************************************************/
void RefMaker::walkNode(QSet<RefTarget*>& nodes, const RefMaker* node)
{
	OVITO_CHECK_OBJECT_POINTER(node);
	
	// Iterate over all reference fields in the class hierarchy.
	for(const OvitoObjectType* clazz = &node->getOOType(); clazz != nullptr; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field != nullptr; field = field->next()) {
			if(!field->isReferenceField()) continue;
			if(field->isVector() == false) {
				RefTarget* target = node->getReferenceField(*field);
				if(target != nullptr && !nodes.contains(target)) {
					nodes.insert(target);
					walkNode(nodes, target);
				}
			}
			else {
				const QVector<RefTarget*>& list = node->getVectorReferenceField(*field);
				Q_FOREACH(RefTarget* target, list) {
					if(target != nullptr && !nodes.contains(target)) {
						nodes.insert(target);
						walkNode(nodes, target);
					}
				}
			}
		}
	}
}

/******************************************************************************
* Loads the user-defined default values of this object's parameter fields from the
* application's settings store.
*
* This function should be called immediately after creation of the object instance.
* It loads the default value for every property field for which the user has set
* a default value. This is usually the case for property fields that have the
* PROPERTY_FIELD_MEMORIZE flag set.
*
* This function is recursive, i.e., it also loads default parameter values for
* referenced objects (when the PROPERTY_FIELD_MEMORIZE flag is set for this RefMaker's reference field).
******************************************************************************/
void RefMaker::loadUserDefaults()
{
	// Iterate over all property fields in the class hierarchy.
	for(const OvitoObjectType* clazz = &getOOType(); clazz != nullptr; clazz = clazz->superClass()) {
		for(const PropertyFieldDescriptor* field = clazz->firstPropertyField(); field != nullptr; field = field->next()) {
			if(field->flags().testFlag(PROPERTY_FIELD_MEMORIZE)) {
				if(field->isReferenceField()) {
					// If it's a reference field, recursively call loadUserDefaults() on the reference object(s).
					if(field->isVector() == false) {
						if(RefTarget* target = getReferenceField(*field)) {
							target->loadUserDefaults();

							// If it's a controller type, load default controller value.
							if(Controller* ctrl = dynamic_object_cast<Controller>(target)) {
								QSettings settings;
								settings.beginGroup(field->definingClass()->plugin()->pluginId());
								settings.beginGroup(field->definingClass()->name());
								QVariant v = settings.value(field->identifier());
								if(!v.isNull()) {
									if(ctrl->controllerType() == Controller::ControllerTypeFloat) {
										ctrl->setFloatValue(0, v.value<FloatType>());
									}
									else if(ctrl->controllerType() == Controller::ControllerTypeInt) {
										ctrl->setIntValue(0, v.value<int>());
									}
									else if(ctrl->controllerType() == Controller::ControllerTypeVector3) {
										ctrl->setVector3Value(0, v.value<Vector3>());
									}
								}
							}
						}
					}
					else {
						const QVector<RefTarget*>& list = getVectorReferenceField(*field);
						for(RefTarget* target : list) {
							if(target) target->loadUserDefaults();
						}
					}
				}
				else {
					// If it's a property field, load the user-defined default value.
					field->loadDefaultValue(this);
				}
			}
		}
	}
}

OVITO_END_INLINE_NAMESPACE
}	// End of namespace

