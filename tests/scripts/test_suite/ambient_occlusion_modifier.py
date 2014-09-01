from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/shear.void.120.cfg")

modifier = AmbientOcclusionModifier()
node.modifiers.append(modifier)

print modifier.buffer_resolution
modifier.buffer_resolution = 4

print modifier.intensity
modifier.intensity = 0.9

print modifier.sample_count
modifier.sample_count = 30

#node.compute()