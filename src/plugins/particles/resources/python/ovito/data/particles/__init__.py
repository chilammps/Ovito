import re
import numpy

# Load dependencies
import ovito.data

# Load the native code module
import Particles

# Inject selected classes into parent module.
ovito.data.SimulationCell = Particles.SimulationCell
ovito.data.ParticleProperty = Particles.ParticleProperty
ovito.data.Bonds = Particles.Bonds
ovito.data.SurfaceMesh = Particles.SurfaceMesh
ovito.data.ParticleTypeProperty = Particles.ParticleTypeProperty
ovito.data.ParticleType = Particles.ParticleType
ovito.data.CutoffNeighborFinder = Particles.CutoffNeighborFinder

# Register attribute keys by which data objects in a DataCollection can be accessed.
Particles.SimulationCell._data_attribute_name = "cell"
Particles.Bonds._data_attribute_name = "bonds"

def _ParticleProperty_data_attribute_name(self):
    if self.type != Particles.ParticleProperty.Type.User:
        return re.sub('\W|^(?=\d)','_', self.name).lower()
    else:
        return None
Particles.ParticleProperty._data_attribute_name = property(_ParticleProperty_data_attribute_name)
def _ParticleProperty_data_key(self):
    return self.name
Particles.ParticleProperty._data_key = property(_ParticleProperty_data_key)

# Returns a NumPy array wrapper for a particle property.
def _ParticleProperty_array(self):
    """ 
    This attribute returns a NumPy array providing direct access to the per-particle data.
        
    The returned array is one-dimensional for scalar particle properties (:py:attr:`.components` == 1),
    or two-dimensional for vector properties (:py:attr:`.components` > 1). The outer length of the array is 
    equal to the number of particles in both cases.
        
    Note that the returned NumPy array is read-only and provides a view of the internal data. 
    No copy of the data is made.
        
..  note::
         
        Write access to particle properties from a script is not available yet in the current version of the scripting interface. 
        This is feature is planned for one of the next program releases.
        
    """
    return numpy.asarray(self)
Particles.ParticleProperty.array = property(_ParticleProperty_array)

# Returns a NumPy array wrapper for bonds list.
def _Bonds_array(self):
    """ This attribute returns a NumPy array providing direct access to the bond list.
        
        The returned array is two-dimensional and contains pairs of particle indices connect by bonds.
        The array's shape is *N x 2*, where *N* is the number of bonds.
        
        Note that the returned NumPy array is read-only and provides a view of the internal data. 
        No copy of the data is made.  
    """
    return numpy.asarray(self)
Particles.Bonds.array = property(_Bonds_array)

# Implement 'pbc' property of SimulationCell class.
def _get_SimulationCell_pbc(self):
    """ A tuple of length 3 with the periodic boundary flags (bool). """
    return (self.pbc_x, self.pbc_y, self.pbc_z)
def _set_SimulationCell_pbc(self, flags):
    assert(len(flags) == 3) # Expect tuple of three Boolean flags
    self.pbc_x = flags[0]
    self.pbc_y = flags[1]
    self.pbc_z = flags[2]
Particles.SimulationCell.pbc = property(_get_SimulationCell_pbc, _set_SimulationCell_pbc)

def _CutoffNeighborFinder_find(self, particle_index):
    """ """
    if particle_index < 0 or particle_index >= self.particle_count:
        raise IndexError("Particle index is out of range.")
    query = Particles.CutoffNeighborFinder.Query(self, particle_index)
Particles.CutoffNeighborFinder.find = _CutoffNeighborFinder_find
