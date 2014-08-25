import ovito
import ovito.io
import ovito.modifiers

# Check program version
print "This is OVITO version", ovito.version

# Load test data
node = ovito.io.import_file("../files/animation.dump.gz")

# Apply a modifier
mod = ovito.modifiers.CoordinationNumberModifier(cutoff = 2.5)
assert mod.cutoff == 2.5
node.modifiers.append(mod)

# Selection access
assert ovito.dataset.selected_node != None
assert ovito.dataset.selected_node == node

# Clear selection
ovito.dataset.selected_node = None

# Load second file
node2 = ovito.io.import_file("../files/nw2.imd.gz")
print node2.source.source_path
assert node2.source.source_path == "../files/nw*.imd.gz"

# Load a third file.
node2.source.source_path = "../files/SiVacancy.cfg"

# Test alternative way of loading a different file:
node2.source.load("../files/shear.void.dump.bin", columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])
print node2.source.source_path

# Delete nodes.
node.deleteNode()
node2.deleteNode()
assert ovito.dataset.selectedNode == None
