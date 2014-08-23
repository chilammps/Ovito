
// Load test data.
node = load("../files/shear.void.120.cfg")

// Set transparency of particles to 50%.
node.applyModifier(new CreateExpressionPropertyModifier({ 
	outputProperty : "Transparency",
	expressions : ["0.5"] 
}))
