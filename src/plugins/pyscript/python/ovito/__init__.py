import os.path

# Load the native module with the core bindings
from PyScript import *
from PyScriptContainers import *
from PyScriptApp import *

# Load sub-modules (in the right order because there are dependencies between them)
import ovito.linalg
import ovito.anim
import ovito.data
import ovito.vis
import ovito.io
import ovito.modifiers

from PyScriptScene import ObjectNode
from PyScriptScene import SceneRoot
from PyScriptScene import PipelineObject     
from PyScriptScene import PipelineStatus

# Load all OVITO modules packages. This is required
# to make all Boost.Python bindings available.
import pkgutil
import importlib
for _, _name, _ in pkgutil.walk_packages(__path__, __name__ + '.'):
	#print "Loading module", _name
	importlib.import_module(_name)

def _get_DataSet_selected_node(self):
    """ The :py:class:`~ovito.ObjectNode` that is currently selected in OVITO's graphical user interface, 
        or ``None`` if no node is selected. """
    return self.selection.front
def _set_DataSet_selected_node(self, node):
    """ Sets the scene node that is currently selected in OVITO. """
    if node: self.selection.setNode(node)
    else: self.selection.clear()
DataSet.selected_node = property(_get_DataSet_selected_node, _set_DataSet_selected_node)

# Implement the 'modifiers' property of the ObjectNode class, which provides access to modifiers in the pipeline. 
def _get_ObjectNode_modifiers(self):
    """The node's modification pipeline.
    
       This list contains the modifiers that are applied to the input data provided by the node's :py:attr:`.source` object. You
       can add and remove modifiers from this list as needed. The first modifier in the list is
       always evaluated first, and its output is passed on to the second modifier and so on. 
       The results of the last modifier are displayed in the viewports. 
       
       Example::
       
           node.modifiers.append(WrapPeriodicImagesModifier())
    """    
    
    class ObjectNodeModifierList:
        """ This helper class emulates a mutable sequence of modifiers. """
        def __init__(self, node): 
            self.node = node
        def _pipelineObjectList(self):
            """ This internal helper function builds a list of PipelineObjects in the node's pipeline. """
            polist = []
            obj = self.node.data_provider
            while isinstance(obj, PipelineObject):
                polist.insert(0, obj)
                obj = obj.source_object
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
            obj = self.node.data_provider
            while isinstance(obj, PipelineObject):
                count += len(obj.modifierApplications)
                obj = obj.source_object
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
        def __str__(self):
            return str(self._modifierList())
                    
    return ObjectNodeModifierList(self)
ObjectNode.modifiers = property(_get_ObjectNode_modifiers)

def _ObjectNode_wait(self, signalError = True, msgText = None):
    # Blocks script execution until the node's modification pipeline is ready.
    #
    #    :param str msgText: An optional text that will be shown to the user while waiting for the operation to finish.
    #    :param signalError: If ``True``, the function raises an exception when the modification pipeline could not be successfully evaluated.
    #                        This may be the case if the input file could not be loaded, or if one of the modifiers reported an error.   
    #    :returns: ``True`` if the pipeline evaluation is complete, ``False`` if the operation has been canceled by the user.
    #
    if not msgText: msgText = "Script is waiting for scene graph to become ready." 
    if not self.waitUntilReady(self.dataset.anim.time, msgText):
        return False
    if signalError:
        state = self.evalPipeline(self.dataset.anim.time)
        if state.status.type == PipelineStatus.Type.Error:
            raise RuntimeError("Pipeline error: %s" % state.status.text)
    return True
ObjectNode.wait = _ObjectNode_wait

def _ObjectNode_compute(self):
    """ Computes and returns the results of the node's modification pipeline.

        This method requests an update of the node's modification pipeline and waits until the effect of all modifiers in the 
        node's modification pipeline has been computed. If the modification pipeline is already up to date, i.e., results are already 
        available in the node's pipeline cache, the method returns immediately.
        
        Even if you are not interested in the final data that leaves the modification pipeline, you should call this method in case you are going to 
        directly access information provided by individual modifiers in the pipeline. This method will ensure that all modifiers 
        have been computed and their internal fields are up to date.

        This function raises a ``RuntimeError`` when the modification pipeline could not be successfully evaluated for some reason.
        This may happen due to invalid modifier parameters for example.

        :returns: A reference to the node's internal :py:class:`~ovito.data.DataCollection` containing the output of the modification pipeline.
                  It is also accessible via the :py:attr:`.output` attribute after calling :py:meth:`.compute`.
    """
    if not self.wait():
        raise RuntimeError("Operation has been canceled by the user.")
    self.__output = self.evalPipeline(self.dataset.anim.time)    
    assert(self.__output.status.type != PipelineStatus.Type.Error)
    assert(self.__output.status.type != PipelineStatus.Type.Pending)
        
    return self.__output    
ObjectNode.compute = _ObjectNode_compute

def _ObjectNode_output(self):
    """ Provides access to the last results of the node's modification pipeline.
        
        After calling the :py:meth:`.compute` method, this attribute holds a :py:class:`DataCollection`
        with the output of the node's modification pipeline.
    """    
    if not hasattr(self, "__output"):
        raise RuntimeError("The node's pipeline has not been evaluated yet. Call compute() first before accessing the pipeline output.")
    return self.__output
ObjectNode.output = property(_ObjectNode_output)

def _ObjectNode_remove_from_scene(self):
	""" Removes the node from the scene by deleting it from the :py:attr:`ovito.DataSet.scene_nodes` list.
	    The node's data will no longer be visible in the viewports after calling this method.
	"""
	del self.dataset.scene_nodes[self]
ObjectNode.remove_from_scene = _ObjectNode_remove_from_scene

# Give SceneRoot class a list-like interface.
SceneRoot.__len__ = lambda self: len(self.children)
SceneRoot.__iter__ = lambda self: self.children.__iter__
SceneRoot.__getitem__ = lambda self, i: self.children[i]
def _SceneRoot__setitem__(self, index, newNode):
    if index < 0: index += len(self)
    if index < 0 or index >= len(self):
        raise IndexError("List index is out of range.")
    self.removeChild(self.children[index])
    self.insertChild(index, newNode)
SceneRoot.__setitem__ = _SceneRoot__setitem__
def _SceneRoot__delitem__(self, index):
	if isinstance(index, ObjectNode):
		self.removeChild(index)
		return
	if index < 0 or index >= len(self):
		raise IndexError("List index is out of range.")
	self.removeChild(self.children[index])
SceneRoot.__delitem__ = _SceneRoot__delitem__
def _SceneRoot_append(self, node):
    if node.parentNode == self:
        raise RuntimeError("Cannot add the same node more than once to the scene.")
    self.addChild(node)
SceneRoot.append = _SceneRoot_append
def _SceneRoot_insert(self, index, node):
    if index < 0: index += len(self)
    if index < 0 or index >= len(self):
        raise IndexError("List index is out of range.")
    if node.parentNode == self:
        raise RuntimeError("Cannot insert the same node more than once into the scene.")
    self.insertChild(index, node)
SceneRoot.insert = _SceneRoot_insert

