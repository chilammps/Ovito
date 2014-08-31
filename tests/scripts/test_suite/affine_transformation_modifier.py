import ovito
import ovito.io
import ovito.modifiers

import numpy

node = ovito.io.import_file("../../files/shear.void.120.cfg")
modifier = ovito.modifiers.AffineTransformationModifier()
modifier.relative_mode = True
modifier.transform_particles = True
modifier.only_selected = False
modifier.transform_box = True
modifier.transform_surface = False

print modifier.transformation