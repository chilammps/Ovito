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
#include <core/utilities/concurrent/ParallelFor.h>
#include <viz/util/TreeNeighborListBuilder.h>

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
	createStructureType(FCC, tr("FCC - Face-centered cubic"), Color(0.4f, 1.0f, 0.4f));
	createStructureType(HCP, tr("HCP - Hexagonal close-packed"), Color(1.0f, 0.4f, 0.4f));
	createStructureType(BCC, tr("BCC - Body-centered cubic"), Color(0.4f, 0.4f, 1.0f));
	createStructureType(ICO, tr("ICO - Icosahedral"), Color(0.95f, 0.8f, 0.2f));
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
void BondAngleAnalysisModifier::retrieveModifierResults(Engine* engine)
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
	size_t particleCount = _positions->size();
	futureInterface.setProgressText(tr("Performing bond angle analysis"));

	// Prepare the neighbor list.
	TreeNeighborListBuilder neighborListBuilder(14);
	if(!neighborListBuilder.prepare(_positions.data(), _simCell))
		return;

	// Create output storage.
	_structures = new ParticleProperty(particleCount, ParticleProperty::StructureTypeProperty);
	ParticleProperty* output = _structures.data();

	// Perform analysis on each particle.
	parallelFor(particleCount, futureInterface, [&neighborListBuilder, output](size_t index) {
		output->setInt(index, determineStructure(neighborListBuilder, index));
	});
}

/******************************************************************************
* This lets the modifier insert the previously computed results into the pipeline.
******************************************************************************/
ObjectStatus BondAngleAnalysisModifier::applyModifierResults(TimePoint time, TimeInterval& validityInterval)
{
	if(structureTypes().size() != NUM_STRUCTURE_TYPES)
		throw Exception(tr("The number of structure types has changed. Please remove this modifier from the modification pipeline and insert it again."));

	if(inputParticleCount() != particleStructures().size())
		throw Exception(tr("The number of input particles has changed. The stored analysis results have become invalid."));

	// Get output property object.
	ParticleTypeProperty* structureProperty = static_object_cast<ParticleTypeProperty>(outputStandardProperty(ParticleProperty::StructureTypeProperty));

	// Insert structure types into output property.
	structureProperty->setParticleTypes(structureTypes());

	// Insert results into output property.
	structureProperty->replaceStorage(_structureProperty.data());

	// Build list of type colors.
	Color structureTypeColors[NUM_STRUCTURE_TYPES];
	for(int index = 0; index < NUM_STRUCTURE_TYPES; index++) {
		structureTypeColors[index] = structureTypes()[index]->color();
	}

	size_t typeCounters[NUM_STRUCTURE_TYPES];
	for(int i = 0; i < NUM_STRUCTURE_TYPES; i++)
		typeCounters[i] = 0;

	// Assign colors to particles based on their structure type.
	ParticlePropertyObject* colorProperty = outputStandardProperty(ParticleProperty::ColorProperty);
	OVITO_ASSERT(colorProperty->size() == particleStructures().size());
	const int* s = particleStructures().constDataInt();
	Color* c = colorProperty->dataColor();
	Color* c_end = c + colorProperty->size();
	for(; c != c_end; ++s, ++c) {
		OVITO_ASSERT(*s >= 0 && *s < NUM_STRUCTURE_TYPES);
		*c = structureTypeColors[*s];
		typeCounters[*s]++;
	}
	colorProperty->changed();

	QString statusMessage;
	statusMessage += tr("%n FCC atoms\n", 0, typeCounters[FCC]);
	statusMessage += tr("%n HCP atoms\n", 0, typeCounters[HCP]);
	statusMessage += tr("%n BCC atoms\n", 0, typeCounters[BCC]);
	statusMessage += tr("%n ICO atoms\n", 0, typeCounters[ICO]);
	statusMessage += tr("%n other atoms", 0, typeCounters[OTHER]);
	return ObjectStatus(ObjectStatus::Success, QString(), statusMessage);
}

