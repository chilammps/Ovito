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
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <viz/util/OnTheFlyNeighborListBuilder.h>
#include <viz/data/ParticleTypeProperty.h>

#include "CreateBondsModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, CreateBondsModifier, AsynchronousParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, CreateBondsModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(CreateBondsModifier, CreateBondsModifierEditor)
DEFINE_PROPERTY_FIELD(CreateBondsModifier, _cutoffMode, "CutoffMode")
DEFINE_FLAGS_PROPERTY_FIELD(CreateBondsModifier, _uniformCutoff, "UniformCutoff", PROPERTY_FIELD_MEMORIZE)
DEFINE_FLAGS_REFERENCE_FIELD(CreateBondsModifier, _bondsDisplay, "BondsDisplay", BondsDisplay, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
DEFINE_FLAGS_REFERENCE_FIELD(CreateBondsModifier, _bondsObj, "BondsObject", BondsObject, PROPERTY_FIELD_ALWAYS_DEEP_COPY)
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _cutoffMode, "Cutoff mode")
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _uniformCutoff, "Cutoff radius")
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _bondsDisplay, "Bonds display")
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _bondsObj, "Bonds")
SET_PROPERTY_FIELD_UNITS(CreateBondsModifier, _uniformCutoff, WorldParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CreateBondsModifier::CreateBondsModifier() : _cutoffMode(UniformCutoff), _uniformCutoff(3.2)
{
	INIT_PROPERTY_FIELD(CreateBondsModifier::_cutoffMode);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_uniformCutoff);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_bondsDisplay);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_bondsObj);

	// Create the output object.
	_bondsObj = new BondsObject();
	_bondsObj->setSaveWithScene(storeResultsWithScene());

	// Create the display object for bonds rendering and assign it to the scene object.
	_bondsDisplay = new BondsDisplay();
	_bondsObj->setDisplayObject(_bondsDisplay);
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CreateBondsModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	// Recompute results when the parameters have been changed.
	if(autoUpdateEnabled()) {
		if(field == PROPERTY_FIELD(CreateBondsModifier::_uniformCutoff) || field == PROPERTY_FIELD(CreateBondsModifier::_cutoffMode))
			invalidateCachedResults();
	}

	// Adopt "Save with scene" flag.
	if(field == PROPERTY_FIELD(AsynchronousParticleModifier::_saveResults)) {
		if(bondsObject())
			bondsObject()->setSaveWithScene(storeResultsWithScene());
	}

	AsynchronousParticleModifier::propertyChanged(field);
}

/******************************************************************************
* Sets the cutoff radii for pairs of particle types.
******************************************************************************/
void CreateBondsModifier::setPairCutoffs(const PairCutoffsList& pairCutoffs)
{
	// Make the property change undoable.
	UndoManager::instance().undoablePropertyChange<PairCutoffsList>(this,
			&CreateBondsModifier::pairCutoffs, &CreateBondsModifier::setPairCutoffs);

	_pairCutoffs = pairCutoffs;

	if(autoUpdateEnabled())
		invalidateCachedResults();

	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* Saves the class' contents to the given stream.
******************************************************************************/
void CreateBondsModifier::saveToStream(ObjectSaveStream& stream)
{
	AsynchronousParticleModifier::saveToStream(stream);

	stream.beginChunk(0x01);
	stream << _pairCutoffs;
	stream.endChunk();
}

/******************************************************************************
* Loads the class' contents from the given stream.
******************************************************************************/
void CreateBondsModifier::loadFromStream(ObjectLoadStream& stream)
{
	AsynchronousParticleModifier::loadFromStream(stream);

	stream.expectChunk(0x01);
	stream >> _pairCutoffs;
	stream.closeChunk();
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> CreateBondsModifier::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<CreateBondsModifier> clone = static_object_cast<CreateBondsModifier>(AsynchronousParticleModifier::clone(deepCopy, cloneHelper));
	clone->_pairCutoffs = this->_pairCutoffs;
	return clone;
}

/******************************************************************************
* Handles reference events sent by reference targets of this object.
******************************************************************************/
bool CreateBondsModifier::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	// Do not propagate messages from the attached output and display objects.
	if(source == _bondsDisplay || source == _bondsObj)
		return false;

	return AsynchronousParticleModifier::referenceEvent(source, event);
}

