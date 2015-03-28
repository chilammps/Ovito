from ovito.io import import_file
from ovito.data import CutoffNeighborFinder

# Load input simulation file.
node = import_file("simulation.dump")
particle_data = node.source.data
num_particles = particle_data.position.size

# Initialize neighbor finder object:
cutoff = 3.5
finder = CutoffNeighborFinder(cutoff, particle_data)

# Iterate over all input particles:
for index in range(num_particles):
    print("Neighbors of particle %i:" % index)
    # Iterate over the neighbors of the current particle:
    for neigh in finder.find(index):
        print(neigh)