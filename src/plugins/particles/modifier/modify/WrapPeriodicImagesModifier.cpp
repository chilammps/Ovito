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
#include <plugins/particles/objects/BondsObject.h>
#include "WrapPeriodicImagesModifier.h"

#include <QtConcurrent>

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Modify)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Particles, WrapPeriodicImagesModifier, ParticleModifier);

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
WrapPeriodicImagesModifier::WrapPeriodicImagesModifier(DataSet* dataset) : ParticleModifier(dataset)
{
}

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
PipelineStatus WrapPeriodicImagesModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
	std::array<bool, 3> pbc = expectSimulationCell()->pbcFlags();
	if(!pbc[0] && !pbc[1] && !pbc[2])
		return PipelineStatus(PipelineStatus::Warning, tr("The simulation cell has no periodic boundary conditions."));

	AffineTransformation simCell = expectSimulationCell()->cellMatrix();
	if(std::abs(simCell.determinant()) < FLOATTYPE_EPSILON)
		 throw Exception(tr("The simulation cell is degenerated."));
	AffineTransformation inverseSimCell = simCell.inverse();

	expectStandardProperty(ParticleProperty::PositionProperty);
	ParticlePropertyObject* posProperty = outputStandardProperty(ParticleProperty::PositionProperty, true);

	// Wrap bonds
	for(const VersionedOORef<DataObject>& obj : output().objects()) {
		BondsObject* bondsObj = dynamic_object_cast<BondsObject>(obj.get());
		if(bondsObj) {
			// Is the object still a shallow copy of the input?
			if(input().contains(bondsObj)) {
				// If yes, make a real copy of the object, which may be modified.
				OORef<BondsObject> newObject = cloneHelper()->cloneObject(bondsObj, false);
				output().replaceObject(bondsObj, newObject);
				bondsObj = newObject;
			}

			// Wrap bonds by adjusting their shift vectors.
			for(BondsStorage::Bond& bond : bondsObj->modifiableBonds()) {
				const Point3& p1 = posProperty->getPoint3(bond.index1);
				const Point3& p2 = posProperty->getPoint3(bond.index2);
				for(size_t dim = 0; dim < 3; dim++) {
					if(pbc[dim]) {
						bond.pbcShift[dim] -= (int8_t)floor(inverseSimCell.prodrow(p1, dim));
						bond.pbcShift[dim] += (int8_t)floor(inverseSimCell.prodrow(p2, dim));
					}
				}
			}
			bondsObj->changed();
		}
	}

	// Wrap particles coordinates
	Point3* pbegin = posProperty->dataPoint3();
	Point3* pend = pbegin + posProperty->size();

	if(pbc[0]) QtConcurrent::blockingMap(pbegin, pend, [simCell, inverseSimCell](Point3& p) {
		if(FloatType n = floor(inverseSimCell.prodrow(p, 0)))
			p -= simCell.column(0) * n;
	});
	if(pbc[1]) QtConcurrent::blockingMap(pbegin, pend, [simCell, inverseSimCell](Point3& p) {
		if(FloatType n = floor(inverseSimCell.prodrow(p, 1)))
			p -= simCell.column(1) * n;
	});
	if(pbc[2]) QtConcurrent::blockingMap(pbegin, pend, [simCell, inverseSimCell](Point3& p) {
		if(FloatType n = floor(inverseSimCell.prodrow(p, 2)))
			p -= simCell.column(2) * n;
	});
	posProperty->changed();

	return PipelineStatus::Success;
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
