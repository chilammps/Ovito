# Import OVITO modules.
from ovito import *
from ovito.io import *
from ovito.modifiers import *
from ovito.view import *
from ovito.render import *

# Import modules from standard Python library.
import math

# Query program version.
print "This is Ovito version", version

# Import a data file.
node = import_file("../data/NanocrystallinePd.dump.gz")

# Block execution of the script until the node's modification pipeline is ready, that is, until 
# the input data has been completely loaded.
# This waiting step is optional, but it ensures that the modifier we are going to create 
# has access to the input data when it is inserted into the modification pipeline.
# This allows the Color Coding modifier to automatically adjust its interval to the min/max range of 
# the input data.
node.wait()

# Apply a modifier to the dataset.
node.modifiers.append(ColorCodingModifier(
	source = "Potential Energy",
	gradient = ColorCodingModifier.Hot()
))

# Set up view, looking along the [2,3,-3] direction from camera position (-100, -150, 150).
vp = Viewport()
vp.type = Viewport.Type.PERSPECTIVE
vp.camera_pos = (-100, -150, 150)
vp.camera_dir = (2, 3, -3)
vp.fov = math.radians(60.0)

# Render a picture of the dataset.
vp.render(RenderSettings(
	filename = "rendered_image.png",
	size = (120,120)
))

# Apply two more modifiers to delete some particles.
node.modifiers.append(SelectExpressionModifier(expression = "PotentialEnergy < -3.9"))
node.modifiers.append(DeleteParticlesModifier())

# Print the modification pipeline of the selected node to the console.
print "Modification pipeline:"
print node.modifiers
	
# Perform some analysis.
cna = CommonNeighborAnalysisModifier(cutoff = 3.2, adaptiveMode = False)
node.modifiers.append(cna)

# Wait until computation has been completed.
node.wait()

# Read out analysis results.
print "Number of FCC atoms:", cna.structureCounts[CommonNeighborAnalysisModifier.StructureTypes.FCC]

# Write processed atoms back to an output file.
export_file(node, "exported_data.dump", "lammps_dump", 	
	columns = ["Position.X", "Position.Y", "Position.Z", "Structure Type"]
)
