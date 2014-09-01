from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/shear.void.120.cfg")

modifier = AffineTransformationModifier()
node.modifiers.append(modifier)

print modifier.relative_mode
modifier.relative_mode = True

print modifier.transform_particles
modifier.transform_particles = True

print modifier.only_selected
modifier.only_selected = False

print modifier.transform_box
modifier.transform_box = True

print modifier.transform_surface
modifier.transform_surface = False

print modifier.target_cell
modifier.target_cell = [[2,0,0,0],[0,1,0,0],[0,0,1,0]]
modifier.target_cell = np.array([[1,2,0,0],[0,1,0,0],[0,0,1,0.5]])
 
print modifier.transformation
modifier.transformation = [[1,0,0,0],[0,1,0,0.8],[0,0,1,0]]

node.compute()