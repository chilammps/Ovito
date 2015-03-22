"""
This module contains data container classes that are used by OVITO's modification pipeline system.

**Data collection:**

  * The :py:class:`DataCollection` class is container for multiple data objects and holds the results of a modification pipeline.

**Data objects:**

  * :py:class:`DataObject` (base class of all other data classes)
  * :py:class:`Bonds`
  * :py:class:`ParticleProperty`
  * :py:class:`ParticleTypeProperty`
  * :py:class:`SimulationCell`

**Helper classes:**

  * :py:class:`ParticleType`

"""

import collections

# Load the native module.
from PyScriptScene import DataCollection
from PyScriptScene import DataObject

# Give the DataCollection class a dict-like interface.
DataCollection.__len__ = lambda self: len(self.objects)
def _DataCollection__iter__(self):
    for o in self.objects:
        if hasattr(o, "_data_key"):
            yield o._data_key
        else:
            yield o.objectTitle
DataCollection.__iter__ = _DataCollection__iter__
def _DataCollection__getitem__(self, key):
    for o in self.objects:
        if hasattr(o, "_data_key"):
            if o._data_key == key:
                return o
        else:
            if o.objectTitle == key:
                return o
    raise KeyError("DataCollection does not contain object key '%s'." % key)
DataCollection.__getitem__ = _DataCollection__getitem__
def _DataCollection__getattr__(self, name):
    for o in self.objects:
        if hasattr(o, "_data_attribute_name"):
            if o._data_attribute_name == name:
                return o
    raise AttributeError("DataCollection does not have an attribute named '%s'." % name)
DataCollection.__getattr__ = _DataCollection__getattr__
def _DataCollection__str__(self):
    return "DataCollection(" + str(list(self.keys())) + ")"
DataCollection.__str__ = _DataCollection__str__
# Mix in base class collections.Mapping:
DataCollection.__bases__ = DataCollection.__bases__ + (collections.Mapping, )

# Implement 'display' attribute of DataObject class.
def _DataObject_display(self):
    """ The :py:class:`~ovito.vis.Display` object associated with this data object, which is responsible for
        displaying the data. If this field is ``None``, the data is of a non-visual type.
    """ 
    if not self.displayObjects:
        return None # This data object doesn't have a display object.
    return self.displayObjects[0]
DataObject.display = property(_DataObject_display)