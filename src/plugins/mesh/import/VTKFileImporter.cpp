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
#include <core/utilities/io/CompressedTextReader.h>
#include "VTKFileImporter.h"

namespace Mesh {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Mesh, VTKFileImporter, FileSourceImporter);

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool VTKFileImporter::checkFileFormat(QFileDevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextReader stream(input, sourceLocation.path());

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
void VTKFileImporter::VTKFileImportTask::parseFile(CompressedTextReader& stream)
{
	setProgressText(tr("Reading VTK file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

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
		throw Exception(tr("Can read only text-based VTK files (ASCII format)."));

	// Read data set type.
	stream.readLine();
	bool isPolyData;
	if(stream.lineStartsWith("DATASET UNSTRUCTURED_GRID"))
		isPolyData = false;
	else if(stream.lineStartsWith("DATASET POLYDATA"))
		isPolyData = true;
	else
		throw Exception(tr("Can read only read VTK files containing triangle polydata or unstructured grids with triangle cells."));

	// Read point count.
	stream.readLine();
	if(!stream.lineStartsWith("POINTS"))
		throw Exception(tr("Invalid VTK file. Expected POINTS token in line %1 but found %2").arg(stream.lineNumber()).arg(stream.lineString()));
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

	std::string line;
	do { line = stream.readLine(); }
	while(line.find_first_not_of(" \t\n\r") == std::string::npos);
	int triangleCount;
	if(!isPolyData) {
		// Parse number of cells.
		if(!stream.lineStartsWith("CELLS"))
			throw Exception(tr("Invalid VTK file. Expected token CELLS in line %1, but found %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(sscanf(stream.line() + 5, "%i", &triangleCount) != 1 || triangleCount < 0)
			throw Exception(tr("Invalid number of cells in VTK file (line %2): %1").arg(stream.lineNumber()).arg(stream.lineString()));
	}
	else {
		// Parse number of polygons.
		if(!stream.lineStartsWith("POLYGONS"))
			throw Exception(tr("Invalid VTK file. Expected token POLYGONS in line %1, but found %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(sscanf(stream.line() + 8, "%i", &triangleCount) != 1 || triangleCount < 0)
			throw Exception(tr("Invalid number of polygons in VTK file (line %2): %1").arg(stream.lineNumber()).arg(stream.lineString()));
	}
	// Parse triangles.
	mesh().setFaceCount(triangleCount);
	auto f = mesh().faces().begin();
	for(int i = 0; i < triangleCount; i++, ++f) {
		int vcount = 0, a, b, c, token_count;
		token_count = sscanf(stream.readLine(), "%i %i %i %i", &vcount, &a, &b, &c);
		if(vcount != 3)
			throw Exception(tr("Only triangle cells/polygons are supported in VTK files. Wrong number of vertices in line %1 of VTK file: %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(token_count != 4)
			throw Exception(tr("Invalid triangle in VTK file (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		if(a >= pointCount || b >= pointCount || c >= pointCount)
			throw Exception(tr("Vertex indices out of range in triangle cell (line %1): %2").arg(stream.lineNumber()).arg(stream.lineString()));
		f->setVertices(a,b,c);
	}
	mesh().invalidateFaces();

	if(!isPolyData) {
		// Parse cell types
		do { line = stream.readLine(); }
		while(line.find_first_not_of(" \t\n\r") == std::string::npos);
		if(!stream.lineStartsWith("CELL_TYPES"))
			throw Exception(tr("Invalid VTK file. Expected token CELL_TYPES in line %1, but found %2").arg(stream.lineNumber()).arg(stream.lineString()));
		for(int i = 0; i < triangleCount; i++) {
			int t;
			if(sscanf(stream.readLine(), "%i", &t) != 1 || t != 5)
				throw Exception(tr("Invalid cell type in VTK file (line %1): %2. Only triangle cells are supported by OVITO.").arg(stream.lineNumber()).arg(stream.lineString()));
		}

		// Look for color information.
		for(; !stream.eof() && !stream.lineStartsWith("CELL_DATA"); )
			line = stream.readLine();
		for(; !stream.eof() && !stream.lineStartsWith("COLOR_SCALARS"); )
			line = stream.readLine();
		if(!stream.eof()) {
			int componentCount = 0;
			if(sscanf(stream.line(), "COLOR_SCALARS %*s %i", &componentCount) != 1 || (componentCount != 3 && componentCount != 4))
				throw Exception(tr("Invalid COLOR_SCALARS property in line %1 of VTK file. Component count must be 3 or 4.").arg(stream.lineNumber()));
			mesh().setHasFaceColors(true);
			auto& faceColors = mesh().faceColors();
			std::fill(faceColors.begin(), faceColors.end(), ColorA(1,1,1,1));
			component = 0;
			for(int i = 0; i < triangleCount;) {
				if(stream.eof())
					throw Exception(tr("Unexpected end of VTK file in line %1.").arg(stream.lineNumber()));
				const char* s = stream.readLine();
				for(; ;) {
					while(*s <= ' ' && *s != '\0') ++s;			// Skip whitespace in front of token
					if(!*s || i >= triangleCount) break;
					faceColors[i][component++] = (FloatType)std::atof(s);
					if(component == componentCount) {
						component = 0;
						++i;
					}
					while(*s > ' ') ++s;						// Proceed to end of token
				}
			}
			mesh().invalidateFaces();
		}
	}
	else {
		// ...to be implemented.
	}

	setStatus(tr("%1 vertices, %2 triangles").arg(pointCount).arg(mesh().faceCount()));
}

};
