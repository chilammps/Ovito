# Load the native module.
from PyScriptViewport import *

import ovito.render

def _Viewport_perspective(self, cameraPos, cameraDir, fov):
    """ Sets up a viewport view with perspective projection. """
    self.viewType = ViewType.PERSPECTIVE
    self.cameraPosition = cameraPos
    self.cameraDirection = cameraDir
    self.fieldOfView = fov
Viewport.perspective = _Viewport_perspective

def _Viewport_render(self, renderSettings = None):
    """ Renders an image or movie of the the viewport's view. """
    if renderSettings == None:
        renderSettings = self.dataset.renderSettings
    elif isinstance(renderSettings, dict):
        renderSettings = ovito.render.RenderSettings(renderSettings)
    return self.dataset.renderScene(renderSettings, self)
Viewport.render = _Viewport_render


def _ViewportConfiguration__len__(self):
    return len(self.viewports)
ViewportConfiguration.__len__ = _ViewportConfiguration__len__

def _ViewportConfiguration__iter__(self):
    return self.viewports.__iter__()
ViewportConfiguration.__iter__ = _ViewportConfiguration__iter__

def _ViewportConfiguration__getitem__(self, index):
    return self.viewports[index]
ViewportConfiguration.__getitem__ = _ViewportConfiguration__getitem__
