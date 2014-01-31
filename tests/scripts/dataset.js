// Load test data
load("../../examples/data/NanocrystallinePd.dump.gz")

// Selection access
node = ovito.selectedNode
print("Selected node:", node)

// Clear selection
ovito.selectedNode = null

// Change selection
ovito.selectedNode = node

// Program version
print("Program version:", ovito.version)