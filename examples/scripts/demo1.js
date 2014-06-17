// Query program version.
print("This is Ovito " + ovito.version)

// Import a data file.
node = load("../data/NanocrystallinePd.dump.gz")

// Block execution of the script until the scene is ready, that is, until 
// the input file has been completely loaded.
// This is an optional step, but it ensures that the modifier we are going to create 
// has access to the input data at the time it is inserted into the modification pipeline.
// This allows the Color Coding modifier to automatically adjust its interval to the range of 
// values present in the input data.
wait()

// Apply a modifier to the dataset.
node.applyModifier(new ColorCodingModifier({ 
	sourceProperty : "Potential Energy",
	colorGradient  : new ColorCodingHotGradient()
}))

// Set up view, looking along the [2,3,-3] vector.
activeViewport.perspective(Point(-100, -150, 150), Vector(2, 3, -3), 60.0 * Math.PI/180.0)

// Render a picture of the dataset.
activeViewport.render({
	filename    : "rendering.png",
	imageWidth  : 120,
	imageHeight : 120
})

// Apply two more modifiers to delete some particles.
node.applyModifier(new SelectExpressionModifier({ expression : "PotentialEnergy < -3.9" }))
node.applyModifier(new DeleteParticlesModifier())

// Print the modification pipeline of the selected node to the console.
print("Current modification pipeline:")
for(var i = 0; i < node.modifiers.length; i++)
	print("  " + node.modifiers[i])  
	
// Perform some analysis.
cna = new CommonNeighborAnalysisModifier({ cutoff : 3.2, adaptiveMode : false })
node.applyModifier(cna)

// Wait until computation has been completed.
wait()

// Read out analysis results.
print("Number of FCC atoms: " + cna.structureCounts[CommonNeighborAnalysisModifier.FCC])

// Write processed atoms back to an output file.
save("exporteddata.dump", LAMMPSDumpExporter, 
	{ columnMapping: ["Position.X", "Position.Y", "Position.Z", "Structure Type"] })
