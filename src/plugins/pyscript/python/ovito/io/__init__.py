""" This module contains functions and classes related to file input and output.

It primarily provides two high-level functions for reading and writing
external files:

    * :py:func:`import_file`
    * :py:func:`export_file`

In addition, it contains the :py:class:`FileSourceObject` class, which represents a mutable reference to an external
input file.
"""

import ovito
import ovito.scene

# Load the native module.
from PyScriptFileIO import *

def import_file(location, mode = "AddToScene", **params):
    """ This high-level function imports an external data file. 
    
        This Python function corresponds to the *Open Local File* command in OVITO's
        user interface. The format of the imported file is automatically detected.
        However, depending on the file's format, additional keyword parameters may need to be supplied to 
        the file parser to specify how the data should be interpreted. 
        These keyword parameters are documented below.
        
        :param str location: The file to import. This can be a local file path or a remote sftp:// URL.
        :param str mode: Determines how the imported data is inserted into the current scene. 
                           
                           * ``AddToScene`` (default): A new :py:class:`~ovito.scene.ObjectNode` is created and added to the scene.
                           * ``ReplaceSelected``: The source object of the currently selected node is updated to reference the new file. 
                             Existing modifiers are kept. 
                           * ``ResetScene``: All existing nodes are deleted from the scene before importing the data.
                       
        :returns: The :py:class:`~ovito.scene.ObjectNode` that has been created for the imported data.
                  
        ..  Note::
    
                The loading of external files typically happens asynchronously in OVITO. That is, 
                this function may return a node whose input data is still being loaded in the background.
                If necessary, you can call the node's :py:func:`~ovito.scene.ObjectNode.wait` function to 
                block script execution until the data has been fully loaded; for example, if you are going to
                directly access the imported data instead of relying on OVITO's modifier system.
        
        **File columns**
        
        When importing XYZ files or binary LAMMPS dump files, the mapping of file columns 
        to OVITO's particle properties must be specified using the ``columns`` keyword parameter::
        
            import_file("file.xyz", columns = 
              ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])
        
        The length of the list must match the number of columns in the input file. To ignore a column 
        during import, specify ``None`` instead of a property name at the corresponding position in the list.
        
        **Multi-timestep files**
        
        Some data formats can store multiple frames in a single file (e.g. XYZ and LAMMPS dump). OVITO cannot know 
        that such a file contains multiple frames (reading the entire file is avoided for performance reasons). 
        So explicitly tell OVITO to scan the entire file and load a sequence of frames by supplying the ``multiple_frames`` 
        option:: 
        
            node = import_file("file.dump", multiple_frames = True)
            print "Number of frames:", node.source.num_frames
            
    """
    
    if isinstance(mode, basestring):
        mode = ImportMode.__dict__[mode]

    # Determine the file's format.
    importer = ImportExportManager.instance.autodetectFileFormat(ovito.dataset, location)
    if not importer:
        raise RuntimeError("Could not detect the file format. The format might not be supported.")
    
    # Forward user parameters to the file importer object.
    for key in params:
        if not hasattr(importer, key):
            raise RuntimeError("Importer object %s has no attribute '%s'." % (importer, key))
        importer.__setattr__(key, params[key])

    # Import data.
    if not importer.importFile(location, mode):
        raise RuntimeError("Operation has been canceled by the user.")

    # Return the newly created ObjectNode.
    node = ovito.dataset.selected_node
    if not isinstance(node, ovito.scene.ObjectNode):
        raise RuntimeError("File import failed. Nothing was imported.")
    
    return node    

def _FileSourceObject_load(self, location, **params):
    """ Loads a different external data file and replaces the reference to the old file.
    
        The function auto-detects the format of the new file.
        
        The function accepts additional keyword arguments that are forwarded to the format-specific file importer.
        See the documentation of the :py:func:`ovito.io.import_file` function for more information.
        
        :param str location: The local file or remote sftp:// URL to load.
    """

    # Determine the file's format.
    importer = ImportExportManager.instance.autodetectFileFormat(self.dataset, location)
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
    if not self.setSource(location, importer, False):
        raise RuntimeError("Operation has been canceled by the user.")
FileSourceObject.load = _FileSourceObject_load

# Implement the 'sourceUrl' property of FileSourceObject, which returns or sets the currently loaded file path.
def _get_FileSourceObject_source_path(self, _originalGetterMethod = FileSourceObject.source_path):
    """ The path or URL of the loaded file. """    
    return _originalGetterMethod.__get__(self)
def _set_FileSourceObject_source_path(self, url):
    """ Sets the URL of the file referenced by this FileSourceObject. """
    self.setSource(url, None) 
FileSourceObject.source_path = property(_get_FileSourceObject_source_path, _set_FileSourceObject_source_path)

def export_file(node, file, format, **params):
    """ High-level function that exports data to a file.
    
        :param node: The node that provides the data to be exported.
        :type node: :py:class:`~ovito.scene.ObjectNode` 
        :param str file: The name of the output file.
        :param str format: The type of file to write:
        
                            * ``"lammps_dump"`` -- LAMMPS text-based dump format
                            * ``"lammps_data"`` -- LAMMPS data format
                            * ``"imd"`` -- IMD format
                            * ``"vasp"`` -- POSCAR format
                            * ``"xyz"`` -- XYZ format
        
        :returns: ``True`` on success; ``False`` if the operation was canceled by the user.
        
        The function evaluates the modification pipeline of the given object node and exports
        the results to one or more files. By default, only the current animation frame is exported.
        
        Depending on the selected export format, additional keyword parameters need to be specified.
       
        **File columns**
        
        When writing files in the ``"lammps_dump"`` or ``"xyz"`` format, you must specify the particle properties to be exported 
        using the ``columns`` keyword parameter::
        
            export_file(node, "output.xyz", "xyz", columns = 
              ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"]
            )
    """
    
    # Look up the exporter class for the selected format.
    if not format in export_file._formatTable:
        raise RuntimeError("Unknown output file format: %s" % format)
    
    # Create an instance of the exporter class.
    exporter = export_file._formatTable[format](params)
    
    # Ensure the data is available.
    node.wait()
    
    # Export data.
    return exporter.exportToFile([node], file, True)

# This is the table of export formats used by the export_file() function
# to look up the right exporter class for a file format.
# Plugins can register their exporter class by inserting a new entry in this dictionary.
export_file._formatTable = {}
    