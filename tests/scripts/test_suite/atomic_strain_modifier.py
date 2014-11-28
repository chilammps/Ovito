from ovito import *
from ovito.io import *
from ovito.modifiers import *
import numpy as np

node = import_file("../../files/shear.void.120.cfg")

# Apply smoe strain to the atoms.
node.modifiers.append(AffineTransformationModifier(
    transformation = [[1,0.1,0,0],[0,1,0,0.8],[0,0,1,0]],
    transform_box = True
))

# Calculate the atomic strain.
modifier = AtomicStrainModifier()
node.modifiers.append(modifier)
modifier.reference.load("../../files/shear.void.120.cfg")

print("Parameter defaults:")

print("  assume_unwrapped_coordinates: {}".format(modifier.assume_unwrapped_coordinates))
modifier.assume_unwrapped_coordinates = False

print("  cutoff: {}".format(modifier.cutoff))
modifier.cutoff = 2.8

print("  eliminate_cell_deformation: {}".format(modifier.eliminate_cell_deformation))
modifier.eliminate_cell_deformation = False

print("  frame_offset: {}".format(modifier.frame_offset))
modifier.frame_offset = 0

print("  output_deformation_gradients: {}".format(modifier.output_deformation_gradients))
modifier.output_deformation_gradients = True

print("  output_nonaffine_squared_displacements: {}".format(modifier.output_nonaffine_squared_displacements))
modifier.output_nonaffine_squared_displacements = True

print("  output_strain_tensors: {}".format(modifier.output_strain_tensors))
modifier.output_strain_tensors = True

print("  reference_frame: {}".format(modifier.reference_frame))
modifier.reference_frame = 0

print("  select_invalid_particles: {}".format(modifier.select_invalid_particles))
modifier.select_invalid_particles = True

print("  use_frame_offset: {}".format(modifier.use_frame_offset))
modifier.use_frame_offset = True

node.compute()

print("Output:")
print("  invalid_particle_count={}".format(modifier.invalid_particle_count))
print(node.output["Shear Strain"].array)
print(node.output.deformation_gradient.array)
print(node.output.strain_tensor.array)
print(node.output.selection.array)

assert(abs(node.output["Shear Strain"].array[0] - 0.05008306) < 1e-4)
