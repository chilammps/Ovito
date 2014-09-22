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

# Set up view, looking along the [2,3,-3] direction from camera position (-100, -150, 150).
vp = Viewport()
vp.type = Viewport.Type.PERSPECTIVE
vp.camera_pos = (-100, -150, 150)
vp.camera_dir = (2, 3, -3)
vp.fov = math.radians(60.0)

settings = RenderSettings(
	filename = "rendered_image.png",
	size = (220,220)
)

# Render a picture of the dataset.
vp.render(settings)

# Apply two more modifiers to delete some particles.
node.modifiers.append(SelectExpressionModifier(expression = "PotentialEnergy < -3.9"))
node.modifiers.append(DeleteSelectedParticlesModifier())

# Print the modification pipeline of the selected node to the console.
print "Modification pipeline:"
print node.modifiers
	
# Perform some analysis.
cna = CommonNeighborAnalysisModifier(adaptive_mode = True)
node.modifiers.append(cna)

# Block execution of the script until the node's modification pipeline is ready, that is, until 
# the results of all modifiers have been computed.
node.compute()
for c in enumerate(cna.counts):
	print "Type %i: %i particles" % c
print node.output['Position'].array
for t in node.output.structure_type.type_list:
	print t.id, t.name, t.color

# Read out analysis results.
print "Number of FCC atoms:", cna.counts[CommonNeighborAnalysisModifier.Type.FCC]

# Write processed atoms back to an output file.
export_file(node, "exported_data.dump", "lammps_dump", 	
	columns = ["Position.X", "Position.Y", "Position.Z", "Structure Type"]
)
