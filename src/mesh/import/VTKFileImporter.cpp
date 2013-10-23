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
#include <core/utilities/io/CompressedTextParserStream.h>
#include "VTKFileImporter.h"

namespace Mesh {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Mesh, VTKFileImporter, LinkedFileImporter)

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool VTKFileImporter::checkFileFormat(QIODevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextParserStream stream(input, sourceLocation.path());

	// Read first line.
	stream.readLine(24);

	// VTK files start with the string "# vtk DataFile Version".
	if(stream.lineStartsWith("# vtk DataFile Version"))
		return true;

	return false;
}

/******************************************************************************
* Parses the given input file and stores the data in the given container object.
******************************************************************************/
void VTKFileImporter::VTKFileImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Reading VTK file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Read first line.
	stream.readLine(256);

	// Check header code in first line.
	if(!stream.lineStartsWith("# vtk DataFile Version"))
		throw Exception(tr("Invalid first line in VTK file."));

	// Ignore comment line.
	stream.readLine();

	// Read encoding type.
	stream.readLine();
	if(!stream.lineStartsWith("ASCII"))
		throw Exception(tr("Can read only text-based VTK files."));

	// Read data set type.
	stream.readLine();
	if(!stream.lineStartsWith("DATASET UNSTRUCTURED_GRID"))
		throw Exception(tr("Can read only VTK files with unstructured grids."));

	// Read point count.
	stream.readLine();
	if(!stream.lineStartsWith("POINTS"))
		throw Exception(tr("Invalid VTK file. Unexpected token in line %1: %2").arg(stream.lineNumber()).arg(stream.lineString()));
	int pointCount;
	if(sscanf(stream.line() + 6, "%i", &pointCount) != 1 || pointCount < 0)
		throw Exception(tr("Invalid number of points in VTK file (line %2): %1").arg(stream.lineNumber()).arg(stream.lineString()));

	// Parse point coordinates.
	mesh().setVertexCount(pointCount);
	auto v = mesh().vertices().begin();
	for(int i = 0; i < pointCount; i++, ++v) {
		if(sscanf(stream.readLine(),
				FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
				&v->x(), &v->y(), &v->z()) != 3)
			throw Exception(tr("Invalid vertex coordinates in VTK file (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
	}
	mesh().invalidateVertices();

	// Parse number of cells.
	std::string line;
	do {
		line = stream.readLine();
	}
	while(line.find_first_not_of(" \t\n\r") == std::string::npos);
	if(!stream.lineStartsWith("CELLS"))
		throw Exception(tr("Invalid VTK file. Unexpected token in line %1: %2").arg(stream.lineNumber()).arg(stream.lineString()));
	int cellCount;
	if(sscanf(stream.line() + 5, "%i", &cellCount) != 1 || cellCount < 0)
		throw Exception(tr("Invalid number of cells in VTK file (line %2): %1").arg(stream.lineNumber()).arg(stream.lineString()));

	// Parse triangles.
	mesh().setFaceCount(cellCount);
	auto f = mesh().faces().begin();
	for(int i = 0; i < cellCount; i++, ++f) {
		int vcount, a, b, c;
		if(sscanf(stream.readLine(), "%i %i %i %i", &vcount, &a, &b, &c) != 4)
			throw Exception(tr("Invalid triangle cell in VTK file (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(vcount != 3)
			throw Exception(tr("Wrong number of cell vertices in VTK file (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(a >= pointCount || b >= pointCount || c >= pointCount)
			throw Exception(tr("Vertex indices out of range in triangle cell (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		f->setVertices(a,b,c);
	}
	mesh().invalidateFaces();
}

};
