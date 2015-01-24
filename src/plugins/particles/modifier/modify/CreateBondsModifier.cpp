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
#include <core/gui/properties/SubObjectParameterUI.h>
#include <core/gui/properties/IntegerRadioButtonParameterUI.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <plugins/particles/util/CutoffNeighborFinder.h>
#include <plugins/particles/objects/ParticleTypeProperty.h>

#include "CreateBondsModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, CreateBondsModifier, AsynchronousParticleModifier);
SET_OVITO_OBJECT_EDITOR(CreateBondsModifier, CreateBondsModifierEditor);
DEFINE_PROPERTY_FIELD(CreateBondsModifier, _cutoffMode, "CutoffMode");
DEFINE_FLAGS_PROPERTY_FIELD(CreateBondsModifier, _uniformCutoff, "UniformCutoff", PROPERTY_FIELD_MEMORIZE);
DEFINE_FLAGS_REFERENCE_FIELD(CreateBondsModifier, _bondsDisplay, "BondsDisplay", BondsDisplay, PROPERTY_FIELD_ALWAYS_DEEP_COPY|PROPERTY_FIELD_MEMORIZE);
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _cutoffMode, "Cutoff mode");
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _uniformCutoff, "Cutoff radius");
SET_PROPERTY_FIELD_LABEL(CreateBondsModifier, _bondsDisplay, "Bonds display");
SET_PROPERTY_FIELD_UNITS(CreateBondsModifier, _uniformCutoff, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, CreateBondsModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
CreateBondsModifier::CreateBondsModifier(DataSet* dataset) : AsynchronousParticleModifier(dataset),
	_cutoffMode(UniformCutoff), _uniformCutoff(3.2)
{
	INIT_PROPERTY_FIELD(CreateBondsModifier::_cutoffMode);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_uniformCutoff);
	INIT_PROPERTY_FIELD(CreateBondsModifier::_bondsDisplay);

	// Create the display object for bonds rendering and assign it to the data object.
	_bondsDisplay = new BondsDisplay(dataset);
}

/******************************************************************************
* Is called when the value of a property of this object has changed.
******************************************************************************/
void CreateBondsModifier::propertyChanged(const PropertyFieldDescriptor& field)
{
	AsynchronousParticleModifier::propertyChanged(field);

	// Recompute results when the parameters have been changed.
	if(field == PROPERTY_FIELD(CreateBondsModifier::_uniformCutoff) || field == PROPERTY_FIELD(CreateBondsModifier::_cutoffMode))
		invalidateCachedResults();
}

/******************************************************************************
* Sets the cutoff radii for pairs of particle types.
******************************************************************************/
void CreateBondsModifier::setPairCutoffs(const PairCutoffsList& pairCutoffs)
{
	// Make the property change undoable.
	dataset()->undoStack().undoablePropertyChange<PairCutoffsList>(this,
			&CreateBondsModifier::pairCutoffs, &CreateBondsModifier::setPairCutoffs);

	_pairCutoffs = pairCutoffs;

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
	// Do not propagate messages from the attached display object.
	if(source == _bondsDisplay)
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
	_bonds.reset();
}

/******************************************************************************
* Creates and initializes a computation engine that will compute the modifier's results.
******************************************************************************/
std::shared_ptr<AsynchronousParticleModifier::ComputeEngine> CreateBondsModifier::createEngine(TimePoint time, TimeInterval validityInterval)
{
	// Get modifier input.
	ParticlePropertyObject* posProperty = expectStandardProperty(ParticleProperty::PositionProperty);
	SimulationCellObject* simCell = expectSimulationCell();

	// Build table of pair-wise cutoff radii.
	ParticleTypeProperty* typeProperty = nullptr;
	std::vector<std::vector<FloatType>> pairCutoffTable;
	if(cutoffMode() == PairCutoff) {
		typeProperty = dynamic_object_cast<ParticleTypeProperty>(expectStandardProperty(ParticleProperty::ParticleTypeProperty));
		if(typeProperty) {
			for(PairCutoffsList::const_iterator entry = pairCutoffs().begin(); entry != pairCutoffs().end(); ++entry) {
				FloatType cutoff = entry.value();
				if(cutoff > 0.0f) {
					ParticleType* ptype1 = typeProperty->particleType(entry.key().first);
					ParticleType* ptype2 = typeProperty->particleType(entry.key().second);
					if(ptype1 && ptype2 && ptype1->id() >= 0 && ptype2->id() >= 0) {
						if((int)pairCutoffTable.size() <= std::max(ptype1->id(), ptype2->id())) pairCutoffTable.resize(std::max(ptype1->id(), ptype2->id()) + 1);
						if((int)pairCutoffTable[ptype1->id()].size() <= ptype2->id()) pairCutoffTable[ptype1->id()].resize(ptype2->id() + 1, FloatType(0));
						if((int)pairCutoffTable[ptype2->id()].size() <= ptype1->id()) pairCutoffTable[ptype2->id()].resize(ptype1->id() + 1, FloatType(0));
						pairCutoffTable[ptype1->id()][ptype2->id()] = cutoff * cutoff;
						pairCutoffTable[ptype2->id()][ptype1->id()] = cutoff * cutoff;
					}
				}
			}
		}
	}

	// Create engine object. Pass all relevant modifier parameters to the engine as well as the input data.
	return std::make_shared<BondsEngine>(validityInterval, posProperty->storage(),
			typeProperty ? typeProperty->storage() : nullptr, simCell->data(), cutoffMode(),
			uniformCutoff(), std::move(pairCutoffTable));
}

