import ovito
import ovito.io
from ovito.modifiers import BondAngleAnalysisModifier

import numpy

node = ovito.io.import_file("../../files/fcc_coherent_twin.0.cfg")
modifier = BondAngleAnalysisModifier()
node.modifiers.append(modifier)
modifier.structures[BondAngleAnalysisModifier.Type.FCC].color = (1,0,0)

node.compute()
print "Computed structure types:"
print node.output.structure_type.array
#print "Assigned particle colors:"
#print node.output.color.array

print modifier.counts[BondAngleAnalysisModifier.Type.FCC]

assert(node.output.structure_type.array[0] == 1)
assert(node.output.structure_type.array[0] == BondAngleAnalysisModifier.Type.FCC)
assert((node.output.color.array[0] == (1,0,0)).all())
assert(BondAngleAnalysisModifier.Type.OTHER == 0)
assert(BondAngleAnalysisModifier.Type.FCC == 1)
assert(BondAngleAnalysisModifier.Type.HCP == 2)
assert(BondAngleAnalysisModifier.Type.BCC == 3)
assert(BondAngleAnalysisModifier.Type.ICO == 4)