/******************************************************************************
* Determines the coordination structure of a single particle using the bond-angle analysis method.
******************************************************************************/
BondAngleAnalysisModifier::StructureType BondAngleAnalysisModifier::determineStructure(TreeNeighborListBuilder& neighList, size_t particleIndex)
{
	// Create neighbor list finder.
	TreeNeighborListBuilder::Locator<14> loc(neighList);

	// Find N nearest neighbor of current atom.
	loc.findNeighbors(neighList.particlePos(particleIndex));

	// Reject under-coordinated particles.
	if(loc.results().size() < 6)
		return OTHER;

	// Mean squared distance of 6 nearest neighbors.
	FloatType r0_sq = 0.0;
	for(int j = 0; j < 6; j++)
		r0_sq += loc.results()[j].distanceSq;
	r0_sq /= 6.0;

	// n0 near neighbors with: distsq<1.45*r0_sq
	// n1 near neighbors with: distsq<1.55*r0_sq
	FloatType n0_dist_sq = 1.45f * r0_sq;
	FloatType n1_dist_sq = 1.55f * r0_sq;
	int n0 = 0;
	for(auto n = loc.results().begin(); n != loc.results().end(); ++n, ++n0) {
		if(n->distanceSq > n0_dist_sq) break;
	}
	auto n0end = loc.results().begin() + n0;
	int n1 = n0;
	for(auto n = n0end; n != loc.results().end(); ++n, ++n1) {
		if(n->distanceSq >= n1_dist_sq) break;
	}

	// Evaluate all angles <(r_ij,rik) for all n0 particles with: distsq<1.45*r0_sq
	int chi[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for(auto j = loc.results().begin(); j != n0end; ++j) {
		FloatType norm_j = sqrt(j->distanceSq);
		for(auto k = j + 1; k != n0end; ++k) {
			FloatType norm_k = sqrt(k->distanceSq);
			FloatType bond_angle = j->delta.dot(k->delta) / (norm_j*norm_k);

			// Build histogram for identifying the relevant peaks.
			if(bond_angle < -0.945) { chi[0]++; }
			else if(-0.945 <= bond_angle && bond_angle < -0.915) { chi[1]++; }
			else if(-0.915 <= bond_angle && bond_angle < -0.755) { chi[2]++; }
			else if(-0.755 <= bond_angle && bond_angle < -0.195) { chi[3]++; }
			else if(-0.195 <= bond_angle && bond_angle < 0.195) { chi[4]++; }
			else if(0.195 <= bond_angle && bond_angle < 0.245) { chi[5]++; }
			else if(0.245 <= bond_angle && bond_angle < 0.795) { chi[6]++; }
			else if(0.795 <= bond_angle) { chi[7]++; }
		}
	}

	// Calculate deviations from the different lattice structures.
	FloatType delta_bcc = FloatType(0.35) * chi[4] / (FloatType)(chi[5] + chi[6] - chi[4]);
	FloatType delta_cp = std::abs(FloatType(1) - (FloatType)chi[6] / 24);
	FloatType delta_fcc = FloatType(0.61) * (FloatType)(std::abs(chi[0] + chi[1] - 6) + chi[2]) / 6;
	FloatType delta_hcp = (FloatType)(std::abs(chi[0] - 3) + std::abs(chi[0] + chi[1] + chi[2] + chi[3] - 9)) / 12;

	// Identification of the local structure according to the reference.
	if(chi[0] == 7)       { delta_bcc = 0.; }
	else if(chi[0] == 6)  { delta_fcc = 0.; }
	else if(chi[0] <= 3)  { delta_hcp = 0.; }

	if(chi[7] > 0) return OTHER;
	else if(chi[4] < 3) {
		if(n1 > 13 || n1 < 11) return OTHER;
		else return ICO;
	}
	else if(delta_bcc <= delta_cp) {
		if(n1 < 11) return OTHER;
		else return BCC;
	}
	else if(n1 > 12 || n1 < 11) return OTHER;
	else if(delta_fcc < delta_hcp) return FCC;
	else return HCP;
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