/******************************************************************************
* Performs the actual analysis. This method is executed in a worker thread.
******************************************************************************/
void CreateBondsModifier::BondsEngine::perform()
{
	setProgressText(tr("Generating bonds"));

	// Determine maximum cutoff.
	FloatType maxCutoff = _uniformCutoff;
	if(_particleTypes) {
		OVITO_ASSERT(_particleTypes->size() == _positions->size());
		for(const auto& innerList : _pairCutoffs)
			for(const auto& cutoff : innerList)
				if(cutoff > maxCutoff) maxCutoff = cutoff;
	}

	// Prepare the neighbor list.
	CutoffNeighborFinder neighborFinder;
	if(!neighborFinder.prepare(maxCutoff, _positions.data(), _simCell, this))
		return;

	// Generate (half) bonds.
	size_t particleCount = _positions->size();
	setProgressRange(particleCount);
	if(!_particleTypes) {
		for(size_t particleIndex = 0; particleIndex < particleCount; particleIndex++) {
			for(CutoffNeighborFinder::Query neighborQuery(neighborFinder, particleIndex); !neighborQuery.atEnd(); neighborQuery.next()) {
				_bonds->addBond(particleIndex, neighborQuery.current(), neighborQuery.unwrappedPbcShift());
			}
			// Update progress indicator.
			if((particleIndex % 4096) == 0) {
				setProgressValue(particleIndex);
				if(isCanceled())
					return;
			}
		}
	}
	else {
		for(size_t particleIndex = 0; particleIndex < particleCount; particleIndex++) {
			for(CutoffNeighborFinder::Query neighborQuery(neighborFinder, particleIndex); !neighborQuery.atEnd(); neighborQuery.next()) {
				int type1 = _particleTypes->getInt(particleIndex);
				int type2 = _particleTypes->getInt(neighborQuery.current());
				if(type1 >= 0 && type1 < (int)_pairCutoffs.size() && type2 >= 0 && type2 < (int)_pairCutoffs[type1].size()) {
					if(neighborQuery.distanceSquared() <= _pairCutoffs[type1][type2])
						_bonds->addBond(particleIndex, neighborQuery.current(), neighborQuery.unwrappedPbcShift());
				}
			}
			// Update progress indicator.
			if((particleIndex % 4096) == 0) {
				setProgressValue(particleIndex);
				if(isCanceled())
					return;
			}
		}
	}
	setProgressValue(particleCount);
}

/******************************************************************************
* Unpacks the results of the computation engine and stores them in the modifier.
******************************************************************************/
void CreateBondsModifier::transferComputationResults(ComputeEngine* engine)
{
	_bonds = static_cast<BondsEngine*>(engine)->bonds();
}

