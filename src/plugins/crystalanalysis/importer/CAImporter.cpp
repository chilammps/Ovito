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
#include <core/utilities/concurrent/Future.h>
#include <core/dataset/importexport/LinkedFileObject.h>
#include <core/scene/ObjectNode.h>
#include <plugins/crystalanalysis/data/surface/DefectSurface.h>
#include <plugins/crystalanalysis/modifier/SmoothSurfaceModifier.h>
#include "CAImporter.h"

namespace CrystalAnalysis {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(CrystalAnalysis, CAImporter, LinkedFileImporter)

/******************************************************************************
* Checks if the given file has format that can be read by this importer.
******************************************************************************/
bool CAImporter::checkFileFormat(QIODevice& input, const QUrl& sourceLocation)
{
	// Open input file.
	CompressedTextParserStream stream(input, sourceLocation.path());

	// Read first line.
	stream.readLine(20);

	// Files start with the string "CA_FILE_VERSION ".
	if(stream.lineStartsWith("CA_FILE_VERSION "))
		return true;

	return false;

}
/******************************************************************************
* Reads the data from the input file(s).
******************************************************************************/
void CAImporter::CrystalAnalysisImportTask::parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream)
{
	futureInterface.setProgressText(tr("Reading crystal analysis file %1").arg(frame().sourceFile.toString(QUrl::RemovePassword | QUrl::PreferLocalFile | QUrl::PrettyDecoded)));

	// Regular expression for whitespace characters.
	QRegularExpression ws_re(QStringLiteral("\\s+"));

	// Read file header.
	stream.readLine();
	if(!stream.lineStartsWith("CA_FILE_VERSION "))
		throw Exception(tr("Failed to parse file. This is not a proper file written by the Crystal Analysis Tool."));
	int fileFormatVersion = 0;
	if(sscanf(stream.line(), "CA_FILE_VERSION %i", &fileFormatVersion) != 1)
		throw Exception(tr("Failed to parse file. This is not a proper file written by the Crystal Analysis Tool."));
	if(fileFormatVersion != 4)
		throw Exception(tr("Failed to parse file. This file format version is not supported: %1").arg(fileFormatVersion));
	stream.readLine();
	if(!stream.lineStartsWith("CA_LIB_VERSION"))
		throw Exception(tr("Failed to parse file. This is not a proper file written by the Crystal Analysis Tool."));

	// Skip file path information.
	stream.readLine();
	stream.readLine();

	// Read pattern catalog.
	int numPatterns;
	if(sscanf(stream.readLine(), "STRUCTURE_PATTERNS %i", &numPatterns) != 1 || numPatterns <= 0)
		throw Exception(tr("Failed to parse file. Invalid number of structure patterns in line %1.").arg(stream.lineNumber()));
	for(int index = 0; index < numPatterns; index++) {
		int patternId;
		if(sscanf(stream.readLine(), "PATTERN ID %i", &patternId) != 1)
			throw Exception(tr("Failed to parse file. Invalid pattern ID in line %1.").arg(stream.lineNumber()));
		stream.readLine();
		QString shortPatternName = stream.lineString().mid(5).trimmed();
		stream.readLine();
		QString longPatternName = stream.lineString().mid(9).trimmed();
		stream.readLine();
		QString patternType = stream.lineString().mid(5).trimmed();
		Color color;
		if(sscanf(stream.readLine(), "COLOR " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &color.r(), &color.g(), &color.b()) != 3)
			throw Exception(tr("Failed to parse file. Invalid pattern color in line %1.").arg(stream.lineNumber()));
		int numFamilies;
		if(sscanf(stream.readLine(), "BURGERS_VECTOR_FAMILIES %i", &numFamilies) != 1 || numFamilies < 0)
			throw Exception(tr("Failed to parse file. Invalid number of Burgers vectors families in line %1.").arg(stream.lineNumber()));
		for(int familyIndex = 0; familyIndex < numFamilies; familyIndex++) {
			int familyId;
			if(sscanf(stream.readLine(), "BURGERS_VECTOR_FAMILY ID %i", &familyId) != 1)
				throw Exception(tr("Failed to parse file. Invalid Burgers vector family ID in line %1.").arg(stream.lineNumber()));
			stream.readLine();
			QString familyName = stream.lineString().trimmed();
			Vector3 burgersVector;
			if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &burgersVector.x(), &burgersVector.y(), &burgersVector.z()) != 3)
				throw Exception(tr("Failed to parse file. Invalid Burgers vector in line %1.").arg(stream.lineNumber()));
			Color familyColor;
			if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &familyColor.r(), &familyColor.g(), &familyColor.b()) != 3)
				throw Exception(tr("Failed to parse file. Invalid color in line %1.").arg(stream.lineNumber()));
		}
		stream.readLine();
	}

	// Read simulation cell geometry.
	AffineTransformation cell;
	if(sscanf(stream.readLine(), "SIMULATION_CELL_ORIGIN " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &cell(0,3), &cell(1,3), &cell(2,3)) != 3)
		throw Exception(tr("Failed to parse file. Invalid cell origin in line %1.").arg(stream.lineNumber()));
	if(sscanf(stream.readLine(), "SIMULATION_CELL " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING
			" " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING
			" " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
			&cell(0,0), &cell(0,1), &cell(0,2), &cell(1,0), &cell(1,1), &cell(1,2), &cell(2,0), &cell(2,1), &cell(2,2)) != 9)
		throw Exception(tr("Failed to parse file. Invalid cell vectors in line %1.").arg(stream.lineNumber()));
	int pbcFlags[3];
	if(sscanf(stream.readLine(), "PBC_FLAGS %i %i %i", &pbcFlags[0], &pbcFlags[1], &pbcFlags[2]) != 3)
		throw Exception(tr("Failed to parse file. Invalid PBC flags in line %1.").arg(stream.lineNumber()));
	simulationCell().setMatrix(cell);
	simulationCell().setPbcFlags(pbcFlags[0], pbcFlags[1], pbcFlags[2]);

	// Read cluster list.
	int numClusters;
	if(sscanf(stream.readLine(), "CLUSTERS %i", &numClusters) != 1)
		throw Exception(tr("Failed to parse file. Invalid number of clusters in line %1.").arg(stream.lineNumber()));
	for(int index = 0; index < numClusters; index++) {
		stream.readLine();
		int clusterId, clusterProc, patternIndex, atomCount;
		if(sscanf(stream.readLine(), "%i %i", &clusterId, &clusterProc) != 2)
			throw Exception(tr("Failed to parse file. Invalid cluster ID in line %1.").arg(stream.lineNumber()));
		if(sscanf(stream.readLine(), "%i", &patternIndex) != 1)
			throw Exception(tr("Failed to parse file. Invalid cluster pattern index in line %1.").arg(stream.lineNumber()));
		if(sscanf(stream.readLine(), "%i", &atomCount) != 1)
			throw Exception(tr("Failed to parse file. Invalid cluster atom count in line %1.").arg(stream.lineNumber()));
		Point3 centerOfMass;
		if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &centerOfMass.x(), &centerOfMass.y(), &centerOfMass.z()) != 3)
			throw Exception(tr("Failed to parse file. Invalid cluster center of mass in line %1.").arg(stream.lineNumber()));
		Matrix3 orientation;
		if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING
				" " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING
				" " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
				&orientation(0,0), &orientation(0,1), &orientation(0,2), &orientation(1,0), &orientation(1,1), &orientation(1,2), &orientation(2,0), &orientation(2,1), &orientation(2,2)) != 9)
			throw Exception(tr("Failed to parse file. Invalid cluster orientation matrix in line %1.").arg(stream.lineNumber()));
	}

	// Read cluster transition list.
	int numClusterTransitions;
	if(sscanf(stream.readLine(), "CLUSTER_TRANSITIONS %i", &numClusterTransitions) != 1)
		throw Exception(tr("Failed to parse file. Invalid number of cluster transitions in line %1.").arg(stream.lineNumber()));
	for(int index = 0; index < numClusterTransitions; index++) {
		int clusterIndex1, clusterIndex2;
		if(sscanf(stream.readLine(), "TRANSITION %i %i", &clusterIndex1, &clusterIndex2) != 2 || clusterIndex1 >= numClusters || clusterIndex2 >= numClusters)
			throw Exception(tr("Failed to parse file. Invalid cluster transition in line %1.").arg(stream.lineNumber()));
		Matrix3 tm;
		if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING
				" " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING
				" " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING,
				&tm(0,0), &tm(0,1), &tm(0,2), &tm(1,0), &tm(1,1), &tm(1,2), &tm(2,0), &tm(2,1), &tm(2,2)) != 9)
			throw Exception(tr("Failed to parse file. Invalid cluster transition matrix in line %1.").arg(stream.lineNumber()));
	}

	// Read dislocations list.
	int numDislocationSegments;
	if(sscanf(stream.readLine(), "DISLOCATIONS %i", &numDislocationSegments) != 1)
		throw Exception(tr("Failed to parse file. Invalid number of dislocation segments in line %1.").arg(stream.lineNumber()));
	for(int index = 0; index < numDislocationSegments; index++) {
		int segmentId;
		if(sscanf(stream.readLine(), "%i", &segmentId) != 1)
			throw Exception(tr("Failed to parse file. Invalid segment ID in line %1.").arg(stream.lineNumber()));

		Vector3 burgersVector;
		if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &burgersVector.x(), &burgersVector.y(), &burgersVector.z()) != 3)
			throw Exception(tr("Failed to parse file. Invalid Burgers vector in line %1.").arg(stream.lineNumber()));

		int clusterIndex;
		if(sscanf(stream.readLine(), "%i", &clusterIndex) != 1 || clusterIndex < 0 || clusterIndex >= numClusters)
			throw Exception(tr("Failed to parse file. Invalid segment cluster ID in line %1.").arg(stream.lineNumber()));

		// Read polyline.
		int numPoints;
		if(sscanf(stream.readLine(), "%i", &numPoints) != 1 || numPoints <= 1)
			throw Exception(tr("Failed to parse file. Invalid segment number of points in line %1.").arg(stream.lineNumber()));
		for(int pindex = 0; pindex < numPoints; pindex++) {
			Point3 p;
			if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &p.x(), &p.y(), &p.z()) != 3)
				throw Exception(tr("Failed to parse file. Invalid point in line %1.").arg(stream.lineNumber()));
		}

		// Read dislocation core size.
		for(int pindex = 0; pindex < numPoints; pindex++) {
			int coreSize;
			if(sscanf(stream.readLine(), "%i", &coreSize) != 1)
				throw Exception(tr("Failed to parse file. Invalid core size in line %1.").arg(stream.lineNumber()));
		}
	}

	// Read dislocation junctions.
	stream.readLine();
	for(int index = 0; index < numDislocationSegments; index++) {
		for(int nodeIndex = 0; nodeIndex < 2; nodeIndex++) {
			int isForward, otherSegmentId;
			if(sscanf(stream.readLine(), "%i %i", &isForward, &otherSegmentId) != 2)
				throw Exception(tr("Failed to parse file. Invalid dislocation junction record in line %1.").arg(stream.lineNumber()));
		}
	}

	// Read defect mesh vertices.
	int numDefectMeshVertices;
	if(sscanf(stream.readLine(), "DEFECT_MESH_VERTICES %i", &numDefectMeshVertices) != 1)
		throw Exception(tr("Failed to parse file. Invalid number of defect mesh vertices in line %1.").arg(stream.lineNumber()));
	_defectSurface.reserveVertices(numDefectMeshVertices);
	for(int index = 0; index < numDefectMeshVertices; index++) {
		Point3 p;
		if(sscanf(stream.readLine(), FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING " " FLOATTYPE_SCANF_STRING, &p.x(), &p.y(), &p.z()) != 3)
			throw Exception(tr("Failed to parse file. Invalid point in line %1.").arg(stream.lineNumber()));
		_defectSurface.createVertex(p);
	}

	// Read defect mesh facets.
	int numDefectMeshFacets;
	if(sscanf(stream.readLine(), "DEFECT_MESH_FACETS %i", &numDefectMeshFacets) != 1)
		throw Exception(tr("Failed to parse file. Invalid number of defect mesh facets in line %1.").arg(stream.lineNumber()));
	_defectSurface.reserveFaces(numDefectMeshFacets);
	for(int index = 0; index < numDefectMeshFacets; index++) {
		int v[3];
		if(sscanf(stream.readLine(), "%i %i %i", &v[0], &v[1], &v[2]) != 3)
			throw Exception(tr("Failed to parse file. Invalid triangle facet in line %1.").arg(stream.lineNumber()));
		_defectSurface.createFace({ _defectSurface.vertex(v[0]), _defectSurface.vertex(v[1]), _defectSurface.vertex(v[2]) });
	}

	// Read facet adjacency information.
	for(int index = 0; index < numDefectMeshFacets; index++) {
		int v[3];
		if(sscanf(stream.readLine(), "%i %i %i", &v[0], &v[1], &v[2]) != 3)
			throw Exception(tr("Failed to parse file. Invalid triangle adjacency info in line %1.").arg(stream.lineNumber()));
		HalfEdgeMesh::Edge* edge = _defectSurface.face(index)->edges();
		for(int i = 0; i < 3; i++, edge = edge->nextFaceEdge()) {
			OVITO_CHECK_POINTER(edge);
			if(edge->oppositeEdge() != nullptr) continue;
			HalfEdgeMesh::Face* oppositeFace = _defectSurface.face(v[i]);
			HalfEdgeMesh::Edge* oppositeEdge = oppositeFace->edges();
			do {
				OVITO_CHECK_POINTER(oppositeEdge);
				if(oppositeEdge->vertex1() == edge->vertex2() && oppositeEdge->vertex2() == edge->vertex1()) {
					edge->linkToOppositeEdge(oppositeEdge);
					break;
				}
				oppositeEdge = oppositeEdge->nextFaceEdge();
			}
			while(oppositeEdge != oppositeFace->edges());
			OVITO_ASSERT(edge->oppositeEdge());
		}
	}

	setInfoText(tr("Number of segments: %1").arg(numDislocationSegments));
}

/******************************************************************************
* Lets the data container insert the data it holds into the scene by creating
* appropriate scene objects.
******************************************************************************/
QSet<SceneObject*> CAImporter::CrystalAnalysisImportTask::insertIntoScene(LinkedFileObject* destination)
{
	QSet<SceneObject*> activeObjects = ParticleImportTask::insertIntoScene(destination);

	OORef<DefectSurface> defectSurfaceObj = destination->findSceneObject<DefectSurface>();
	if(!defectSurfaceObj) {
		defectSurfaceObj = new DefectSurface();
		destination->addSceneObject(defectSurfaceObj.get());
	}
	defectSurfaceObj->mesh().swap(_defectSurface);
	defectSurfaceObj->notifyDependents(ReferenceEvent::TargetChanged);
	activeObjects.insert(defectSurfaceObj.get());

	return activeObjects;
}

/******************************************************************************
* This method is called when the scene node for the LinkedFileObject is created.
******************************************************************************/
void CAImporter::prepareSceneNode(ObjectNode* node, LinkedFileObject* importObj)
{
	LinkedFileImporter::prepareSceneNode(node, importObj);

	node->applyModifier(new SmoothSurfaceModifier());
}


};
