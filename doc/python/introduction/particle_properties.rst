===================================
Particle properties
===================================

Particle properties are array-like data objects in OVITO. A single particle property, for
instance the mass of particles, is stored in one instance of the :py:class:`~ovito.data.ParticleProperty`
class, which holds the mass values for all particles in the system. A particle 
system is therefore nothing else than a collection of :py:class:`~ovito.data.ParticleProperty` instances.
The only mandatory property, which is always present, is the ``Position`` property. Note that all 
:py:class:`ParticleProperties <ovito.data.ParticleProperty>` contain the same number of values, 
implicitly defining the number of particles in the system.

The properties forming a particle system are subsumed in a :py:class:`~ovito.data.DataCollection`,
which is a general container for data objects in OVITO. In addition to per-particle properties, a data collection
can contain other objects such as the :py:class:`~ovito.data.SimulationCell` or a :py:class:`~ovito.data.Bonds`
object. It is a :py:class:`~ovito.data.DataCollection` that is loaded from a simulation file and then flows down 
the modification pipeline. On the way, modifiers access existing data objects from the collection, modify them, or insert new
objects into the collection.

An :py:class:`~ovito.ObjectNode` keeps two :py:class:`DataCollections <ovito.data.DataCollection>`: one caching
the input data of the modification pipeline and one storing the output of the pipeline::

    >>> node.source.data
    DataCollection(['Simulation cell', 'Position'])
    
    >>> node.compute()
    >>> node.output
    DataCollection(['Simulation cell', 'Position', 'Color', 'Structure Type'])

Individual objects in a data collection can be accessed via name keys, e.g.::

    >>> node.output['Position']
    <ParticleProperty at 0x7ff46263cff0>
    
    >>> node.output['Position'].size       # This returns the number of particles
    117500
    
    >>> node.output['Position'].array      # This returns a NumPy array providing direct 
    ...                                    # access to the internal per-particle data
    [[ 73.24230194  -5.77583981  -0.87618297]
     [-49.00170135 -35.47610092 -27.92519951]
     [-50.36349869 -39.02569962 -25.61310005]
     ..., 
     [ 42.71210098  59.44919968  38.6432991 ]
     [ 42.9917984   63.53770065  36.33330154]
     [ 44.17670059  61.49860001  37.5401001 ]]
     
As a simplification, it is also possible to access standard particle properties and the simulation cell
as attributes of the :py:class:`~ovito.data.DataCollection` instead of using the name key::

    >>> node.output.position
    <ParticleProperty at 0x7ff46263cff0>
    
    >>> node.output.cell
    <SimulationCell at 0x7ffe88613a60>
    
To access standard particle properties in this way, the attribute name can be derived from the
particle property name by making all letters lower-case and replacing spaces with underscores (e.g. 
``output['Structure Type']`` becomes ``output.structure_type``). The names of all standard particle
properties are listed :ref:`here <standard-property-list>`.

.. note::

   The :py:attr:`~ovito.data.ParticleProperty.array` attribute of a particle property allows
   you to access the per-particle data as a NumPy array. The NumPy array is one-dimensional
   for scalar particle properties and two-dimensional for vectorial properties.
   In the current version of OVITO, the array is marked as read-only, and you cannot modify 
   the values stored in a particle property directly. That means you have to use OVITO's modifiers to manipulate
   the particle data, e.g. using the :py:class:`~ovito.modifiers.ComputePropertyModifier`.

-----------------------------------
Particle type property
-----------------------------------

Most particle properties are instances of the :py:class:`~ovito.data.ParticleProperty` class. However,
there exist specializations. For instance, the :py:class:`~ovito.data.ParticleTypeProperty` class is a subclass
of :py:class:`~ovito.data.ParticleProperty` and supplements the per-particle type info with a list of 
particle types, each having a name, a display color, and a display radius::

    >>> node = import_file('example.poscar')
    >>> ptp = node.source.data.particle_type   # Access the 'Particle Type' property
    >>> print(ptp)
    <ParticleTypeProperty at 0x7fe0a2c355d0>
    
    >>> print(ptp.array)     # This contains the per-particle data, one integer per particle
    [1 1 2 ..., 1 2 1]
    
    >>> for t in ptp.type_list:
    ...     print(t.id, t.name, t.color)
    1 Cu (1.0 0.4 0.4)
    2 Zr (0.0 1.0 0.4)

The :py:attr:`~ovito.data.ParticleTypeProperty.type_list` attribute contains a list of
:py:class:`~ovito.data.ParticleType`\ s, one for each atom type defined in the simulation.
In the example above we were looping over this list to print the ID, name, and color
of each defined atom type.
