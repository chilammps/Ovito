from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/lammps_dumpi-42-1100-510000.cfg")

modifier = ConstructSurfaceModifier()
node.modifiers.append(modifier)

print("Parameter defaults:")

print("  only_selected: {}".format(modifier.only_selected))
modifier.only_selected = False

print("  radius: {}".format(modifier.radius))
modifier.radius = 3.8

print("  smoothing_level: {}".format(modifier.smoothing_level))
modifier.smoothing_level = 1

print("  cap_color: {}".format(modifier.mesh_display.cap_color))
print("  cap_transparency: {}".format(modifier.mesh_display.cap_transparency))
print("  show_cap: {}".format(modifier.mesh_display.show_cap))
print("  smooth_shading: {}".format(modifier.mesh_display.smooth_shading))
print("  surface_color: {}".format(modifier.mesh_display.surface_color))
print("  surface_transparency: {}".format(modifier.mesh_display.surface_transparency))

node.compute()

print("Output:")
print("  solid_volume= {}".format(modifier.solid_volume))
print("  surface_area= {}".format(modifier.surface_area))
print("  total_volume= {}".format(modifier.total_volume))
print(node.output)
