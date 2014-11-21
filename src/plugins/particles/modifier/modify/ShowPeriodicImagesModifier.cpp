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
#include "ShowPeriodicImagesModifier.h"

namespace Ovito { namespace Plugins { namespace Particles { namespace Modifiers { namespace Modify {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, ShowPeriodicImagesModifier, ParticleModifier);
SET_OVITO_OBJECT_EDITOR(ShowPeriodicImagesModifier, Internal::ShowPeriodicImagesModifierEditor);
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

namespace Internal {
	IMPLEMENT_OVITO_OBJECT(Particles, ShowPeriodicImagesModifierEditor, ParticleModifierEditor);
}

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
	int nPBCx = showImageX() ? std::max(numImagesX(),1) : 1;
	int nPBCy = showImageY() ? std::max(numImagesY(),1) : 1;
	int nPBCz = showImageZ() ? std::max(numImagesZ(),1) : 1;

	// Calculate new number of atoms.
	size_t numCopies = nPBCx * nPBCy * nPBCz;
	if(numCopies <= 1 || inputParticleCount() == 0)
		return PipelineStatus::Success;

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
		size_t destinationIndex = oldParticleCount;

		// Copy original property data.
		memcpy((char*)newProperty->data(), originalOutputProperty->constData(), newProperty->stride() * oldParticleCount);

		for(int imageX = -(nPBCx-1)/2; imageX <= nPBCx/2; imageX++) {
			for(int imageY = -(nPBCy-1)/2; imageY <= nPBCy/2; imageY++) {
				for(int imageZ = -(nPBCz-1)/2; imageZ <= nPBCz/2; imageZ++) {
					if(imageX == 0 && imageY == 0 && imageZ == 0)
						continue;

					// Duplicate property data.
					memcpy((char*)newProperty->data() + (destinationIndex * newProperty->stride()),
							originalOutputProperty->constData(), newProperty->stride() * oldParticleCount);

					if(newProperty->type() == ParticleProperty::PositionProperty) {
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

	if(adjustBoxSize()) {
		simCell.column(3) -= (FloatType)((nPBCx-1)/2) * simCell.column(0);
		simCell.column(3) -= (FloatType)((nPBCy-1)/2) * simCell.column(1);
		simCell.column(3) -= (FloatType)((nPBCz-1)/2) * simCell.column(2);
		simCell.column(0) *= nPBCx;
		simCell.column(1) *= nPBCy;
		simCell.column(2) *= nPBCz;
		outputSimulationCell()->setCellMatrix(simCell);
	}

	return PipelineStatus::Success;
}

namespace Internal {

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

}	// End of namespace

}}}}}	// End of namespace
