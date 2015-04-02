import re
import numpy
import math

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
    assert(len(flags) == 3) # Expected tuple with three Boolean flags.
    self.pbc_x = flags[0]
    self.pbc_y = flags[1]
    self.pbc_z = flags[2]
Particles.SimulationCell.pbc = property(_get_SimulationCell_pbc, _set_SimulationCell_pbc)

class CutoffNeighborFinder(Particles.CutoffNeighborFinder):
    """ 
    A utility class that computes particle neighbor lists.
    
    This class allows a Python script to iterate over the neighbors of each particles within a given cutoff distance.
    You can use it to build neighbors lists or perform other kinds of analyses that require neighbor information.
    
    The constructor takes a positive cutoff radius and a :py:class:`DataCollection <ovito.data.DataCollection>` 
    containing the input particle positions and the cell geometry (including periodic boundary flags).
    
    Once the utility class has been constructed, you can call :py:meth:`.find` to iterate over the neighbors of a selected particle,    
    for example:
    
    .. literalinclude:: ../example_snippets/cutoff_neighbor_finder.py
    """
        
    def __init__(self, cutoff, data_collection):
        """ This is the constructor. """
        super(self.__class__, self).__init__()        
        if not hasattr(data_collection, 'position'):
            raise KeyError("Data collection does not contain particle positions.")
        if not hasattr(data_collection, 'cell'):
            raise KeyError("Data collection does not contain simulation cell information.")
        self.particle_count = data_collection.position.size
        self.prepare(cutoff, data_collection.position, data_collection.cell)
        
    def find(self, index):
        """ 
        Returns an iterator over all neighbors of the given particle.
         
        :param int index: The index of the central particle whose neighbors should be iterated. Particle indices start at 0.
        :returns: A Python iterator that visits all neighbors of the central particle within the cutoff distance. 
                  For each neighbor the iterator returns a tuple containing four entries:
                  
                      1. The index of current neighbor particle (starting at 0).
                      2. The distance of the current neighbor from the central particle.
                      3. The three-dimensional vector connecting the central particle with the current neighbor (taking into account periodic images).
                      4. The periodic shift vector, which specifies how often the vector has crossed each periodic boundary of the simulation cell.
        
        Note that all periodic images of particles are visited. Thus, the same particle index (1st item above) may appear multiple times in the neighbor
        list of one central particle. In fact, the central particle may be among its own neighbors in a sufficiently small periodic simulation cell.
        However, the computed vector (3rd item above) will be unique for each visited image of a neighbor particle.
        """
        if index < 0 or index >= self.particle_count:
            raise IndexError("Particle index is out of range.")
        # Construct the C++ neighbor query. 
        query = Particles.CutoffNeighborFinder.Query(self, index)
        # Iterate over neighbors.
        while not query.atEnd:
            yield (query.current, math.sqrt(query.distanceSquared), tuple(query.delta), tuple(query.pbcShift))
            query.next()
            
ovito.data.CutoffNeighborFinder = CutoffNeighborFinder