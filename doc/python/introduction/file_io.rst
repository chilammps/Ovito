===================================
File I/O
===================================

This page describes how to load data from external files and how to export data again.

------------------------------------
File input
------------------------------------

There are currently two ways of loading an external data file. The primary one is
calling the global :py:func:`~ovito.io.import_file` function::

   >>> from ovito.io import *
   >>> node = import_file("simulation.dump")
   
This high-level function takes the file to import as a first parameter and returns a newly created
:py:class:`~ovito.ObjectNode`, whose :py:class:`~ovito.io.FileSource` is set up to point
to the specified file. Furthermore, the :py:func:`~ovito.io.import_file` function inserts the new object node
into the scene by adding it to the :py:attr:`~ovito.DataSet.scene_nodes` list of the current
dataset. This will let the imported data appear in OVITO's viewports.
You can remove the object node from scene again by calling ::

   >>> node.remove_from_scene()
   
Then the imported data will not be visible anymore in OVITO's interactive viewports or in rendered images.
However, you can still use the node for computations in the Python script.

The second option to load an external data file is to use the :py:meth:`~ovito.io.FileSource.load` method
of the :py:class:`~ovito.io.FileSource` of an existing object node::

   >>> node.source.load("otherdata.dump")

The existing object node and its modification pipeline are preserved; only the input is 
replaced with a different file. Use this option if you already have a modification pipeline set up and 
want to re-use it with other input data. The :py:meth:`~ovito.io.FileSource.load` method is also used to
load a reference configuration for modifiers like the :py:class:`~ovito.modifiers.CalculateDisplacementsModifier`::

   >>> modifier = CalculateDisplacementsModifier()
   >>> modifier.reference.load("reference.dump")
   
Here :py:attr:`CalculateDisplacementsModifier.reference <ovito.modifiers.CalculateDisplacementsModifier.reference>` is 
another :py:class:`~ovito.io.FileSource`, which is responsible for loading the reference particle positions
required by the modifier.

**Column mapping**

Both the global :py:func:`~ovito.io.import_file` function and the :py:meth:`FileSource.load() <ovito.io.FileSource.load>` method
accept format-specific keyword arguments in addition to the filename. For instance, when loading standard XYZ
files, the mapping of file columns to particle properties needs to be specified using the ``columns`` keyword::

   >>> node = import_file("simulation.xyz", columns = 
   ...           ["Particle Type", "Position.X", "Position.Y", "Position.Z", "My Property"])
   
The number of strings in the ``columns`` list must match the number of data columns in the file. 
Individual columns can be ignored during file parsing by specifying ``None`` instead of a string.
Each string contains the target property name and, for vector properties, the component separated by a dot.
OVITO defines a set of standard property names, which are listed in the :ref:`standard-property-list` section.
Specifying a non-standard name will result in the creation of a user-defined particle property. 

**Simulation sequences**

To load a single LAMMPS file or XYZ file containing multiple simulation frames, use the ``multiple_frames`` keyword::

   >>> node = import_file("sequence.dump", multiple_frames = True)

To load a sequence of simulation files, following a naming pattern like :file:`frame.0.dump`, :file:`frame.1000.dump`,
:file:`frame.2000.dump`, etc., specify only one filename from the sequence::

   >>> node = import_file("frame.0.dump")

OVITO will automatically detect the other files in the directory belonging to the same simulation sequence.
You can check how many animation frames were found by querying the current :py:class:`~ovito.anim.AnimationSettings`::

   >>> print ovito.dataset.anim.last_frame
   100

In this example 101 simulation frames were found by OVITO (frame counting starts at 0). 

.. note::
   
   To save memory and time, OVITO does not load all simulation frames at once. It only scans the directory (or the multiframe file) to discover all frames belonging to a 
   sequence and adjusts the internal animation length to match the number of input frames found. 
   The actual data will only be loaded by the :py:class:`~ovito.io.FileSource` on demand, e.g., when 
   jumping to a specific frame in the animation or when rendering a movie.
   
------------------------------------
File output
------------------------------------

You can write particles to a file using the :py:func:`ovito.io.export_file` function::

    >>> export_file(node, "outputfile.dump", "lammps_dump",
    ...    columns = ["Position.X", "Position.Y", "Position.Z", "My Property"])

OVITO will automatically evaluate the node's modification pipeline and export the computed results to the file.
If the node's modification pipeline contains no modifiers, then the original, unmodified data
will be exported. 

The second function parameter specifies the the output filename, and the third parameter selects the 
output format. For a list of supported file formats, see :py:func:`~ovito.io.export_file`.
Depending on the output format, additional keyword arguments must be specified. For instance,
in the example above the ``columns`` parameter was used to list the particle properties to be exported.
 
.. _standard-property-list:

------------------------------------
Standard particle properties
------------------------------------

OVITO defines the following standard particle properties. Any name not included in this list will 
be treated as a user-defined particle property (of data type float).

===================================== ====================================== ==============
Property name                         Components                             Datatype  
===================================== ====================================== ==============
``"Angular Momentum"``                X, Y, Z                                float
``"Angular Velocity"``                X, Y, Z                                float
``"Centrosymmetry"``                                                         float
``"Charge"``                                                                 float
``"Cluster"``                                                                integer
``"Color"``                           R, G, B                                float
``"Coordination"``                                                           integer
``"Deformation Gradient"``            11, 12, 13, 21, 22, 23, 32, 32, 33     float
``"Dipole Magnitude"``                                                       float
``"Dipole Orientation"``              X, Y, Z                                float
``"Displacement"``                    X, Y, Z                                float
``"Displacement Magnitude"``                                                 float
``"Force"``                           X, Y, Z                                float
``"Kinetic Energy"``                                                         float
``"Mass"``                                                                   float
``"Molecule Identifier"``                                                    integer
``"Particle Type"``                                                          integer (str)
``"Position"``                        X, Y, Z                                float
``"Orientation"``                     X, Y, Z, W                             float
``"Particle Identifier"``                                                    integer
``"Periodic Image"``                  X, Y, Z                                integer
``"Potential Energy"``                                                       float
``"Radius"``                                                                 float
``"Selection"``                                                              integer
``"Spin"``                                                                   float
``"Structure Type"``                                                         integer
``"Strain Tensor"``                   XX, YY, ZZ, XY, XZ, YZ                 float
``"Stress Tensor"``                   XX, YY, ZZ, XY, XZ, YZ                 float
``"Torque"``                          X, Y, Z                                float
``"Total Energy"``                                                           float
``"Transparency"``                                                           float
``"Velocity"``                        X, Y, Z                                float
``"Velocity Magnitude"``                                                     float
===================================== ====================================== ==============


