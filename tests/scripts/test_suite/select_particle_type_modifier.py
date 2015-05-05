import ovito
import ovito.io
import ovito.modifiers

import numpy

node = ovito.io.import_file("../../files/CFG/shear.void.120.cfg")
node.modifiers.append(ovito.modifiers.CommonNeighborAnalysisModifier(adaptive_mode = True))
modifier = ovito.modifiers.SelectParticleTypeModifier()
print(modifier.types)
print(modifier.property)
modifier.types = {1,2}
print(modifier.types)
node.modifiers.append(modifier)
node.compute()
print(len(node.output.selection.array))
print(numpy.count_nonzero(node.output.selection))
assert(numpy.count_nonzero(node.output.selection) == 1444)

modifier.property = "Structure Type"
modifier.types = { ovito.modifiers.CommonNeighborAnalysisModifier.Type.FCC, 
                   ovito.modifiers.CommonNeighborAnalysisModifier.Type.HCP }
node.compute()
print(numpy.count_nonzero(node.output.selection))
assert(numpy.count_nonzero(node.output.selection) == 1199)
