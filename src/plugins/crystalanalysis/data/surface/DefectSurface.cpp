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

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include "DefectSurface.h"
#include "DefectSurfaceDisplay.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, DefectSurface, SceneObject)

/******************************************************************************
* Constructs an empty defect surface object.
******************************************************************************/
DefectSurface::DefectSurface()
{
	addDisplayObject(new DefectSurfaceDisplay());
}

/******************************************************************************
* Creates a copy of this object.
******************************************************************************/
OORef<RefTarget> DefectSurface::clone(bool deepCopy, CloneHelper& cloneHelper)
{
	// Let the base class create an instance of this class.
	OORef<DefectSurface> clone = static_object_cast<DefectSurface>(SceneObject::clone(deepCopy, cloneHelper));

	// Copy the internal mesh.
	clone->_mesh = this->_mesh;

	return clone;
}

};	// End of namespace
