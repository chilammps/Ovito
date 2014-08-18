# Load the native module.
from PyScriptApp import *

# Implement the 'selectedNode' property, which returns or sets the currently selected scene node in a dataset.
def _get_DataSet_selectedNode(self):
    """ The currently selected :py:class:`~ovito.scene.ObjectNode`, or ``None`` if no node is selected. """
    return self.selection.firstNode
def _set_DataSet_selectedNode(self, node):
    """ Sets the scene node that is currently selected in OVITO. """
    if node: self.selection.setNode(node)
    else: self.selection.clear()
DataSet.selectedNode = property(_get_DataSet_selectedNode, _set_DataSet_selectedNode)
