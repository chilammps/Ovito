"""The main module of OVITO's scripting interface, which contains a few high-level functions. 

Access to the data of OVITO is provided via the :py:data:`ovito.dataset` module-level
attribute. It points to the global :py:class:`~ovito.app.DataSet` object, which provides access to 
the contents of the scene, the animation settings, viewports, and render settings. 
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
		
def importData(location, importMode = "AddToScene", **params):
	""" Imports an external data file. 
	
		This script function corresponds to the File/Open Local File command in OVITO's
		user interface. The format of the imported file is automatically detected.
		
		Depending on the file format, additional parameters must be supplied to this function to specify 
		how the data in the file should be interpreted. For instance, to load an XYZ file, the mapping of file columns 
		to OVITO's particle properties must be specified using the ``columnMapping`` keyword parameter::
		
			importData("file.xyz", columnMapping = 
				["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])
				
		The ``importData()`` function passes additional keyword parameters on to the format-specific file importer.
			
		:param location: The file to import. This can be a local file path or a remote sftp:// URL.
		:type location: str
		:param importMode: Determines how the imported data is inserted into the current scene. 
		                   
		                   * ``AddToScene`` (default): A new :py:class:`~ovito.scene.ObjectNode` is added to the scene.
		                   * ``ReplaceSelected``: The source object of the currently selected node is updated to reference the new file. 
		                     Existing modifiers are kept. 
		                   * ``ResetScene``: All existing nodes are deleted from the scene before importing the data.
		                   
		:type importMode: str
		:returns: :py:class:`ovito.scene.ObjectNode` -- The scene node that has been created for the imported data.
	"""
	
	if isinstance(importMode, basestring):
		importMode = ovito.io.ImportMode.__dict__[importMode]

	# Determine the file's format.
	importer = ovito.io.ImportExportManager.instance.autodetectFileFormat(dataset, location)
	if not importer:
		raise RuntimeError("Could not detect the file format. The format might not be supported.")
	
	# Forward user parameters to the file importer object.
	for key in params:
		if not hasattr(importer, key):
			raise RuntimeError("Importer object %s has no attribute '%s'." % (importer, key))
		importer.__setattr__(key, params[key])

	# Import data.
	if not importer.importFile(location, importMode):
		raise RuntimeError("Operation has been canceled by the user.")

	# Return the newly created ObjectNode.
	node = dataset.selection.firstNode
	if not isinstance(node, ovito.scene.ObjectNode):
		raise RuntimeError("File import failed. Nothing was imported.")
	
	return node	

