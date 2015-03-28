from ovito.io import import_file
from ovito.modifiers import ConstructSurfaceModifier

# Load a particle structure and construct its surface mesh:
node = import_file("simulation.dump")
node.modifiers.append(ConstructSurfaceModifier(radius = 2.8))
node.compute()

# Access the computed surface mesh and export to a VTK file for 
# visualization with ParaView.
mesh = node.output['Surface mesh']
mesh.export_vtk('surface.vtk', node.output.cell)
mesh.export_cap_vtk('surface_cap.vtk', node.output.cell)