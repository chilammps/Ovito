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
#include <core/animation/AnimManager.h>
#include <core/scene/ObjectNode.h>
#include <core/scene/SceneRoot.h>
#include <core/scene/objects/SceneObject.h>
#include <core/dataset/DataSetManager.h>

#include "ParticleExporter.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, ParticleExporter, FileExporter)
DEFINE_PROPERTY_FIELD(ParticleExporter, _outputFilename, "OutputFile")
SET_PROPERTY_FIELD_LABEL(ParticleExporter, _outputFilename, "Output filename")

/******************************************************************************
* Exports the scene to the given file.
******************************************************************************/
bool ParticleExporter::exportToFile(const QString& filePath, DataSet* dataset)
{
	// Save the output path.
	setOutputFile(filePath);

	// Check if the data to be exported are available.
	PipelineFlowState flowState = getParticles(dataset, dataset->animationSettings()->time());
	AtomsObject* atoms = dynamic_object_cast<AtomsObject>(flowState.result());
	if(atoms == NULL)
		throw Exception(tr("The scene does not contain any atoms that could be exported."));

	// Show optional export settings dialog.
	if(!showSettingsDialog(atoms, NULL))
		return false;

	// Now write the output file.
	return exportParticles(dataset);
}

/******************************************************************************
* Returns the atoms that will be exported by this exporter at a given animation time.
******************************************************************************/
PipelineFlowState ParticleExporter::retrieveAtoms(DataSet* dataset, TimeTicks time)
{
	// Iterate over all scene nodes.
	for(SceneNodesIterator iter(dataset->sceneRoot()); !iter.finished(); iter.next()) {
		ObjectNode* node = dynamic_object_cast<ObjectNode>(iter.current());
		if(!node) continue;

		// Check if the node's object evaluates to an AtomsObject.
		const PipelineFlowState& state = node->evalPipeline(time);
		AtomsObject* atomsObj = dynamic_object_cast<AtomsObject>(state.result());
		if(atomsObj) return state;
	}
	return PipelineFlowState();
}


};