/******************************************************************************
* Resets the modifier's result cache.
******************************************************************************/
void CreateBondsModifier::invalidateCachedResults()
{
	AsynchronousParticleModifier::invalidateCachedResults();

	// Reset all bonds when the input has changed.
	if(bondsObject())
		bondsObject()->clear();
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::Engine> CreateBondsModifier::createEngine(TimePoint time)
{
	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCell* simCell = expectSimulationCell();

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<BondGenerationEngine>(posProperty->storage(), simCell->data(), uniformCutoff());
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CreateBondsModifier::BondGenerationEngine::compute(FutureInterfaceBase& futureInterface)
{
	futureInterface.setProgressText(tr("Generating bonds"));

	// Prepare the neighbor list.
	OnTheFlyNeighborListBuilder neighborListBuilder(_cutoff);
	if(!neighborListBuilder.prepare(_positions.data(), _simCell, &_hasWrappedParticles) || futureInterface.isCanceled())
		return;

	// Generate (half) bonds.
	size_t particleCount = _positions->size();
	futureInterface.setProgressRange(particleCount);
	for(size_t particleIndex = 0; particleIndex < particleCount; particleIndex++) {

		for(OnTheFlyNeighborListBuilder::iterator neighborIter(neighborListBuilder, particleIndex); !neighborIter.atEnd(); neighborIter.next()) {
			_bonds->addBond(particleIndex, neighborIter.current(), neighborIter.pbcShift());
		}

		// Update progress indicator.
		if((particleIndex % 1024) == 0) {
			futureInterface.setProgressValue(particleIndex);
			if(futureInterface.isCanceled())
				return;
		}
	}
	futureInterface.setProgressValue(particleCount);
}

/******************************************************************************
* Unpacks the computation results stored in the given engine object.
******************************************************************************/
void CreateBondsModifier::retrieveModifierResults(Engine* engine)
{
	BondGenerationEngine* eng = static_cast<BondGenerationEngine*>(engine);
	if(eng->bonds() && bondsObject()) {
		bondsObject()->setStorage(eng->bonds());
		_hasWrappedParticles = eng->hasWrappedParticles();
	}
}

/******************************************************************************
* This lets the modifier insert the previously computed results into the pipeline.
******************************************************************************/
ObjectStatus CreateBondsModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	// Insert output object into pipeline.
	size_t bondsCount = 0;
	if(bondsObject()) {
		output().addObject(bondsObject());
		bondsCount = bondsObject()->bonds().size();
	}

	if(!_hasWrappedParticles)
		return ObjectStatus(ObjectStatus::Success, QString(), tr("Created %1 bonds").arg(bondsCount));
	else
		return ObjectStatus(ObjectStatus::Warning, QString(), tr("Created %1 bonds. Some of the particles are located outside the simulation cell boundaries. The bonds of these particles may not display correctly. Please use the 'Wrap at periodic boundaries' modifier to avoid this problem.").arg(bondsCount));
}

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CreateBondsModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Create bonds"), rolloutParams);

    // Create the rollout contents.
	QVBoxLayout* layout1 = new QVBoxLayout(rollout);
	layout1->setContentsMargins(4,4,4,4);
	layout1->setSpacing(6);

	QGridLayout* gridlayout = new QGridLayout();
	gridlayout->setContentsMargins(0,0,0,0);
	gridlayout->setColumnStretch(1, 1);

	IntegerRadioButtonParameterUI* cutoffModePUI = new IntegerRadioButtonParameterUI(this, PROPERTY_FIELD(CreateBondsModifier::_cutoffMode));
	QRadioButton* uniformCutoffModeBtn = cutoffModePUI->addRadioButton(CreateBondsModifier::UniformCutoff, tr("Uniform cutoff radius"));

	// Cutoff parameter.
	FloatParameterUI* cutoffRadiusPUI = new FloatParameterUI(this, PROPERTY_FIELD(CreateBondsModifier::_uniformCutoff));
	gridlayout->addWidget(uniformCutoffModeBtn, 0, 0);
	gridlayout->addLayout(cutoffRadiusPUI->createFieldLayout(), 0, 1);
	cutoffRadiusPUI->setMinValue(0);
	cutoffRadiusPUI->setEnabled(false);
	connect(uniformCutoffModeBtn, SIGNAL(toggled(bool)), cutoffRadiusPUI, SLOT(setEnabled(bool)));

	layout1->addLayout(gridlayout);

	QRadioButton* pairCutoffModeBtn = cutoffModePUI->addRadioButton(CreateBondsModifier::PairCutoff, tr("Pair-wise cutoff radii:"));
	layout1->addWidget(pairCutoffModeBtn);

	_pairCutoffTable = new QTableWidget();
	_pairCutoffTable->setColumnCount(3);
	_pairCutoffTable->setHorizontalHeaderLabels({ tr("1st Type"), tr("2nd Type"), tr("Cutoff") });
	_pairCutoffTable->verticalHeader()->setVisible(false);
	_pairCutoffTable->setEnabled(false);
	connect(pairCutoffModeBtn, SIGNAL(toggled(bool)), _pairCutoffTable, SLOT(setEnabled(bool)));
	layout1->addWidget(_pairCutoffTable);
	connect(_pairCutoffTable, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onPairCutoffTableChanged(QTableWidgetItem*)));

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	// Open a sub-editor for the bonds display object.
	new SubObjectParameterUI(this, PROPERTY_FIELD(CreateBondsModifier::_bondsDisplay), rolloutParams.after(rollout));

	// Update pair-wise cutoff table whenever a modifier has been loaded into the editor.
	connect(this, SIGNAL(contentsReplaced(RefTarget*)), this, SLOT(updatePairCutoffList()));
	connect(this, SIGNAL(contentsChanged(RefTarget*)), this, SLOT(updatePairCutoffList()));
}