/******************************************************************************
* Lets the modifier insert the cached computation results into the
* modification pipeline.
******************************************************************************/
PipelineStatus CreateBondsModifier::applyComputationResults(TimePoint time, TimeInterval& validityInterval)
{
	if(!_bonds)
		throw Exception(tr("No computation results available."));

	size_t bondsCount = _bonds->bonds().size();

	// Create the output data object.
	OORef<BondsObject> bondsObj(new BondsObject(dataset()));
	bondsObj->setStorage(_bonds.data());
	bondsObj->addDisplayObject(_bondsDisplay);

	// Insert output object into the pipeline.
	output().addObject(bondsObj);

	// If the number of bonds is unusually high, we better turn off bonds display to prevent the program from freezing.
	if(bondsCount > 1000000) {
		bondsDisplay()->setEnabled(false);
		return PipelineStatus(PipelineStatus::Warning, tr("Created %1 bonds. Automatically disabled display of such a large number of bonds to prevent the program from freezing.").arg(bondsCount));
	}

	return PipelineStatus(PipelineStatus::Success, tr("Created %1 bonds.").arg(bondsCount));
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void CreateBondsModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Create bonds"), rolloutParams, "particles.modifiers.create_bonds.html");

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
	connect(uniformCutoffModeBtn, &QRadioButton::toggled, cutoffRadiusPUI, &FloatParameterUI::setEnabled);

	layout1->addLayout(gridlayout);

	QRadioButton* pairCutoffModeBtn = cutoffModePUI->addRadioButton(CreateBondsModifier::PairCutoff, tr("Pair-wise cutoff radii:"));
	layout1->addWidget(pairCutoffModeBtn);

	_pairCutoffTable = new QTableView();
	_pairCutoffTable->verticalHeader()->setVisible(false);
	_pairCutoffTable->setEnabled(false);
	_pairCutoffTableModel = new PairCutoffTableModel(_pairCutoffTable);
	_pairCutoffTable->setModel(_pairCutoffTableModel);
	connect(pairCutoffModeBtn,&QRadioButton::toggled, _pairCutoffTable, &QTableView::setEnabled);
	layout1->addWidget(_pairCutoffTable);

	// Status label.
	layout1->addSpacing(10);
	layout1->addWidget(statusLabel());

	// Open a sub-editor for the bonds display object.
	new SubObjectParameterUI(this, PROPERTY_FIELD(CreateBondsModifier::_bondsDisplay), rolloutParams.after(rollout));

	// Update pair-wise cutoff table whenever a modifier has been loaded into the editor.
	connect(this, &CreateBondsModifierEditor::contentsReplaced, this, &CreateBondsModifierEditor::updatePairCutoffList);
	connect(this, &CreateBondsModifierEditor::contentsChanged, this, &CreateBondsModifierEditor::updatePairCutoffListValues);
}

/******************************************************************************
* Updates the contents of the pair-wise cutoff table.
******************************************************************************/
void CreateBondsModifierEditor::updatePairCutoffList()
{
	CreateBondsModifier* mod = static_object_cast<CreateBondsModifier>(editObject());
	if(!mod) return;

	// Obtain the list of particle types in the modifier's input.
	PairCutoffTableModel::ContentType pairCutoffs;
	PipelineFlowState inputState = mod->getModifierInput();
	ParticleTypeProperty* typeProperty = dynamic_object_cast<ParticleTypeProperty>(
			ParticlePropertyObject::findInState(inputState, ParticleProperty::ParticleTypeProperty));
	if(typeProperty) {
		for(auto ptype1 = typeProperty->particleTypes().constBegin(); ptype1 != typeProperty->particleTypes().constEnd(); ++ptype1) {
			for(auto ptype2 = ptype1; ptype2 != typeProperty->particleTypes().constEnd(); ++ptype2) {
				pairCutoffs.push_back(qMakePair((*ptype1)->name(), (*ptype2)->name()));
			}
		}
	}
	_pairCutoffTableModel->setContent(mod, pairCutoffs);
}

/******************************************************************************
* Updates the cutoff values in the pair-wise cutoff table.
******************************************************************************/
void CreateBondsModifierEditor::updatePairCutoffListValues()
{
	_pairCutoffTableModel->updateContent();
}

/******************************************************************************
* Returns data from the pair-cutoff table model.
******************************************************************************/
QVariant CreateBondsModifierEditor::PairCutoffTableModel::data(const QModelIndex& index, int role) const
{
	if(role == Qt::DisplayRole) {
		if(index.column() == 0) return _data[index.row()].first;
		else if(index.column() == 1) return _data[index.row()].second;
		else if(index.column() == 2) {
			FloatType cutoffRadius = _modifier->pairCutoffs()[_data[index.row()]];
			if(cutoffRadius > 0.0f)
				return cutoffRadius;
		}
	}
	return QVariant();
}

/******************************************************************************
* Sets data in the pair-cutoff table model.
******************************************************************************/
bool CreateBondsModifierEditor::PairCutoffTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role == Qt::EditRole && index.column() == 2) {
		bool ok;
		FloatType cutoff = (FloatType)value.toDouble(&ok);
		if(!ok) cutoff = 0;

		CreateBondsModifier::PairCutoffsList pairCutoffs = _modifier->pairCutoffs();
		pairCutoffs[_data[index.row()]] = cutoff;

		UndoableTransaction::handleExceptions(_modifier->dataset()->undoStack(), tr("Change cutoff"), [&pairCutoffs, this]() {
			_modifier->setPairCutoffs(pairCutoffs);
		});
		return true;
	}
	return false;
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
