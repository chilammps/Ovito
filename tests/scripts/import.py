import ovito
import ovito.particles

# Check program version
print "This is OVITO version", ovito.version

# Load test data
node = ovito.load("../files/animation.dump.gz")

# Apply a modifier
mod = ovito.particles.CoordinationNumberModifier({'cutoff': 2.5})
assert mod.cutoff == 2.5
node.applyModifier(mod)
assert node.sourceObject != node.sceneObject

# Selection access
assert ovito.dataset.selectedNode != None
assert ovito.dataset.selectedNode == node

# Clear selection
ovito.dataset.selectedNode = None

# Load second file
node2 = ovito.load("../files/nw2.imd.gz")
print node2.sourceObject.sourceUrl
assert node2.sourceObject.sourceUrl == "../files/nw*.imd.gz"

# Load a third file.
node2.sourceObject.sourceUrl = "../files/SiVacancy.cfg"

# Test alternative way of loading a different file:
node2.sourceObject.load("../files/shear.void.dump.bin", { 'columnMapping': ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"]})
print node2.sourceObject.sourceUrl

# Delete nodes.
node.deleteNode()
node2.deleteNode()
assert ovito.dataset.selectedNode == None
