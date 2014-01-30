// Query program version.
print("This is Ovito " + version())

// Set the working directory and import a data file.
cd("/Users/stuko/Documents/temp/")
node = load("test.dump")

// Block execution of the script until the scene is ready, that is, until 
// the input file has been completely loaded.
// This is an optional step, but it ensures that the modifier we are going to create 
// has access to the input data at the time it is inserted into the modification pipeline.
// Then the Color Coding modifier will automatically adjust its interval to the range of 
// values present in the input data.
wait()

// Apply a modifier to the dataset.
node.applyModifier(new ColorCodingModifier({ 
	sourceProperty : "potentialenergy",
	colorGradient  : new ColorCodingHotGradient()
}))

// Apply two more modifiers to delete some particles.
node.applyModifier(new SelectExpressionModifier({ expression : "potentialenergy < -3.9" }))
node.applyModifier(new DeleteParticlesModifier())

// Set up view.
activeViewport.perspective(Point3(-100, -50, 50), Vector(1, 0.5, -0.5), 70.0 * Math.PI/180.0)
activeViewport.zoomToSceneExtents()

// Render a picture of the dataset.
activeViewport.render({
	filename    : "test_abc.png",
	imageWidth  : 100,
	imageHeight : 100
})

// Print the modification pipeline of the selected node to the console.
print("Current modification pipeline:")
for(var i = 0; i < selectedNode.modifiers.length; i++)
	print("  " + selectedNode.modifiers[i])  
	
// Perform some analysis.
cna = new CommonNeighborAnalysisModifier({ cutoff : 3.2, adaptiveMode : false })
node.applyModifier(cna)

// Wait until computation has been completed.
wait()

// Read out analysis results.
print("Number of FCC atoms: " + cna.structureCounts[CommonNeighborAnalysisModifier.FCC])

// Writes processed atoms back to an output file.
save("exporteddata.dump", LAMMPSDumpExporter, 
	{ columnMapping: ["Position.X", "Position.Y", "Position.Z", "Structure Type"] })
