import os.path

# Load the native module with the core bindings
from PyScript import *

# Add __str__ method to TimeInterval class:
TimeInterval.__str__ = lambda self: str((self.start,self.end))

# Load bindings of plugins.
# Scan directory containing the main 'ovito' module for subpackages.
base_module_dir = os.path.dirname(__file__)
for item in os.listdir(base_module_dir):
	if os.path.isdir(os.path.join(base_module_dir, item)):
		__import__("ovito.%s" % item)
		
def wait(msgText = "Script is waiting for scene graph to become ready."):
	""" Blocks script execution until all data have been loaded and all modifiers in the scene have been computed. """
	return dataset.waitUntilSceneIsReady(msgText)
	
def load(source_url, params = {}, importMode = ImportMode.AddToScene):
	""" Imports an external file into Ovito. """

	# Determine the file's format.
	importer = ImportExportManager.instance.autodetectFileFormat(dataset, source_url)
	if not importer:
		raise RuntimeError("Could not detect the file format. The format might not be supported.")
		
	# Forward user parameters to the file importer object.
	for key in params:
		if not hasattr(importer, key):
			raise RuntimeError("Importer object %s does not have an attribute '%s'." % (importer, key))
		importer.__setattr__(key, params[key])

	# Import data.
	if not importer.importFile(source_url, importMode):
		raise RuntimeError("Operation has been canceled by the user.")

	# Return the newly created ObjectNode.
	node = dataset.selection.firstNode
	if not isinstance(node, ObjectNode):
		raise RuntimeError("File import failed. Nothing was imported.")
	
	return node	

def _LinkedFileObject_load(self, source_url, params = {}):
	"""Loads a new file into the LinkedFileObject."""

	# Determine the file's format.
	importer = ImportExportManager.instance.autodetectFileFormat(self.dataset, source_url)
	if not importer:
		raise RuntimeError("Could not detect the file format. The format might not be supported.")
	
	# Re-use existing importer if compatible.
	if self.importer != None and type(self.importer) == type(importer):
		importer = self.importer
		
	# Forward user parameters to the importer.
	for key in params:
		if not hasattr(importer, key):
			raise RuntimeError("Importer object %s does not have an attribute '%s'." % (importer, key))
		importer.__setattr__(key, params[key])

	# Load new data file.
	if not self.setSource(source_url, importer, False):
		raise RuntimeError("Operation has been canceled by the user.")

LinkedFileObject.load = _LinkedFileObject_load

# Implement the 'selectedNode' property, which returns or sets the currently selected scene node in a dataset.
def _get_DataSet_selectedNode(self):
	""" Returns the scene node that is currently selected in OVITO. """	
	return self.selection.firstNode

def _set_DataSet_selectedNode(self, node):
	""" Sets the scene node that is currently selected in OVITO. """
	if node: self.selection.setNode(node)
	else: self.selection.clear()

DataSet.selectedNode = property(_get_DataSet_selectedNode, _set_DataSet_selectedNode)

# Implement the 'sourceUrl' property of LinkedFileObject, which returns or sets the currently loaded file path.
def _get_LinkedFileObject_sourceUrl(self, oldGetterMethod = LinkedFileObject.sourceUrl):
	""" Returns the URL of the file referenced by this LinkedFileObject. """	
	return oldGetterMethod.__get__(self)

def _set_LinkedFileObject_sourceUrl(self, url):
	""" Sets the URL of the file referenced by this LinkedFileObject. """
	self.setSource(url, None) 

LinkedFileObject.sourceUrl = property(_get_LinkedFileObject_sourceUrl, _set_LinkedFileObject_sourceUrl)

def _Viewport_perspective(self, cameraPos, cameraDir, fov):
	""" Sets up a viewport view with perspective projection. """
	self.viewType = ViewType.PERSPECTIVE
	self.cameraPosition = cameraPos
	self.cameraDirection = cameraDir
	self.fieldOfView = fov

Viewport.perspective = _Viewport_perspective