import os.path

# Load the native module with the core bindings
from PyScript import *
from PyScriptContainers import *
from PyScriptApp import *

# Load sub-modules (in the right order because there are dependencies between them)
import ovito.linalg
import ovito.view
import ovito.anim
import ovito.scene
import ovito.render
import ovito.io
import ovito.modifiers

# Load all OVITO modules packages. This is required
# to make all Boost.Python bindings available.
import pkgutil
import importlib
for _, _name, _ in pkgutil.walk_packages(__path__, __name__ + '.'):
	#print "Loading module", _name
	importlib.import_module(_name)

def _get_DataSet_selected_node(self):
    """ The :py:class:`~ovito.scene.ObjectNode` that is currently selected in OVITO's graphical user interface, 
        or ``None`` if no node is selected. """
    return self.selection.firstNode
def _set_DataSet_selected_node(self, node):
    """ Sets the scene node that is currently selected in OVITO. """
    if node: self.selection.setNode(node)
    else: self.selection.clear()
DataSet.selected_node = property(_get_DataSet_selected_node, _set_DataSet_selected_node)
		