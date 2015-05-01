from ovito.io import import_file
from ovito.modifiers import ConstructSurfaceModifier

# Load a particle structure and construct its geometric surface:
node = import_file("simulation.dump")
mod = ConstructSurfaceModifier(radius = 2.9)
node.modifiers.append(mod)
node.compute()

# Query surface properties:
print("Surface area: %f" % mod.surface_area)
print("Solid volume: %f" % mod.solid_volume)
print("Solid volume fraction: %f" % (mod.solid_volume/mod.total_volume))

# Export the surface mesh to a VTK file.
mesh = node.output.surface
mesh.export_vtk('surface.vtk', node.output.cell)
