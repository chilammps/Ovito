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

#include <plugins/particles/Particles.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include <core/gui/properties/IntegerParameterUI.h>
#include <plugins/particles/objects/BondsObject.h>
#include "ShowPeriodicImagesModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ShowPeriodicImagesModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(ShowPeriodicImagesModifier, ShowPeriodicImagesModifierEditor);
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _showImageX, "ShowImageX");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _showImageY, "ShowImageY");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _showImageZ, "ShowImageZ");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _numImagesX, "NumImagesX");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _numImagesY, "NumImagesY");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _numImagesZ, "NumImagesZ");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _adjustBoxSize, "AdjustBoxSize");
DEFINE_PROPERTY_FIELD(ShowPeriodicImagesModifier, _uniqueIdentifiers, "UniqueIdentifiers");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _showImageX, "Periodic images X");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _showImageY, "Periodic images Y");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _showImageZ, "Periodic images Z");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _numImagesX, "Number of periodic images - X");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _numImagesY, "Number of periodic images - Y");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _numImagesZ, "Number of periodic images - Z");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _adjustBoxSize, "Adjust simulation box size");
SET_PROPERTY_FIELD_LABEL(ShowPeriodicImagesModifier, _uniqueIdentifiers, "Assign unique particle IDs");

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_OVITO_OBJECT(Particles, ShowPeriodicImagesModifierEditor, ParticleModifierEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
ShowPeriodicImagesModifier::ShowPeriodicImagesModifier(DataSet* dataset) : ParticleModifier(dataset),
	_showImageX(false), _showImageY(false), _showImageZ(false),
	_numImagesX(3), _numImagesY(3), _numImagesZ(3), _adjustBoxSize(false), _uniqueIdentifiers(true)
{
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_showImageX);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_showImageY);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_showImageZ);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_numImagesX);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_numImagesY);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_numImagesZ);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_adjustBoxSize);
	INIT_PROPERTY_FIELD(ShowPeriodicImagesModifier::_uniqueIdentifiers);
}

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
PipelineStatus ShowPeriodicImagesModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	std::array<int,3> nPBC;
	nPBC[0] = showImageX() ? std::max(numImagesX(),1) : 1;
	nPBC[1] = showImageY() ? std::max(numImagesY(),1) : 1;
	nPBC[2] = showImageZ() ? std::max(numImagesZ(),1) : 1;

	// Calculate new number of atoms.
	size_t numCopies = nPBC[0] * nPBC[1] * nPBC[2];
	if(numCopies <= 1 || inputParticleCount() == 0)
		return PipelineStatus::Success;

	Box3I newImages;
	newImages.minc[0] = -(nPBC[0]-1)/2;
	newImages.minc[1] = -(nPBC[1]-1)/2;
	newImages.minc[2] = -(nPBC[2]-1)/2;
	newImages.maxc[0] = nPBC[0]/2;
	newImages.maxc[1] = nPBC[1]/2;
	newImages.maxc[2] = nPBC[2]/2;

	// Enlarge particle property arrays.
	size_t oldParticleCount = inputParticleCount();
	size_t newParticleCount = oldParticleCount * numCopies;

	_outputParticleCount = newParticleCount;
	AffineTransformation simCell = expectSimulationCell()->cellMatrix();

	for(DataObject* outobj : _output.objects()) {
		OORef<ParticlePropertyObject> originalOutputProperty = dynamic_object_cast<ParticlePropertyObject>(outobj);
		if(!originalOutputProperty)
			continue;

		OVITO_ASSERT(originalOutputProperty->size() == oldParticleCount);

		// Create copy.
		OORef<ParticlePropertyObject> newProperty = cloneHelper()->cloneObject(originalOutputProperty, false);
		newProperty->resize(newParticleCount, false);

		OVITO_ASSERT(originalOutputProperty->size() == oldParticleCount);
		size_t destinationIndex = 0;

		for(int imageX = newImages.minc.x(); imageX <= newImages.maxc.x(); imageX++) {
			for(int imageY = newImages.minc.y(); imageY <= newImages.maxc.y(); imageY++) {
				for(int imageZ = newImages.minc.z(); imageZ <= newImages.maxc.z(); imageZ++) {

					// Duplicate property data.
					memcpy((char*)newProperty->data() + (destinationIndex * newProperty->stride()),
							originalOutputProperty->constData(), newProperty->stride() * oldParticleCount);

					if(newProperty->type() == ParticleProperty::PositionProperty && (imageX != 0 || imageY != 0 || imageZ != 0)) {
						// Shift particle positions by the periodicity vector.
						const Vector3 imageDelta = simCell * Vector3(imageX, imageY, imageZ);

						const Point3* pend = newProperty->dataPoint3() + (destinationIndex + oldParticleCount);
						for(Point3* p = newProperty->dataPoint3() + destinationIndex; p != pend; ++p)
							*p += imageDelta;
					}

					destinationIndex += oldParticleCount;
				}
			}
		}

		// Assign unique IDs to duplicated particle.
		if(uniqueIdentifiers() && newProperty->type() == ParticleProperty::IdentifierProperty) {
			auto minmax = std::minmax_element(newProperty->constDataInt(), newProperty->constDataInt() + oldParticleCount);
			int minID = *minmax.first;
			int maxID = *minmax.second;
			for(size_t c = 1; c < numCopies; c++) {
				int offset = (maxID - minID + 1) * c;
				for(auto id = newProperty->dataInt() + c * oldParticleCount, id_end = id + oldParticleCount; id != id_end; ++id)
					*id += offset;
			}
		}

		// Replace original property with the modified one.
		_output.replaceObject(originalOutputProperty, newProperty);
	}

	// Extend simulation box if requested.
	if(adjustBoxSize()) {
		simCell.translation() += (FloatType)newImages.minc.x() * simCell.column(0);
		simCell.translation() += (FloatType)newImages.minc.y() * simCell.column(1);
		simCell.translation() += (FloatType)newImages.minc.z() * simCell.column(2);
		simCell.column(0) *= nPBC[0];
		simCell.column(1) *= nPBC[1];
		simCell.column(2) *= nPBC[2];
		outputSimulationCell()->setCellMatrix(simCell);
	}

	// Replicate bonds.
	for(DataObject* outobj : _output.objects()) {
		OORef<BondsObject> originalOutputBonds = dynamic_object_cast<BondsObject>(outobj);
		if(!originalOutputBonds)
			continue;

		OORef<BondsObject> newBondsObj = cloneHelper()->cloneObject(originalOutputBonds, false);

		// Duplicate bonds and adjust particle indices and PBC shift vectors as needed.
		// Some bonds may no longer cross periodic boundaries.
		size_t oldBondCount = newBondsObj->bonds().size();
		newBondsObj->modifiableBonds().resize(oldBondCount * numCopies);
		auto outBond = newBondsObj->modifiableBonds().begin();
		Point3I image;
		for(image[0] = newImages.minc.x(); image[0] <= newImages.maxc.x(); image[0]++) {
			for(image[1] = newImages.minc.y(); image[1] <= newImages.maxc.y(); image[1]++) {
				for(image[2] = newImages.minc.z(); image[2] <= newImages.maxc.z(); image[2]++) {
					auto inBond = originalOutputBonds->bonds().begin();
					for(size_t bindex = 0; bindex < oldBondCount; bindex++, ++inBond, ++outBond) {
						Point3I newImage;
						Vector_3<int8_t> newShift;
						for(size_t dim = 0; dim < 3; dim++) {
							int i = image[dim] + (int)inBond->pbcShift[dim] - newImages.minc[dim];
							newImage[dim] = SimulationCell::modulo(i, nPBC[dim]) + newImages.minc[dim];
							newShift[dim] = i >= 0 ? (i / nPBC[dim]) : ((i-nPBC[dim]+1) / nPBC[dim]);
							if(!adjustBoxSize())
								newShift[dim] *= nPBC[dim];
						}
						OVITO_ASSERT(newImage.x() >= newImages.minc.x() && newImage.x() <= newImages.maxc.x());
						OVITO_ASSERT(newImage.y() >= newImages.minc.y() && newImage.y() <= newImages.maxc.y());
						OVITO_ASSERT(newImage.z() >= newImages.minc.z() && newImage.z() <= newImages.maxc.z());
						size_t imageIndex1 =   ((image.x()-newImages.minc.x()) * nPBC[1] * nPBC[2])
										 	 + ((image.y()-newImages.minc.y()) * nPBC[2])
											 +  (image.z()-newImages.minc.z());
						size_t imageIndex2 =   ((newImage.x()-newImages.minc.x()) * nPBC[1] * nPBC[2])
										 	 + ((newImage.y()-newImages.minc.y()) * nPBC[2])
											 +  (newImage.z()-newImages.minc.z());
						outBond->pbcShift = newShift;
						outBond->index1 = inBond->index1 + imageIndex1 * oldParticleCount;
						outBond->index2 = inBond->index2 + imageIndex2 * oldParticleCount;
						OVITO_ASSERT(outBond->index1 < newParticleCount);
						OVITO_ASSERT(outBond->index2 < newParticleCount);
					}
				}
			}
		}

		newBondsObj->changed();
		// Replace original property with the modified one.
		_output.replaceObject(originalOutputBonds, newBondsObj);
	}

	return PipelineStatus::Success;
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void ShowPeriodicImagesModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	QWidget* panel = createRollout(tr("Show periodic images"), rolloutParams, "particles.modifiers.show_periodic_images.html");

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(panel);
	layout->setContentsMargins(4,4,4,4);
