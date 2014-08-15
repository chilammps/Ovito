# Load the native module.
from PyScriptFileIO import *

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

# Implement the 'sourceUrl' property of LinkedFileObject, which returns or sets the currently loaded file path.
def _get_LinkedFileObject_sourceUrl(self, _originalGetterMethod = LinkedFileObject.sourceUrl):
    """ Returns the URL of the file referenced by this LinkedFileObject. """    
    return _originalGetterMethod.__get__(self)
def _set_LinkedFileObject_sourceUrl(self, url):
    """ Sets the URL of the file referenced by this LinkedFileObject. """
    self.setSource(url, None) 
LinkedFileObject.sourceUrl = property(_get_LinkedFileObject_sourceUrl, _set_LinkedFileObject_sourceUrl)

def FileExporter_exportToFile(self, filename, nodeList = None, noninteractive = True, _originalMethod = FileExporter.exportToFile):
    # If caller doesn't specify a list of nodes to export, simply export all nodes in the scene.
    if nodeList == None:
        nodeList = self.dataset.sceneRoot.children
    return _originalMethod(self, nodeList, filename, noninteractive)
FileExporter.exportToFile = FileExporter_exportToFile

