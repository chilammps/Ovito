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
#include "WrapPeriodicImagesModifier.h"

#include <QtConcurrent>

namespace Ovito { namespace Particles { namespace Modifiers { namespace Modify {

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

}}}}	// End of namespace
