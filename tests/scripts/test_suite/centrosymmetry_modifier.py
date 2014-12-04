from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/shear.void.120.cfg")

modifier = CentroSymmetryModifier()
node.modifiers.append(modifier)

print("Parameter defaults:")

print("  num_neighbors: {}".format(modifier.num_neighbors))
modifier.num_neighbors = 12

node.compute()

print("Output:")
print(node.output.centrosymmetry)
print(node.output.centrosymmetry.array)
