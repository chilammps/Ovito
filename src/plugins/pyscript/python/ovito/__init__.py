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
def _get_LinkedFileObject_sourceUrl(self, _originalGetterMethod = LinkedFileObject.sourceUrl):
	""" Returns the URL of the file referenced by this LinkedFileObject. """	
	return _originalGetterMethod.__get__(self)
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

def _Viewport_render(self, renderSettings = None):
	""" Renders an image of the the viewport's view. """
	if renderSettings == None:
		renderSettings = dataset.renderSettings
	elif isinstance(renderSettings, dict):
		renderSettings = RenderSettings(renderSettings)
	return dataset.renderScene(renderSettings, self)
Viewport.render = _Viewport_render

# Implement the 'modifiers' property of the ObjectNode class, which provides access to modifiers in the pipeline. 
def _get_ObjectNode_modifiers(self):
	""" Returns a sequence object that provides access to the modifiers in the node's modification pipeline. """	
	class ObjectNodeModifierList:
		""" This helper class emulates a mutable sequence of modifiers. """
		def __init__(self, node): 
			self.node = node
		def _pipelineObjectList(self):
			""" This internal helper function builds a list of PipelineObjects in the node's pipeline. """
			polist = []
			obj = self.node.sceneObject
			while isinstance(obj, PipelineObject):
				polist.insert(0, obj)
				obj = obj.inputObject
			return polist
		def _modifierList(self):
			""" This internal helper function builds a list containing all modifiers in the node's pipeline. """
			mods = []
			for obj in self._pipelineObjectList():
				for app in obj.modifierApplications:
					mods.append(app.modifier)
			return mods
		def __len__(self):
			""" Returns the total number of modifiers in the node's pipeline. """
			count = 0
			obj = self.node.sceneObject
			while isinstance(obj, PipelineObject):
				count += len(obj.modifierApplications)
				obj = obj.inputObject
			return count
		def __iter__(self):
			return self._modifierList().__iter__()
		def __getitem__(self, i):
			return self._modifierList()[i]
		def __setitem__(self, index, newMod):
			""" Replaces an existing modifier in the pipeline with a different one. """
			if index < 0: index += len(self)
			count = 0
			for obj in self._pipelineObjectList():
				for app in enumerate(obj.modifierApplications):
					if count == index:
						obj.removeModifier(app[1])
						obj.insertModifier(newMod, app[0])
						return
					count += 1
			raise IndexError("List index is out of range.")
		def __delitem__(self, index):
			if index < 0: index += len(self)
			count = 0
			for obj in self._pipelineObjectList():
				for app in obj.modifierApplications:
					if count == index:
						obj.removeModifier(app)
						return
					count += 1
			raise IndexError("List index is out of range.")
		def append(self, mod):
			self.node.applyModifier(mod)
		def insert(self, index, mod):
			if index < 0: index += len(self)
			count = 0
			for obj in self._pipelineObjectList():
				for i in range(len(obj.modifierApplications)):
					if count == index:
						obj.insertModifier(mod, i)
						return
					count += 1
			self.node.applyModifier(mod)
					
	return ObjectNodeModifierList(self)
ObjectNode.modifiers = property(_get_ObjectNode_modifiers)

def FileExporter_exportToFile(self, filename, nodeList = None, noninteractive = True, _originalMethod = FileExporter.exportToFile):
	""" Exports data from OVITO to a file. """	
	# If caller doesn't specify a list of nodes to export, simply export all nodes in the scene.
	if nodeList == None:
		nodeList = self.dataset.sceneRoot.children
	return _originalMethod(self, nodeList, filename, noninteractive)
FileExporter.exportToFile = FileExporter_exportToFile

