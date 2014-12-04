from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/shear.void.120.cfg")

node.modifiers.append(SliceModifier(
    distance = -12,
    inverse = True,
    slice_width = 18.0
))

node.modifiers.append(SliceModifier(
    distance = 12,
    inverse = True,
    slice_width = 18.0
))

modifier = ClusterAnalysisModifier()
node.modifiers.append(modifier)

print("Parameter defaults:")

print("  cutoff: {}".format(modifier.cutoff))
modifier.cutoff = 2.8

node.compute()

print("Output:")
print("Number of clusters: {}".format(modifier.count))
print(node.output.cluster)
print(node.output.cluster.array)

assert(modifier.count == 2)