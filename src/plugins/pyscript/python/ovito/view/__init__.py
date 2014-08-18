# Load the native module.
from PyScriptViewport import *

import ovito.render

def _Viewport_render(self, renderSettings = None):
    """ Renders an image or movie of the the viewport's view.
    
        :param renderSettings: A custom render settings object. If ``None``, the global render settings are used (see :py:attr:`ovito.app.DataSet.renderSettings` attribute).
        :type renderSettings: :py:class:`~ovito.render.RenderSettings`
        :returns: ``True`` on success; ``False`` if operation has been canceled by the user.
    """
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
