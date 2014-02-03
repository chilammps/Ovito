
// Check program version
print("Program version:", ovito.version)

// Check current directory
print("Current directory:", pwd())

// Load test data
obj = load("../files/animation.dump.gz")

// Selection access
assert(ovito.selectedNode != null)
assert(ovito.selectedNode == obj)
node = ovito.selectedNode

// Clear selection
ovito.selectedNode = null

// Change selection
ovito.selectedNode = node

// Load second file
cd("../files")
obj2 = load("nw2.imd.gz")
assert(ovito.selectedNode == obj2)

print(obj2.source)
obj2.source.sourceUrl = "C60impact.nc"

// Delete nodes.
obj.deleteNode()
//obj2.deleteNode()
//assert(ovito.selectedNode == null)
