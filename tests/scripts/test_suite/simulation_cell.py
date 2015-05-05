from ovito import *
from ovito.io import *

node = import_file("../../files/LAMMPS/animation.dump.gz")
cell = node.source.data.cell

print("  input pbc flags: {}".format(cell.pbc))
cell.pbc = (False, True, True)

print("  input cell: {}".format(cell.matrix))
cell.matrix = [[10,0,0,0],[0,2,0,0],[0,0,1,0]]

node.compute()

print("Output:")
print(node.output.cell.pbc)
print(node.output.cell.matrix)
