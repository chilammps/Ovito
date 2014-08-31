import ovito
import ovito.io
import ovito.modifiers

import numpy

node = ovito.io.import_file("../../files/animation.dump.gz")
node.modifiers.append(ovito.modifiers.AssignColorModifier(color = (0,1,0)))
assert((node.compute().color.array[0] == numpy.array([0,1,0])).all())
