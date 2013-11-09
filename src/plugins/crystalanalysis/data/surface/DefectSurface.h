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

#ifndef __OVITO_CA_DEFECT_SURFACE_H
#define __OVITO_CA_DEFECT_SURFACE_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/scene/objects/SceneObject.h>
#include <core/scene/objects/geometry/HalfEdgeMesh.h>

namespace CrystalAnalysis {

using namespace Ovito;

/**
 * \brief Wraps the defect surface mesh of the CALib.
 */
class OVITO_CRYSTALANALYSIS_EXPORT DefectSurface : public SceneObject
{
public:

	/// \brief Default constructor that creates an empty DefectSurface object.
	Q_INVOKABLE DefectSurface();

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("Defect surface"); }

	/// \brief Returns whether this object, when returned as an editable sub-object by another object,
	///        should be displayed in the modification stack.
	///
	/// Return false because this object cannot be edited.
	virtual bool isSubObjectEditable() const override { return false; }

	/// Returns a const-reference to the mesh encapsulated by this scene object.
	const HalfEdgeMesh& mesh() const { return _mesh; }

	/// Returns a reference to the mesh encapsulated by this scene object.
	/// The reference can be used to modify the mesh. However, each time the mesh has been modified,
	/// this->notifyDependents(ReferenceEvent::TargetChanged) must be called to increment
	/// the scene object's revision number.
	HalfEdgeMesh& mesh() { return _mesh; }

protected:

	/// Creates a copy of this object.
	virtual OORef<RefTarget> clone(bool deepCopy, CloneHelper& cloneHelper) override;

private:

	/// The triangle mesh.
	HalfEdgeMesh _mesh;

	Q_OBJECT
	OVITO_OBJECT
};

};	// End of namespace

#endif // __OVITO_CA_DEFECT_SURFACE_H
