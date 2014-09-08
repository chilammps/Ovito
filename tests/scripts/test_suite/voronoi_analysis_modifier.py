from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/sheared_aSi.nc")

modifier = VoronoiAnalysisModifier()
node.modifiers.append(modifier)

print "Parameter defaults:"

print "  compute_indices:", modifier.compute_indices
modifier.compute_indices = True

print "  cutoff:", modifier.cutoff
modifier.cutoff = 8.0

print "  edge_count:", modifier.edge_count
modifier.edge_count = 5

print "  edge_threshold:", modifier.edge_threshold
modifier.edge_threshold = 0.1

print "  face_threshold:", modifier.face_threshold
modifier.face_threshold = 0.04

print "  only_selected:", modifier.only_selected
modifier.only_selected = False

print "  use_cutoff:", modifier.use_cutoff
modifier.use_cutoff = True

print "  use_radii:", modifier.use_radii
modifier.use_radii = True

node.compute()

print "Output:"
print node.output["Atomic Volume"].array
print node.output["Voronoi Index"].array
print node.output["Coordination"].array
print node.output.coordination.array
