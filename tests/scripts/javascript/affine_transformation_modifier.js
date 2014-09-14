
// Load test data.
node = load("../files/SiVacancy.cfg")

// Set up shear transformation matrix. Start by creating an identity transformation matrix:
m = new AffineTransformation()
// Set XZ shear component to 0.1
m.setElement(0, 2, 0.1)

// Create and apply affine transformation modifier.
ovito.selectedNode.applyModifier(new AffineTransformationModifier({ 
	applyToSimulationBox : true,
	transformation : m
}))
