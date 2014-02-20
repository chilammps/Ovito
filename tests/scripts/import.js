
// Check program version
print("Program version:", ovito.version)
assert(ovito.version[0] == "v")

// Check current directory
print("Current directory:", pwd())

// Load test data
obj = load("../files/animation.dump.gz")

// Selection access
assert(ovito.selectedNode != null)
node = ovito.selectedNode

// Clear selection
ovito.selectedNode = null

// Change selection
ovito.selectedNode = node

// Load second file
cd("../files")
obj2 = load("nw2.imd.gz")
assert(obj2.source.sourceUrl == "file:nw*.imd.gz")

// Load third file.
obj2.source.sourceUrl = "SiVacancy.cfg"

// Delete nodes.
obj.deleteNode()
obj2.deleteNode()
assert(ovito.selectedNode == null)
