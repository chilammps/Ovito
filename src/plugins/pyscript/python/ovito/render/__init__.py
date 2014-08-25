# Load the native module.
from PyScriptRendering import *

def _get_RenderSettings_custom_range(self):
    """ 
    Specifies the range of animation frames to render if :py:attr:`.range` is ``CUSTOM_INTERVAL``.
    
    :Default: ``(0,100)`` 
    """
    return (self.customRangeStart, self.customRangeEnd)
def _set_RenderSettings_custom_range(self, range):
    if len(range) != 2: raise TypeError("Tuple or list of length two expected.")
    self.customRangeStart = range[0]
    self.customRangeEnd = range[1]
RenderSettings.custom_range = property(_get_RenderSettings_custom_range, _set_RenderSettings_custom_range)

def _get_RenderSettings_size(self):
    """ 
    Specifies the resolution of the generated image or movie in pixels. 
    
    :Default: ``(640,480)`` 
    """
    return (self.outputImageWidth, self.outputImageHeight)
def _set_RenderSettings_size(self, size):
    if len(size) != 2: raise TypeError("Tuple or list of length two expected.")
    self.outputImageWidth = size[0]
    self.outputImageHeight = size[1]
RenderSettings.size = property(_get_RenderSettings_size, _set_RenderSettings_size)

def _get_RenderSettings_filename(self):
    """ 
    A string specifying the file name under which the rendered image or movie will be saved.
    
    :Default: ``None``
    """
    if self.saveToFile and self.imageFilename: return self.imageFilename
    else: return None
def _set_RenderSettings_filename(self, filename):
    if filename:
        self.imageFilename = filename
        self.saveToFile = True
    else:
        self.saveToFile = False
RenderSettings.filename = property(_get_RenderSettings_filename, _set_RenderSettings_filename)