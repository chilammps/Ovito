///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
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

#include <plugins/particles/Particles.h>
#include <core/viewport/Viewport.h>
#include "StructureIdentificationModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Analysis)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, StructureIdentificationModifier, AsynchronousParticleModifier);
DEFINE_VECTOR_REFERENCE_FIELD(StructureIdentificationModifier, _structureTypes, "StructureTypes", ParticleType);
SET_PROPERTY_FIELD_LABEL(StructureIdentificationModifier, _structureTypes, "Structure types");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
StructureIdentificationModifier::StructureIdentificationModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset)
{
	INIT_PROPERTY_FIELD(StructureIdentificationModifier::_structureTypes);
}

/******************************************************************************
* Create an instance of the ParticleType class to represent a structure type.
******************************************************************************/
void StructureIdentificationModifier::createStructureType(int id, const QString& name, const Color& color)
{
	OORef<ParticleType> stype(new ParticleType(dataset()));
	stype->setId(id);
	stype->setName(name);
	stype->setColor(color);
	_structureTypes.push_back(stype);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void StructureIdentificationModifier::saveToStream(ObjectSaveStream& stream)
{
	AsynchronousParticleModifier::saveToStream(stream);
	stream.beginChunk(0x02);
	// For future use.
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void StructureIdentificationModifier::loadFromStream(ObjectLoadStream& stream)
{
	AsynchronousParticleModifier::loadFromStream(stream);
	stream.expectChunkRange(0, 2);
	// For future use.
	stream.closeChunk();
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void StructureIdentificationModifier::transferComputationResults(ComputeEngine* engine)
{
	_structureData = static_cast<StructureIdentificationEngine*>(engine)->structures();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the modification pipeline.
******************************************************************************/
PipelineStatus StructureIdentificationModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_structureData)
		throw Exception(tr("No computation results available."));

	if(inputParticleCount() != _structureData->size())
		throw Exception(tr("The number of input particles has changed. The stored analysis results have become invalid."));

	// Create output property object.
	ParticleTypeProperty* structureProperty = static_object_cast<ParticleTypeProperty>(outputStandardProperty(_structureData.data()));

	// Insert structure types into output property.
	structureProperty->setParticleTypes(structureTypes());

	// Build structure type to color map.
	std::vector<Color> structureTypeColors(structureTypes().size());
	std::vector<size_t> typeCounters(structureTypes().size());
	for(ParticleType* stype : structureTypes()) {
		OVITO_ASSERT(stype->id() >= 0);
		if(stype->id() >= (int)structureTypeColors.size()) {
			structureTypeColors.resize(stype->id() + 1);
			typeCounters.resize(stype->id() + 1);
		}
		structureTypeColors[stype->id()] = stype->color();
		typeCounters[stype->id()] = 0;
	}

	// Assign colors to particles based on their structure type.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);
	const int* s = structureProperty->constDataInt();
	for(Color& c : colorProperty->colorRange()) {
		OVITO_ASSERT(*s >= 0 && *s < structureTypeColors.size());
		c = structureTypeColors[*s];
		typeCounters[*s]++;
		++s;
	}
	colorProperty->changed();

	QList<int> structureCounts;
	for(ParticleType* stype : structureTypes()) {
		OVITO_ASSERT(stype->id() >= 0);
		while(structureCounts.size() <= stype->id())
			structureCounts.push_back(0);
		structureCounts[stype->id()] = typeCounters[stype->id()];
	}
	if(_structureCounts != structureCounts) {
		_structureCounts.swap(structureCounts);
		notifyDependents(ReferenceEvent::ObjectStatusChanged);
	}

	return PipelineStatus::Success;
}

/******************************************************************************
* Returns the default color for a structure type.
******************************************************************************/
Color StructureIdentificationModifier::getDefaultStructureColor(const QString& structureName)
{
	if(structureName == QStringLiteral("FCC")) return Color(0.4f, 1.0f, 0.4f);
	else if(structureName == QStringLiteral("HCP")) return Color(1.0f, 0.4f, 0.4f);
	else if(structureName == QStringLiteral("BCC")) return Color(0.4f, 0.4f, 1.0f);
	else if(structureName == QStringLiteral("ICO")) return Color(0.95f, 0.8f, 0.2f);
	else if(structureName == QStringLiteral("DIA")) return Color(0.2f, 0.95f, 0.8f);
	else return Color(0.95f, 0.95f, 0.95f);
}

/******************************************************************************
* Constructor.
******************************************************************************/
StructureListParameterUI::StructureListParameterUI(PropertiesEditor* parentEditor)
	: RefTargetListParameterUI(parentEditor, PROPERTY_FIELD(StructureIdentificationModifier::_structureTypes), RolloutInsertionParameters(), nullptr)
{
	connect(tableWidget(220), &QTableWidget::doubleClicked, this, &StructureListParameterUI::onDoubleClickStructureType);
	tableWidget()->setAutoScroll(false);
}

/******************************************************************************
* Returns a data item from the list data model.
******************************************************************************/
QVariant StructureListParameterUI::getItemData(RefTarget* target, const QModelIndex& index, int role)
{
	ParticleType* stype = dynamic_object_cast<ParticleType>(target);
	StructureIdentificationModifier* modifier = dynamic_object_cast<StructureIdentificationModifier>(editor()->editObject());

	if(stype && modifier) {
		if(role == Qt::DisplayRole) {
			if(index.column() == 1)
				return stype->name();
			else if(index.column() == 2) {
				if(stype->id() >= 0 && stype->id() < modifier->structureCounts().size())
					return modifier->structureCounts()[stype->id()];
				else
					return QString();
			}
			else if(index.column() == 3) {
				if(stype->id() >= 0 && stype->id() < modifier->structureCounts().size()) {
					size_t totalCount = 0;
					for(int c : modifier->structureCounts())
						totalCount += c;
					return QString("%1%").arg((double)modifier->structureCounts()[stype->id()] * 100.0 / std::max((size_t)1, totalCount), 0, 'f', 1);
				}
				else
					return QString();
			}
			else if(index.column() == 4)
				return stype->id();
		}
		else if(role == Qt::DecorationRole) {
			if(index.column() == 0)
				return (QColor)stype->color();
		}
	}
	return QVariant();
}

/******************************************************************************
* This method is called when a reference target changes.
******************************************************************************/
bool StructureListParameterUI::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(source == editObject()) {
		if(event->type() == ReferenceEvent::ObjectStatusChanged) {
			// Update the structure count and fraction columns.
			_model->updateColumns(2, 3);
		}
	}
	return RefTargetListParameterUI::referenceEvent(source, event);
}

/******************************************************************************
* Is called when the user has double-clicked on one of the structure
* types in the list widget.
******************************************************************************/
void StructureListParameterUI::onDoubleClickStructureType(const QModelIndex& index)
{
	// Let the user select a color for the structure type.
	ParticleType* stype = static_object_cast<ParticleType>(selectedObject());
	if(!stype) return;

	QColor oldColor = (QColor)stype->color();
	QColor newColor = QColorDialog::getColor(oldColor, editor()->container());
	if(!newColor.isValid() || newColor == oldColor) return;

	undoableTransaction(tr("Change structure type color"), [stype, newColor]() {
		stype->setColor(Color(newColor));
	});
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
