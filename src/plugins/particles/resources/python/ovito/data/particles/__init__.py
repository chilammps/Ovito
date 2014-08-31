import re
import numpy

# Load dependencies
import ovito.data

# Load the native code module
import Particles

# Inject selected classes into parent module.
ovito.data.SimulationCell = Particles.SimulationCell
ovito.data.ParticleProperty = Particles.ParticleProperty
ovito.data.BondsObject = Particles.BondsObject
ovito.data.SurfaceMesh = Particles.SurfaceMesh

# Register attribute keys by which data objects in a DataCollection can be accessed.
Particles.SimulationCell._data_attribute_name = "cell"
def _ParticleProperty_data_attribute_name(self):
    if self.type != Particles.ParticleProperty.Type.User:
        return re.sub('\W|^(?=\d)','_', self.name).lower()
    else:
        return None
Particles.ParticleProperty._data_attribute_name = property(_ParticleProperty_data_attribute_name)

# Returns a NumPy array wrapper for a particle property.
def _ParticleProperty_array(self):
    return numpy.asarray(self)
Particles.ParticleProperty.array = property(_ParticleProperty_array)

# Implement 'pbc' property of SimulationCell class.
def _SimulationCell_pbc(self):
    """ A 3-tuple with the periodic boundary flags. """
    return (self.pbc_x, self.pbc_y, self.pbc_z)
Particles.SimulationCell.pbc = property(_SimulationCell_pbc)