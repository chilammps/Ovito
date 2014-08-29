import re
import numpy

# Load dependencies first.
import ovito

# Load the native code module
from Particles import *

# Register attribute keys by which data objects in a DataCollection can be accessed.
SimulationCell._data_attribute_name = "cell"
def _ParticleProperty_data_attribute_name(self):
    if self.type != ParticlePropertyType.UserProperty:
        return re.sub('\W|^(?=\d)','_', self.name).lower()
    else:
        return None
ParticleProperty._data_attribute_name = property(_ParticleProperty_data_attribute_name)

# Returns a NumPy array wrapper for a particle property
def _ParticleProperty_array(self):
    return numpy.asarray(self)
ParticleProperty.array = property(_ParticleProperty_array)