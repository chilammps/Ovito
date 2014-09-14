from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/sheared_aSi.nc")

modifier = WignerSeitzAnalysisModifier()
node.modifiers.append(modifier)
modifier.reference.load("../../files/sheared_aSi.nc")

dataset.anim.current_frame = 4

print "Parameter defaults:"

print "  eliminate_cell_deformation:", modifier.eliminate_cell_deformation
modifier.eliminate_cell_deformation = True

print "  frame_offset:", modifier.frame_offset
modifier.frame_offset = 0

print "  reference_frame:", modifier.reference_frame
modifier.reference_frame = 0

print "  use_frame_offset:", modifier.use_frame_offset
modifier.use_frame_offset = False

node.compute()

print "Output:"
print "  vacancy_count=", modifier.vacancy_count
print "  interstitial_count=", modifier.interstitial_count
print node.output["Occupancy"].array

assert(modifier.vacancy_count == 970)
