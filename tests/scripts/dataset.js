// Program version
print("Program version:", ovito.version)

// Load test data
load("../../examples/data/NanocrystallinePd.dump.gz")

// Selection access
node = ovito.selectedNode
print("Selected node:", node)
assert(node != null)

// Clear selection
ovito.selectedNode = null

// Change selection
ovito.selectedNode = node

