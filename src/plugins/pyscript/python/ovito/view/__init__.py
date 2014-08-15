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
    """ Renders an image of the the viewport's view. """
    if renderSettings == None:
        renderSettings = self.dataset.renderSettings
    elif isinstance(renderSettings, dict):
        renderSettings = ovito.render.RenderSettings(renderSettings)
    return self.dataset.renderScene(renderSettings, self)
Viewport.render = _Viewport_render
