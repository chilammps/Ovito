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
#include <core/dataset/importexport/LinkedFileObject.h>
#include <viz/data/SimulationCell.h>
#include <viz/data/ParticleProperty.h>
#include <viz/data/ParticlePropertyObject.h>
#include <viz/data/ParticleTypeProperty.h>
#include <viz/data/ParticleDisplay.h>
#include <viz/data/ParticleType.h>
#include "ParticleImportData.h"
#include "ParticleImporter.h"

namespace Viz {

/******************************************************************************
* Lets the data container insert the data it holds into the scene by creating
* appropriate scene objects.
******************************************************************************/
void ParticleImportData::insertIntoScene(LinkedFileObject* destination)
{
	QSet<SceneObject*> activeObjects;

	// Adopt simulation cell.
	OORef<SimulationCell> cell = destination->findSceneObject<SimulationCell>();
	if(!cell) {
		cell = new SimulationCell(simulationCell());
		destination->addSceneObject(cell.get());
	}
	else {
		cell->setData(simulationCell());
	}
	activeObjects.insert(cell.get());

	// Adopt particle properties.
	for(auto& property : _properties) {
		OORef<ParticlePropertyObject> propertyObj;
		for(const auto& sceneObj : destination->sceneObjects()) {
			ParticlePropertyObject* po = dynamic_object_cast<ParticlePropertyObject>(sceneObj);
			if(po != nullptr && po->type() == property->type() && po->name() == property->name()) {
				propertyObj = po;
				break;
			}
		}
		if(propertyObj)
			propertyObj->replaceStorage(QSharedDataPointer<ParticleProperty>(property.release()));
		else {
			propertyObj = ParticlePropertyObject::create(QSharedDataPointer<ParticleProperty>(property.release()));
			destination->addSceneObject(propertyObj.get());
		}
		if(propertyObj->type() == ParticleProperty::ParticleTypeProperty)
			insertParticleTypes(propertyObj.get());
		activeObjects.insert(propertyObj.get());
	}

	destination->removeInactiveObjects(activeObjects);
}

/******************************************************************************
* Inserts the stores particle types into the given destination object.
******************************************************************************/
void ParticleImportData::insertParticleTypes(ParticlePropertyObject* propertyObj)
{
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(propertyObj);
	if(!typeProperty)
		return;

	QSet<ParticleType*> activeTypes;
	for(const auto& mapitem : _particleTypes) {
		OORef<ParticleType> ptype = typeProperty->particleType(mapitem.second.id);
		if(ptype == nullptr) {
			ptype = new ParticleType();
			ptype->setId(mapitem.second.id);

			// Assign initial standard color to new particle types.
			static const Color defaultTypeColors[] = {
				Color(0.4f,1.0f,0.4f),
				Color(1.0f,0.4f,0.4f),
				Color(0.4f,0.4f,1.0f),
				Color(1.0f,1.0f,0.7f),
				Color(0.97f,0.97f,0.97f),
				Color(1.0f,1.0f,0.0f),
				Color(1.0f,0.4f,1.0f),
				Color(0.7f,0.0f,1.0f),
				Color(0.2f,1.0f,1.0f),
			};
			ptype->setColor(defaultTypeColors[std::abs(ptype->id()) % (sizeof(defaultTypeColors) / sizeof(defaultTypeColors[0]))]);

			typeProperty->insertParticleType(ptype);
		}
		activeTypes.insert(ptype.get());

		if(!mapitem.second.name.isEmpty())
			ptype->setName(mapitem.second.name);
		else if(ptype->name().isEmpty())
			ptype->setName(ParticleImporter::tr("Type %1").arg(mapitem.second.id));

		if(mapitem.second.color != Color(0,0,0))
			ptype->setColor(mapitem.second.color);

		if(mapitem.second.radius != 0)
			ptype->setRadius(mapitem.second.radius);
	}

	// Remove unused particle types.
	for(int index = typeProperty->particleTypes().size() - 1; index >= 0; index--) {
		if(!activeTypes.contains(typeProperty->particleTypes()[index]))
			typeProperty->removeParticleType(index);
	}
}

};
