import collections

# Load the native module.
from PyScriptScene import *

# Implement the 'modifiers' property of the ObjectNode class, which provides access to modifiers in the pipeline. 
def _get_ObjectNode_modifiers(self):
    """The node's modification pipeline.
    
       This list contains the modifiers that are applied to the node's :py:attr:`.source` object. You
       can add and remove modifiers from this list as needed. The first modifier in the list is
       always evaluated first, and its output is passed on to the second modifier and so on. 
       The results of the last modifier are displayed in the viewports. 
       
       Usage example::
       
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
        if state.status.type == PipelineStatusType.Error:
            raise RuntimeError(state.status.text)
    return True
ObjectNode.wait = _ObjectNode_wait

def _ObjectNode_compute(self):
    """ Computes and returns the results of the node's modification pipeline.

        This method requests an update of the node's modification pipeline and waits until the effect of all modifiers in the 
        node's modification pipeline has been computed. If the modification pipeline is already up to date, that is, if the 
        results are already available in the node's pipeline cache, the method returns immediately.
        
        The method returns a new :py:class:`~ovito.scene.PipelineFlowState` object, which holds the results of the modification pipeline.
        
        Even if you are not interested in the final data that leaves the modification pipeline, you should call this method in case you are going to 
        directly access information provided by individual modifiers in the pipeline. This method will ensure that all modifiers 
        have been computed and their internal fields are up to date.

        This function raises a ``RuntimeError`` when the modification pipeline could not be successfully evaluated for some reason.
        This may happen due to invalid modifier parameters for example.

        :returns: A :py:class:`~ovito.scene.PipelineFlowState` object holding the output of the modification pipeline.
    """
    if not self.wait():
        raise RuntimeError("Operation has been canceled by the user.")
    state = self.evalPipeline(self.dataset.anim.time)
    
    # Raise Python error if the pipeline could not be successfully evaluated.
    if state.status.type == PipelineStatusType.Error:
        raise RuntimeError(state.status.text)
    
    return state    
    
ObjectNode.compute = _ObjectNode_compute

# Give PipelineFlowState class a dict-like interface.
PipelineFlowState.__len__ = lambda self: len(self.objects)
def _PipelineFlowState__iter__(self):
    for o in self.objects:
        if hasattr(o, "data_key"):
            yield o.data_key
        else:
            yield o.objectTitle
PipelineFlowState.__iter__ = _PipelineFlowState__iter__
def _PipelineFlowState__getitem__(self, key):
    for o in self.objects:
        if hasattr(o, "data_key"):
            if o.data_key == key:
                return o
        elif o.objectTitle == key:
            return o
    raise KeyError("Data object '%s' does not exist." % key)
PipelineFlowState.__getitem__ = _PipelineFlowState__getitem__
def _PipelineFlowState__getattr__(self, name):
    for o in self.objects:
        if hasattr(o, "data_key"):
            if o.data_key == name:
                return o
        elif o.objectTitle == name:
            return o
    raise AttributeError("PipelineFlowState object does not have an attribute '%s'." % name)
PipelineFlowState.__getattr__ = _PipelineFlowState__getattr__
# Mix in base class collections.Mapping:
PipelineFlowState.__bases__ = PipelineFlowState.__bases__ + (collections.Mapping, )


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
