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

#ifndef __OVITO_TRIMESH_LOADER_H
#define __OVITO_TRIMESH_LOADER_H

#include <core/dataset/importexport/FileSourceImporter.h>
#include <core/utilities/io/CompressedTextReader.h>
#include <core/utilities/mesh/TriMesh.h>
#include <plugins/mesh/Mesh.h>

namespace Mesh {

using namespace Ovito;

/**
 * \brief Base class for file loader reading a triangle mesh from a file.
 */
class TriMeshLoader : public FileSourceImporter::FrameLoader
{
public:

	/// Constructor.
	TriMeshLoader(DataSetContainer* container, const FileSourceImporter::Frame& frame)
		: FileSourceImporter::FrameLoader(container, frame) {}

	/// Loads the requested frame data from the external file.
	virtual void perform() override;

	/// Inserts the data loaded by perform() into the provided container object. This function is
	/// called by the system from the main thread after the asynchronous loading task has finished.
	virtual void handOver(CompoundObject* container) override;

	/// Returns the triangle mesh data structure.
	const TriMesh& mesh() const { return _mesh; }

	/// Returns a reference to the triangle mesh data structure.
	TriMesh& mesh() { return _mesh; }

protected:

	/// Parses the given input file and stores the data in this container object.
	virtual void parseFile(CompressedTextReader& stream) = 0;

private:

	/// The triangle mesh.
	TriMesh _mesh;
};

};

#endif // __OVITO_TRIMESH_LOADER_H