#ifndef Q_OS_MACX
	layout->setHorizontalSpacing(2);
	layout->setVerticalSpacing(2);
#endif
	layout->setColumnStretch(1, 1);

	BooleanParameterUI* showPeriodicImageXUI = new BooleanParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_showImageX));
	layout->addWidget(showPeriodicImageXUI->checkBox(), 0, 0);
	IntegerParameterUI* numImagesXPUI = new IntegerParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_numImagesX));
	numImagesXPUI->setMinValue(1);
	layout->addLayout(numImagesXPUI->createFieldLayout(), 0, 1);

	BooleanParameterUI* showPeriodicImageYUI = new BooleanParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_showImageY));
	layout->addWidget(showPeriodicImageYUI->checkBox(), 1, 0);
	IntegerParameterUI* numImagesYPUI = new IntegerParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_numImagesY));
	numImagesYPUI->setMinValue(1);
	layout->addLayout(numImagesYPUI->createFieldLayout(), 1, 1);

	BooleanParameterUI* showPeriodicImageZUI = new BooleanParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_showImageZ));
	layout->addWidget(showPeriodicImageZUI->checkBox(), 2, 0);
	IntegerParameterUI* numImagesZPUI = new IntegerParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_numImagesZ));
	numImagesZPUI->setMinValue(1);
	layout->addLayout(numImagesZPUI->createFieldLayout(), 2, 1);

	BooleanParameterUI* adjustBoxSizeUI = new BooleanParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_adjustBoxSize));
	layout->addWidget(adjustBoxSizeUI->checkBox(), 3, 0, 1, 2);

	BooleanParameterUI* uniqueIdentifiersUI = new BooleanParameterUI(this, PROPERTY_FIELD(ShowPeriodicImagesModifier::_uniqueIdentifiers));
	layout->addWidget(uniqueIdentifiersUI->checkBox(), 4, 0, 1, 2);
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
