from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/lammps_dumpi-42-1100-510000.cfg")

modifier = ConstructSurfaceModifier()
node.modifiers.append(modifier)

print "Parameter defaults:"

print "  only_selected:", modifier.only_selected
modifier.only_selected = False

print "  radius:", modifier.radius
modifier.radius = 3.8

print "  smoothing_level:", modifier.smoothing_level
modifier.smoothing_level = 1

print "  cap_color:", modifier.mesh_display.cap_color
print "  cap_transparency:", modifier.mesh_display.cap_transparency
print "  show_cap:", modifier.mesh_display.show_cap
print "  smooth_shading:", modifier.mesh_display.smooth_shading
print "  surface_color:", modifier.mesh_display.surface_color
print "  surface_transparency:", modifier.mesh_display.surface_transparency

node.compute()

print "Output:"
print "  solid_volume=", modifier.solid_volume
print "  surface_area=", modifier.surface_area
print "  total_volume=", modifier.total_volume
