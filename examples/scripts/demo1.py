# Import OVITO modules.
from ovito import *
from ovito.io import *
from ovito.modifiers import *
from ovito.vis import *
from ovito.data import *

# Import modules from standard Python library.
import math
import numpy as np

# Query program version.
print "This is Ovito version", version

# Import a data file.
node = import_file("../data/NanocrystallinePd.dump.gz")

# Apply a modifier to the dataset.
node.modifiers.append(ColorCodingModifier(
	property = ParticleProperty.Type.PotentialEnergy,
	gradient = ColorCodingModifier.Hot()
))

# Set visual appearance.
#node.source.data.position.display.enabled = False
#print node.source.data.position
#print node.source.data.position.display

# Set up view, looking along the [2,3,-3] direction from camera position (-100, -150, 150).
vp = Viewport()
vp.type = Viewport.Type.PERSPECTIVE
vp.camera_pos = (-100, -150, 150)
vp.camera_dir = (2, 3, -3)
vp.fov = math.radians(60.0)

node.modifiers.append(AmbientOcclusionModifier())

settings = RenderSettings(
	filename = "rendered_image.png",
	size = (220,220)
)
print settings.renderer

# Render a picture of the dataset.
vp.render(settings)

# Apply two more modifiers to delete some particles.
node.modifiers.append(SelectExpressionModifier(expression = "PotentialEnergy < -3.9"))
node.modifiers.append(DeleteSelectedParticlesModifier())

# Print the modification pipeline of the selected node to the console.
print "Modification pipeline:"
print node.modifiers
	
# Perform some analysis.
cna = CommonNeighborAnalysisModifier(cutoff = 3.2, adaptive_mode = False)
node.modifiers.append(cna)

# Block execution of the script until the node's modification pipeline is ready, that is, until 
# the results of all modifiers have been computed.
#node.compute()

state = node.compute()
print node.output
print state.keys()
print state.position.array
print state.position
print state.cell
print state.cell.display.enabled
print state.cell.matrix
print state.cell.pbc

# Read out analysis results.
print "Number of FCC atoms:", cna.counts[CommonNeighborAnalysisModifier.Type.FCC]

# Write processed atoms back to an output file.
export_file(node, "exported_data.dump", "lammps_dump", 	
	columns = ["Position.X", "Position.Y", "Position.Z", "Structure Type"]
)

print len(dataset.scene_nodes)
node.remove_from_scene()
print len(dataset.scene_nodes)
