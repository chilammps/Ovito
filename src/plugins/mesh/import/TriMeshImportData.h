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

#ifndef __OVITO_TRIMESH_IMPORT_DATA_H
#define __OVITO_TRIMESH_IMPORT_DATA_H

#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/utilities/io/CompressedTextParserStream.h>
#include <core/scene/objects/geometry/TriMesh.h>
#include <plugins/mesh/Mesh.h>

namespace Mesh {

using namespace Ovito;

/**
 * Container structure for triangle mesh data imported by a parser class.
 */
class OVITO_MESH_EXPORT TriMeshImportData : public LinkedFileImporter::ImportTask
{
public:

	/// Constructor.
	TriMeshImportData(const LinkedFileImporter::FrameSourceInformation& frame) : LinkedFileImporter::ImportTask(frame) {}

	/// Is called in the background thread to perform the data file import.
	virtual void load(DataSetContainer& container, FutureInterfaceBase& futureInterface) override;

	/// Lets the data container insert the data it holds into the scene by creating
	/// appropriate scene objects.
	virtual QSet<SceneObject*> insertIntoScene(LinkedFileObject* destination) override;

	/// Returns the triangle mesh data structure.
	const TriMesh& mesh() const { return _mesh; }

	/// Returns a reference to the triangle mesh data structure.
	TriMesh& mesh() { return _mesh; }

protected:

	/// Parses the given input file and stores the data in this container object.
	virtual void parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream) = 0;

private:

	/// The triangle mesh.
	TriMesh _mesh;
};

};

#endif // __OVITO_TRIMESH_IMPORT_DATA_H
