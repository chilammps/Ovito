from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/CFG/shear.void.120.cfg")

modifier = CreateBondsModifier()
node.modifiers.append(modifier)

print("Parameter defaults:")

print("  cutoff: {}".format(modifier.cutoff))
modifier.cutoff = 3.1

node.compute()

print("Output:")
print(node.output.bonds)
print(node.output.bonds.array)
print(len(node.output.bonds.array))

assert(len(node.output.bonds.array) == 21894)
