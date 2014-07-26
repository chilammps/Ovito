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

#include <plugins/mesh/Mesh.h>
#include <core/utilities/io/CompressedTextParserStream.h>
#include "VTKFileImporter.h"

namespace Mesh {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Mesh, VTKFileImporter, LinkedFileImporter);

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool VTKFileImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
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
		throw Exception(tr("Can read only VTK files storing unstructured grids."));

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
	size_t component = 0;
	for(int i = 0; i < pointCount; ) {
		if(stream.eof())
			throw Exception(tr("Unexpected end of VTK file in line %1.").arg(stream.lineNumber()));
		const char* s = stream.readLine();
		for(;;) {
			while(*s <= ' ' && *s != '\0') ++s;			// Skip whitespace in front of token
			if(*s == '\0' || i >= pointCount) break;
			(*v)[component++] = (FloatType)std::atof(s);
			if(component == 3) {
				component = 0;
				++v; ++i;
			}
			while(*s > ' ') ++s;						// Proceed to end of token
		}

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
		int vcount = 0, a, b, c, token_count;
		token_count = sscanf(stream.readLine(), "%i %i %i %i", &vcount, &a, &b, &c);
		if(vcount != 3)
			throw Exception(tr("Only triangle cells are supported in VTK files. Wrong number of cell vertices in line %1 of VTK file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(token_count != 4)
			throw Exception(tr("Invalid triangle cell in VTK file (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(a >= pointCount || b >= pointCount || c >= pointCount)
			throw Exception(tr("Vertex indices out of range in triangle cell (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		f->setVertices(a,b,c);
	}
	mesh().invalidateFaces();

	setInfoText(tr("%1 vertices, %2 triangles").arg(pointCount).arg(cellCount));
}

};
