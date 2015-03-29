===================================
File I/O
===================================

This section describes how to load simulation data from external files and how to export modified  
particle data again using a script.

------------------------------------
File input
------------------------------------

The primary way of loading an external data file is the :py:func:`~ovito.io.import_file` function::

   >>> from ovito.io import import_file
   >>> node = import_file("simulation.dump")

This high-level function works like the `Open Local File` function in OVITO's graphical user interface. 
It creates and returns an :py:class:`~ovito.ObjectNode`, whose :py:class:`~ovito.io.FileSource` is set up to point
to the specified file, and which is reponsible for loading the actual data from the file. 
Furthermore, the :py:func:`~ovito.io.import_file` function inserts the returned object node
into the three-dimensional scene by adding it to the :py:attr:`~ovito.DataSet.scene_nodes` list of the current
dataset. This will let the imported data appear in OVITO's viewports.
You may remove it from scene again by calling ::

   >>> node.remove_from_scene()

Then the imported data will no longer be visible in OVITO's interactive viewports or in rendered images.
However, you can still continue to use the :py:class:`~ovito.ObjectNode` for computations in the Python script.

In case you already have an existing object node, for example after being done with the first simulation
file imported above using :py:func:`~ovito.io.import_file`, you can load a new simulation file and replace the old
input data. This is done using the :py:meth:`~ovito.io.FileSource.load` method
of the :py:class:`~ovito.io.FileSource` attached to the node::

   >>> node.source.load("other_simulation.dump")

It takes the same parameters as the :py:func:`~ovito.io.import_file` function, but it doesn't create a new
object node. The existing object node and its modification pipeline are preserved; only its input data is 
replaced with a different input file. Calling :py:func:`~ovito.io.import_file` multiple times, in contrast, would
add additional object nodes to the scene, and you might quickly run out of memory. Thus, if you want to process
a set of simulation files in a directory, for example, you should use the following kind of loop::

   node = None
   for filename in files:
       if node:
           node.source.load(filename)
       else:
           node = import_file(filename)
           # ... set up computation, add modifiers, etc. 
       node.compute()
       # ... access computation results

Note that the :py:meth:`~ovito.io.FileSource.load` method is also used to
load reference configurations for modifiers that require initial coordinates of particles, e.g.::

   >>> modifier = CalculateDisplacementsModifier()
   >>> modifier.reference.load("reference.dump")

Here the :py:attr:`~ovito.modifiers.CalculateDisplacementsModifier.reference` attribute refers 
to a second :py:class:`~ovito.io.FileSource`, which is owned by the :py:class:`~ovito.modifiers.CalculateDisplacementsModifier` and which is responsible
for loading the reference particle positions needed by the modifier.

**Column mapping**

Both the global :py:func:`~ovito.io.import_file` function and the :py:meth:`FileSource.load() <ovito.io.FileSource.load>` method
accept format-specific keyword arguments in addition to the filename. For instance, when loading XYZ
files, the mapping of input file columns to OVITO's particle properties needs to be specified using the ``columns`` keyword::

   >>> node = import_file("simulation.xyz", columns = 
   ...           ["Particle Type", "Position.X", "Position.Y", "Position.Z", "My Property"])
   
The number of entries in the ``columns`` list must match the number of data columns in the input file. 
Each entry specifies the destination property and includes, for vector properties, the component.
File columns can be skipped during parsing by specifying ``None`` instead of a property name.
OVITO defines a set of standard property names, which are listed in the :ref:`standard-property-list` section.
Specifying a non-standard name is also possible, which creates a user-defined particle property
filled with the data from the corresponding file column.

**Simulation sequences**

So far we have only considered loading one simulation snapshot at a time by explicit calls to :py:func:`~ovito.io.import_file`
or :py:meth:`~ovito.io.FileSource.load`. As you know from the graphical program version, OVITO is also able to
load a sequence of simulation snapshots (a trajectory), which can be played back as an animation. 
There are two possible cases:

1. To load a file that stores multiple simulation frames, use the ``multiple_frames`` keyword::

    >>> node = import_file("sequence.dump", multiple_frames = True)

   OVITO will scan the entire file and discover all contained simulation frames.

2. To load a series of simulation files from a directory, following a naming pattern like :file:`frame.0.dump`, :file:`frame.1000.dump`,
   :file:`frame.2000.dump`, etc., pass only the first filename from the sequence to the :py:func:`~ovito.io.import_file` function::

    >>> node = import_file("frame.0.dump")

   OVITO will automatically detect the other files in the directory belonging to the same simulation trajectory.

In both cases you can check how many animation frames were found by querying the :py:class:`~ovito.anim.AnimationSettings`::

   >>> ovito.dataset.anim.last_frame
   100

In this example, 101 simulation frames were found by OVITO (frame counting starts at 0).

.. note::
   
   To save memory and time, OVITO never loads all frames from a trjectory at once. It only scans the directory (or the multiframe file) 
   to discover all frames belonging to the sequence and adjusts the internal animation length to match the number of input frames found. 
   The actual simulation data will only be loaded by the :py:class:`~ovito.io.FileSource` on demand, e.g., when 
   jumping to a specific frame in the animation or when rendering a movie.
   
You can iterate over the frames of a loaded animation sequence in a script loop::

   # Load a sequence of simulation files 'frame.0.dump', 'frame.1000.dump', etc.
   node = import_file("frame.0.dump")
   # ... Apply modifiers to the node here.
   
   # Now iterate over the frames:
   for f in range(ovito.dataset.anim.last_frame+1):
       # Set the time slider position:
       ovito.dataset.anim.current_frame = f
       # This will load the input data for the current frame and evaluate the modifiers:
       node.compute()
       # ... access computation results for current animation frame.
       
------------------------------------
File output
------------------------------------

You can write particles to a file using the :py:func:`ovito.io.export_file` function::

    >>> export_file(node, "outputfile.dump", "lammps_dump",
    ...    columns = ["Position.X", "Position.Y", "Position.Z", "My Property"])

OVITO will automatically evaluate the node's modification pipeline and export the computed results to the file.
If the node's modification pipeline contains no modifiers, then the original, unmodified data
will be exported. 

The second function parameter specifies the output filename, and the third parameter selects the 
output format. For a list of supported file formats, see :py:func:`~ovito.io.export_file` documentation.
Depending on the selected output format, additional keyword arguments must be specified. For instance,
in the example above the ``columns`` parameter lists the particle properties to be exported.
 
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
``"Aspherical Shape"``                X, Y, Z                                float
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


