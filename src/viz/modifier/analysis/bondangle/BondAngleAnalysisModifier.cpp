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
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/animation/AnimManager.h>
#include <core/gui/properties/BooleanParameterUI.h>

#include "BondAngleAnalysisModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, BondAngleAnalysisModifier, AsynchronousParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, BondAngleAnalysisModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(BondAngleAnalysisModifier, BondAngleAnalysisModifierEditor)
DEFINE_VECTOR_REFERENCE_FIELD(BondAngleAnalysisModifier, _structureTypes, "StructureTypes", ParticleType)
SET_PROPERTY_FIELD_LABEL(BondAngleAnalysisModifier, _structureTypes, "Structure types")

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
BondAngleAnalysisModifier::BondAngleAnalysisModifier() :
	_structureProperty(new ParticleProperty(0, ParticleProperty::StructureTypeProperty))
{
	INIT_PROPERTY_FIELD(BondAngleAnalysisModifier::_structureTypes);

	// Create the structure types.
	createStructureType(OTHER, tr("Other"), Color(0.95f, 0.95f, 0.95f));
	createStructureType(FCC, tr("FCC"), Color(0.4f, 1.0f, 0.4f));
	createStructureType(HCP, tr("HCP"), Color(1.0f, 0.4f, 0.4f));
	createStructureType(BCC, tr("BCC"), Color(0.4f, 0.4f, 1.0f));
	createStructureType(ICO, tr("Icosahedral"), Color(0.2f, 1.0f, 1.0f));
}

/******************************************************************************
* Create an instance of the ParticleType class to represent a structure type.
******************************************************************************/
void BondAngleAnalysisModifier::createStructureType(StructureType id, const QString& name, const Color& color)
{
	OORef<ParticleType> stype(new ParticleType());
	stype->setId(id);
	stype->setName(name);
	stype->setColor(color);
	_structureTypes.push_back(stype);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void BondAngleAnalysisModifier::saveToStream(ObjectSaveStream& stream)
{
	AsynchronousParticleModifier::saveToStream(stream);
	stream.beginChunk(0x01);
	_structureProperty.constData()->saveToStream(stream, !storeResultsWithScene());
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void BondAngleAnalysisModifier::loadFromStream(ObjectLoadStream& stream)
{
	AsynchronousParticleModifier::loadFromStream(stream);
	stream.expectChunk(0x01);
	_structureProperty.data()->loadFromStream(stream);
	stream.closeChunk();
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> BondAngleAnalysisModifier::createEngine(TimePoint time)
{
	if(structureTypes().size() != NUM_STRUCTURE_TYPES)
		throw Exception(tr("The number of structure types has changed. Please remove this modifier from the modification pipeline and insert it again."));

	// Get input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCell* simCell = expectSimulationCell();

	return std::make_shared<BondAngleAnalysisEngine>(posProperty->storage(), simCell->data());
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void BondAngleAnalysisModifier::retrieveResults(Engine* engine)
{
	BondAngleAnalysisEngine* eng = static_cast<BondAngleAnalysisEngine*>(engine);
	if(eng->structures())
		_structureProperty = eng->structures();
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void BondAngleAnalysisModifier::BondAngleAnalysisEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Performing bond angle analysis"));
	futureInterface.setProgressRange(100);

	_structures = new ParticleProperty(_positions->size(), ParticleProperty::StructureTypeProperty);
	for(int i = 0; i < 100 && !futureInterface.isCanceled(); i++) {
		QThread::msleep(30);
		futureInterface.setProgressValue(i);
	}
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void BondAngleAnalysisModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Bond angle analysis"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
#ifndef Q_WS_MAC
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(0);
#endif

	BooleanParameterUI* autoUpdateUI = new BooleanParameterUI(this, PROPERTY_FIELD(AsynchronousParticleModifier::_autoUpdate));
	layout1->addWidget(autoUpdateUI->checkBox());

	BooleanParameterUI* saveResultsUI = new BooleanParameterUI(this, PROPERTY_FIELD(AsynchronousParticleModifier::_saveResults));
	layout1->addWidget(saveResultsUI->checkBox());

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	// Derive a custom class from the list parameter UI to
	// give the items a color.
	class CustomRefTargetListParameterUI : public RefTargetListParameterUI {
	public:
		CustomRefTargetListParameterUI(PropertiesEditor* parentEditor, const PropertyFieldDescriptor& refField)
			: RefTargetListParameterUI(parentEditor, refField, RolloutInsertionParameters(), NULL) {}
	protected:
		virtual QVariant getItemData(RefTarget* target, const QModelIndex& index, int role) override {
			if(role == Qt::DecorationRole && target) {
				return (QColor)static_object_cast<ParticleType>(target)->color();
			}
			else return RefTargetListParameterUI::getItemData(target, index, role);
		}
		/// Do not open sub-editor for selected atom type.
		virtual void openSubEditor() override {}
	};

	_structureTypesPUI = new CustomRefTargetListParameterUI(this, PROPERTY_FIELD(BondAngleAnalysisModifier::_structureTypes));
	layout1->addSpacing(10);
	layout1->addWidget(new QLabel(tr("Structure types:")));
	layout1->addWidget(_structureTypesPUI->listWidget(150));
	connect(_structureTypesPUI->listWidget(), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onDoubleClickStructureType(const QModelIndex&)));
}

/******************************************************************************
* Is called when the user has double-clicked on one of the structure
* types in the list widget.
******************************************************************************/
void BondAngleAnalysisModifierEditor::onDoubleClickStructureType(const QModelIndex& index)
{
	// Let the user select a color for the structure type.
	ParticleType* stype = static_object_cast<ParticleType>(_structureTypesPUI->selectedObject());
	if(!stype) return;

	QColor oldColor = (QColor)stype->color();
	QColor newColor = QColorDialog::getColor(oldColor, container());
	if(!newColor.isValid() || newColor == oldColor) return;

	UndoManager::instance().beginCompoundOperation(tr("Change color"));
	stype->setColor(Color(newColor));
	UndoManager::instance().endCompoundOperation();
}

};	// End of namespace
