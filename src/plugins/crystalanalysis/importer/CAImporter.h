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

#ifndef __OVITO_CRYSTALANALYSIS_IMPORTER_H
#define __OVITO_CRYSTALANALYSIS_IMPORTER_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <core/dataset/importexport/LinkedFileImporter.h>
#include <core/scene/objects/geometry/HalfEdgeMesh.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <plugins/particles/importer/ParticleImportTask.h>
#include <plugins/crystalanalysis/data/patterns/StructurePattern.h>

namespace CrystalAnalysis {

using namespace Ovito;
using namespace Particles;

/**
 * \brief Importer for output files generated by the Crystal Analysis Tool.
 */
class OVITO_CRYSTALANALYSIS_EXPORT CAImporter : public LinkedFileImporter
{
public:

	/// \brief Constructs a new instance of this class.
	Q_INVOKABLE CAImporter(DataSet* dataset) : LinkedFileImporter(dataset), _loadParticles(false) {
		INIT_PROPERTY_FIELD(CAImporter::_loadParticles);
	}

	/// \brief Returns the file filter that specifies the files that can be imported by this service.
	virtual QString fileFilter() override { return QString("*"); }

	/// \brief Returns the filter description that is displayed in the drop-down box of the file dialog.
	virtual QString fileFilterDescription() override { return tr("Crystal Analysis files"); }

	/// \brief Checks if the given file has format that can be read by this importer.
	virtual bool checkFileFormat(QFileDevice& input, const QUrl& sourceLocation) override;

	/// Returns the title of this object.
	virtual QString objectTitle() override { return tr("CAT Output"); }

	/// Returns whether loading of the associated particle file is enabled.
	bool loadParticles() const { return _loadParticles; }

	/// Controls the loading of the associated particle file.
	void setLoadParticles(bool enable) { _loadParticles = enable; }

protected:

	/// The format-specific task object that is responsible for reading an input file in the background.
	class CrystalAnalysisImportTask : public ParticleImportTask
	{
	public:

		/// Normal constructor.
		CrystalAnalysisImportTask(const LinkedFileImporter::FrameSourceInformation& frame, bool loadParticles) : ParticleImportTask(frame, true), _loadParticles(loadParticles) {}

		/// Lets the data container insert the data it holds into the scene by creating
		/// appropriate scene objects.
		virtual QSet<SceneObject*> insertIntoScene(LinkedFileObject* destination) override;

	protected:

		/// Parses the given input file and stores the data in this container object.
		virtual void parseFile(FutureInterfaceBase& futureInterface, CompressedTextParserStream& stream) override;

		struct BurgersVectorFamilyInfo {
			int id;
			QString name;
			Vector3 burgersVector;
			Color color;
		};

		struct PatternInfo {
			int id;
			StructurePattern::StructureType type;
			QString shortName;
			QString longName;
			Color color;
			QVector<BurgersVectorFamilyInfo> burgersVectorFamilies;
		};

		struct ClusterInfo {
			int id;
			int proc;
			int patternIndex;
			int atomCount;
			Point3 centerOfMass;
			Matrix3 orientation;
		};

		struct ClusterTransitionInfo {
			int cluster1, cluster2;
			Matrix3 tm;
		};

		struct DislocationSegmentInfo {
			int id;
			Vector3 burgersVector;
			int clusterIndex;
			QVector<Point3> line;
			QVector<int> coreSize;
			bool isClosedLoop;
		};

		/// The triangle mesh of the defect surface.
		HalfEdgeMesh _defectSurface;

		/// The structure pattern catalog.
		QVector<PatternInfo> _patterns;

		/// The cluster list.
		QVector<ClusterInfo> _clusters;

		/// The cluster transition list.
		QVector<ClusterTransitionInfo> _clusterTransitions;

		/// The dislocation segments.
		QVector<DislocationSegmentInfo> _dislocations;

		/// Controls whether particles should be loaded too.
		bool _loadParticles;

		/// This is the sub-task task that loads the particles.
		LinkedFileImporter::ImportTaskPtr _particleLoadTask;
	};

	/// \brief Creates an import task object to read the given frame.
	virtual ImportTaskPtr createImportTask(const FrameSourceInformation& frame) override {
		return std::make_shared<CrystalAnalysisImportTask>(frame, _loadParticles);
	}

	/// This method is called when the scene node for the LinkedFileObject is created.
	virtual void prepareSceneNode(ObjectNode* node, LinkedFileObject* importObj) override;

	/// \brief Is called when the value of a property of this object has changed.
	virtual void propertyChanged(const PropertyFieldDescriptor& field) override;

private:

	/// Controls whether the associated particle file should be loaded too.
	PropertyField<bool> _loadParticles;

	Q_OBJECT
	OVITO_OBJECT

	DECLARE_PROPERTY_FIELD(_loadParticles);
};

/**
 * \brief A properties editor for the CAImporter class.
 */
class CAImporterEditor : public PropertiesEditor
{
public:

	/// Constructor.
	Q_INVOKABLE CAImporterEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

private:

	Q_OBJECT
	OVITO_OBJECT
};

};

#endif // __OVITO_CRYSTALANALYSIS_IMPORTER_H
