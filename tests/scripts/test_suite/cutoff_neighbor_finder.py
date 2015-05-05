from ovito.io import *
from ovito.data import *

node = import_file("../../files/LAMMPS/animation.dump.gz")

num_particles = node.source.data.position.size

cutoff = 2.5
finder = CutoffNeighborFinder(cutoff, node.source.data)

for index in range(num_particles):
    for n in finder.find(index):
        assert(n[0] >= 0 and n[0] < num_particles)
        assert(n[1] <= cutoff)