/******************************************************************************
* Updates the contents of the pair-wise cutoff table.
******************************************************************************/
void CreateBondsModifierEditor::updatePairCutoffList()
{
	_pairCutoffTable->clearContents();

	CreateBondsModifier* mod = static_object_cast<CreateBondsModifier>(editObject());
	if(!mod) return;

	// Obtain the list of particle types in the modifier's input.
	PipelineFlowState inputState = mod->getModifierInput();
	for(const auto& o : inputState.objects()) {
		ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(o.get());
		if(typeProperty && typeProperty->type() == ParticleProperty::ParticleTypeProperty) {
			_pairCutoffTable->setRowCount(typeProperty->particleTypes().size() * (typeProperty->particleTypes().size() + 1) / 2);
			int row = 0;
			for(auto ptype1 = typeProperty->particleTypes().constBegin(); ptype1 != typeProperty->particleTypes().constEnd(); ++ptype1) {
				for(auto ptype2 = ptype1; ptype2 != typeProperty->particleTypes().constEnd(); ++ptype2) {
					QTableWidgetItem* typeItem1 = new QTableWidgetItem((*ptype1)->name());
					QTableWidgetItem* typeItem2 = new QTableWidgetItem((*ptype2)->name());
					QTableWidgetItem* cutoffItem = new QTableWidgetItem();
					typeItem1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
					typeItem2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
					cutoffItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsEditable);
					_pairCutoffTable->setItem(row, 0, typeItem1);
					_pairCutoffTable->setItem(row, 1, typeItem2);
					_pairCutoffTable->setItem(row, 2, cutoffItem);
					row++;
				}
			}
			break;
		}
	}

	updatePairCutoffListValues();
}

/******************************************************************************
* Updates the cutoff values in the pair-wise cutoff table.
******************************************************************************/
void CreateBondsModifierEditor::updatePairCutoffListValues()
{
	CreateBondsModifier* mod = static_object_cast<CreateBondsModifier>(editObject());
	if(!mod) return;

	for(int row = 0; row < _pairCutoffTable->rowCount(); row++) {
		QString typeName1 = _pairCutoffTable->item(row, 0)->text();
		QString typeName2 = _pairCutoffTable->item(row, 1)->text();
		FloatType cutoffRadius = mod->pairCutoffs()[qMakePair(typeName1, typeName2)];
		if(cutoffRadius > 0.0f)
			_pairCutoffTable->item(row, 2)->setText(QString::number(cutoffRadius));
		else
			_pairCutoffTable->item(row, 2)->setText(QString());
	}
}

/******************************************************************************
* Is called when the user has changed a cutoff value in the pair cutoff table.
******************************************************************************/
void CreateBondsModifierEditor::onPairCutoffTableChanged(QTableWidgetItem* item)
{
	CreateBondsModifier* mod = static_object_cast<CreateBondsModifier>(editObject());
	if(!mod) return;

	int row = item->row();
	QString typeName1 = _pairCutoffTable->item(row, 0)->text();
	QString typeName2 = _pairCutoffTable->item(row, 1)->text();
	bool ok;
	FloatType cutoff = (FloatType)item->text().toDouble(&ok);
	if(!ok) cutoff = 0.0f;

	CreateBondsModifier::PairCutoffsList pairCutoffs = mod->pairCutoffs();

	updatePairCutoffListValues();
}

};	// End of namespace
