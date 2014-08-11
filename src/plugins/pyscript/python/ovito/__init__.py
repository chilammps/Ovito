import os.path

# Load the native module with the core bindings
from PyScript import *

# Add __str__ method to TimeInterval class:
def _TimeInterval_str(self): return str((self.start,self.end))
TimeInterval.__str__ = _TimeInterval_str

# Load bindings of plugins.
# Scan directory containing the main 'ovito' module for subpackages.
base_module_dir = os.path.dirname(__file__)
for item in os.listdir(base_module_dir):
	if os.path.isdir(os.path.join(base_module_dir, item)):
		__import__("ovito.%s" % item)
	
# The load() function imports an external file into Ovito.
def load(source, params = {}):
	importURL = FileManager.instance.urlFromUserInput(source)
	
	# Determine the file's format.
	importer = ImportExportManager.instance.autodetectFileFormat(dataset, importURL)
	if not importer:
		raise RuntimeError("Could not detect the file format. The format might not be supported.")
		
	# Forward user to the file importer object.
	for key in params:
		if not hasattr(importer, key):
			raise RuntimeError("Importer object %s does not have an attribute '%s'." % (importer, key))
		importer.__setattr__(key, params[key])

	# Import data.
	if not importer.importFile(importURL, ImportMode.AddToScene):
		raise RuntimeError("Operation has been canceled by the user.")

	# Return the newly created ObjectNode.
	node = dataset.selection.firstNode
	if not isinstance(node, ObjectNode):
		raise RuntimeError("File import failed. Nothing was imported.")
	
	return node	


def get_selectedNode(self):
	"""Returns the scene node that is currently selected in OVITO."""	
	return self.selection.firstNode

def set_selectedNode(self, node):
	"""Sets the scene node that is currently selected in OVITO."""
	if node: self.selection.setNode(node)
	else: self.selection.clear()

DataSet.selectedNode = property(get_selectedNode, set_selectedNode)
