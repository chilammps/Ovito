from ovito import *
from ovito.io import *
from ovito.modifiers import CommonNeighborAnalysisModifier

import numpy

node = import_file("../../files/fcc_coherent_twin.0.cfg")
modifier = CommonNeighborAnalysisModifier()

print("Parameter defaults:")
print("  start_value: {}".format(modifier.adaptive_mode))
print("  end_value: {}".format(modifier.cutoff))

node.modifiers.append(modifier)

modifier.structures[CommonNeighborAnalysisModifier.Type.FCC].color = (1,0,0)

node.compute()
print("Computed structure types:")
print(node.output.structure_type.array)
print("Number of particles: {}".format(node.output.structure_type.size))
print("Number of FCC atoms: {}".format(modifier.counts[CommonNeighborAnalysisModifier.Type.FCC]))
print("Number of HCP atoms: {}".format(modifier.counts[CommonNeighborAnalysisModifier.Type.HCP]))
print("Number of BCC atoms: {}".format(modifier.counts[CommonNeighborAnalysisModifier.Type.BCC]))

assert(modifier.counts[CommonNeighborAnalysisModifier.Type.FCC] == 128)
assert(node.output.structure_type.array[0] == 1)
assert(node.output.structure_type.array[0] == CommonNeighborAnalysisModifier.Type.FCC)
assert((node.output.color.array[0] == (1,0,0)).all())
assert(CommonNeighborAnalysisModifier.Type.OTHER == 0)
assert(CommonNeighborAnalysisModifier.Type.FCC == 1)
assert(CommonNeighborAnalysisModifier.Type.HCP == 2)
assert(CommonNeighborAnalysisModifier.Type.BCC == 3)
assert(CommonNeighborAnalysisModifier.Type.ICO == 4)
assert(CommonNeighborAnalysisModifier.Type.DIA == 5)