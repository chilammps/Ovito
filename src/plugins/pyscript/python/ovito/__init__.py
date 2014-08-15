"""The main module of OVITO's scripting interface, which provides a few high-level functions. 

The data model of OVITO can be accessed via the :py:data:`ovito.dataset` global
attribute, which provides access to existing objects in the scene, animation
settings, viewports, and render settings. 
"""
import os.path

# Load the native module with the core bindings
from PyScript import *
from PyScriptContainers import *

# Load sub-modules (in the right order because there are dependencies between them)
import ovito.app
import ovito.linalg
import ovito.view
import ovito.anim
import ovito.scene
import ovito.render
import ovito.io

# Load bindings of plugins.
# Scan directory containing the main 'ovito' module for subpackages.
base_module_dir = os.path.dirname(__file__)
for item in os.listdir(base_module_dir):
	if os.path.isdir(os.path.join(base_module_dir, item)):
		__import__("ovito.%s" % item)
		
def importData(source_url, params = {}, importMode = ovito.io.ImportMode.AddToScene):
	""" Imports an external file into Ovito. """

	# Determine the file's format.
	importer = ovito.io.ImportExportManager.instance.autodetectFileFormat(dataset, source_url)
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
	if not isinstance(node, ovito.scene.ObjectNode):
		raise RuntimeError("File import failed. Nothing was imported.")
	
	return node	

