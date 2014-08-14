from ovito import *
from ovito.linalg import *
from ovito.particles import *
import math

# Query program version.
print "This is Ovito version", version

# Import a data file.
node = load("../data/NanocrystallinePd.dump.gz")

# Block execution of the script until the scene is ready, that is, until 
# the input file has been completely loaded.
# This waiting step is optional, but it ensures that the modifier we are going to create 
# has access to the input data when it is inserted into the modification pipeline.
# This allows the Color Coding modifier to automatically adjust its interval to the range of 
# values in the input data.
wait()

# Apply a modifier to the dataset.
node.modifiers.append(ColorCodingModifier({ 
	"sourceProperty" : "Potential Energy",
	"colorGradient"  : ColorCodingHotGradient()
}))

# Set up view, looking along the [2,3,-3] vector from camera position (-100, -150, 150).
vp = dataset.viewportConfig.activeViewport
vp.perspective((-100, -150, 150), (2, 3, -3), math.radians(60.0))

# Render a picture of the dataset.
vp.render({
	"filename"    : "image.png",
	"imageWidth"  : 120,
	"imageHeight" : 120
})

# Apply two more modifiers to delete some particles.
node.modifiers.append(SelectExpressionModifier({ "expression" : "PotentialEnergy < -3.9" }))
node.modifiers.append(DeleteParticlesModifier())

# Print the modification pipeline of the selected node to the console.
print "Modification pipeline:"
for mod in node.modifiers:
	print "  ", mod
	
# Perform some analysis.
cna = CommonNeighborAnalysisModifier({ "cutoff" : 3.2, "adaptiveMode" : False })
node.modifiers.append(cna)

# Wait until computation has been completed.
wait()

# Read out analysis results.
print "Number of FCC atoms:", cna.structureCounts[CommonNeighborAnalysisModifier.StructureTypes.FCC]

# Write processed atoms back to an output file.
LAMMPSDumpExporter({ 
	"columnMapping": ["Position.X", "Position.Y", "Position.Z", "Structure Type"] 
}).exportToFile("exporteddata.dump")
