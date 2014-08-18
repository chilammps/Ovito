# Load the native module.
from PyScriptScene import *

# Implement the 'modifiers' property of the ObjectNode class, which provides access to modifiers in the pipeline. 
def _get_ObjectNode_modifiers(self):
    """A list-like object that provides access to the node's modification pipeline.
    
       It contains the modifiers in the current pipeline. You
       can add and remove modifiers from this list as needed. The first modifier in the list is
       evaluated first, and its output is passed to the second modifier and so on. The output
       of the last modifier is displayed in the viewports. 
       
       Usage example::
       
           modifier = WrapPeriodicImagesModifier()
           dataset.selectedNode.modifiers.append(modifier)
    """    
    
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

def _ObjectNode_wait(self, msgText = None):
    """ Blocks script execution until the node's modification pipeline is ready.

        .. note::
        
            OVITO uses an asynchronous evaluation model to compute the results of the modification pipeline. 
            This allows the user to continue working with the program while it is performing long-running
            operations such as computing the results of modifiers or loading data files. The evaluation of modifiers
            in a pipeline is triggered each time the output of the pipeline is requested by someone. In an interactive
            program session of OVITO this happens very frequently, each time OVITO updates the viewport display.
            The object node caches the results of the last pipeline evaluation and automatically detects any changes made
            to a modification pipeline. Thus, subsequent evaluation requests can be served very quickly as long as 
            the modification pipeline or its input do not change.
            
        This method requests an update of the node's modification pipeline and waits until the input data (i.e. the source object) 
        has been fully loaded and the effect of all modifiers in the node's modification pipeline has been computed.
        If the modification pipeline is already up to date, the method returns immediately.
        
        You should call this method if you are going to access status information or other data from individual modifiers 
        in the node's modification pipeline. This method will ensure that the modifiers have been computed and their result data is
        up to date.
               
        :param msgText: An optional text that will be shown to the user while waiting for the operation to finish.
        :type msgText: str
        :returns: bool -- True if successful, False if the operation has been canceled by the user.
    """
    if not msgText: msgText = "Script is waiting for scene graph to become ready." 
    return self.waitUntilReady(self.dataset.animationSettings.time, msgText)
ObjectNode.wait = _ObjectNode_wait
