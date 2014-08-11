import ovito
import ovito.particles

# Check program version
print "This is OVITO version", ovito.version

# Load test data
obj = ovito.load("../files/animation.dump.gz")
print obj

# Apply a modifier
mod = ovito.particles.CoordinationNumberModifier({'cutoff': 2.36})
obj.applyModifier(mod)

# Selection access
print ovito.dataset.selection
print ovito.dataset.selectedNode
assert ovito.dataset.selectedNode != None
assert ovito.dataset.selectedNode == obj

# Clear selection
ovito.dataset.selectedNode = None
