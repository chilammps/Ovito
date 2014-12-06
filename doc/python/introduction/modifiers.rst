===================================
Modifiers
===================================

Modifiers are objects that can be inserted into an object node's modification pipeline.
They modify, filter, or extend the data that flows down the pipeline from the 
:py:class:`~ovito.io.FileSource` to the node's output :py:class:`~ovito.data.DataCollection`.

You insert a new modifier by creating a new instance of the corresponding modifier class
(See :py:mod:`ovito.modifiers` module for a list of available modifiers) and adding it to the node's :py:attr:`~ovito.ObjectNode.modifiers`
list::

   >>> from ovito.modifiers import *
   >>> mod = AssignColorModifier( color=(0.5, 1.0, 0.0) )
   >>> node.modifiers.append(mod)
   
Entries in the :py:attr:`ObjectNode.modifiers <ovito.ObjectNode.modifiers>` list are processed front to back, i.e.,
appending a modifier to the end of the list will position it at the end of the modification pipeline.
Note that this corresponds to the bottom-up execution order known from OVITO's graphical user interface.

Inserting a modifier into the modification pipeline does not directly trigger a
computation. The modifier will only be executed when the pipeline needs to be (re-)evaluated. 
Evaluation of the modification pipeline can either happen implicitly, e.g. when

  * the interactive viewports in OVITO's main window are updated, 
  * rendering an image,
  * exporting data using :py:func:`ovito.io.export_file`,
  
or explicitly, when calling the :py:meth:`ObjectNode.compute() <ovito.ObjectNode.compute>` method.
This method explicitly updates the output cache holding the results of the node's modification pipeline.
The :py:attr:`output <ovito.ObjectNode.output>` cache is a :py:class:`~ovito.data.DataCollection`, which stores all data objects that
have left modification pipeline the last time it was evaluated::

    >>> node.compute()
    >>> node.output
    DataCollection(['Simulation cell', 'Position', 'Color'])
    
    >>> for key in node.output: 
    ...     print(node.output[key])
    <SimulationCell at 0x7fb6238f1b30>
    <ParticleProperty at 0x7fb623d0c760>
    <ParticleProperty at 0x7fb623d0c060>

In this example, the output data collection consists of a :py:class:`~ovito.data.SimulationCell`
object and two :py:class:`~ovito.data.ParticleProperty` objects, which store the particle positions and 
particle colors.

---------------------------------
Analysis modifiers
---------------------------------

Analysis modifiers use the data they receive from the upstream modification pipeline
to perform some computation. The computation results are either inserted back into the data collection flowing 
further down the pipeline, or are directly cached by the modifier itself as discussed below.

Let us take the :py:class:`~ovito.modifiers.CommonNeighborAnalysisModifier` as an example. It takes
the particle positions as input and classifies each particle as either FCC, HCP, BCC, or some other
structural type. This per-particle information computed by the modifier is inserted into the pipeline as a new 
:py:class:`~ovito.data.ParticleProperty` data object. Since it flows down the pipeline, this particle property
is accessible by subsequent modifiers and will eventually arrive in the node's output data collection::

    >>> cna = CommonNeighborAnalysis()
    >>> node.modifiers.append(cna)
    >>> node.compute()
    >>> print(node.output["Structure Type"].array)
    [1 0 0 ..., 1 2 0]
    
Note that the :py:class:`~ovito.modifiers.CommonNeighborAnalysisModifier` encodes the computed
structural type of each particle as an integer number.

In addition to this per-particle data, some analysis modifiers generate global information. 
This information is not inserted into the data pipeline; instead it is 
cached by the modifier itself and can be directly accessed as an attribute. For instance, 
the :py:attr:`~ovito.modifiers.CommonNeighborAnalysisModifier.counts` attribute of the :py:class:`~ovito.modifiers.CommonNeighborAnalysisModifier` contains 
the numbers of particles found by the modifier for each structural type::

    >>> for c in enumerate(cna.counts):
	...     print("Structure type %i: %i particles" % c)
    Structure type 0: 117317 particles
    Structure type 1: 1262 particles
    Structure type 2: 339 particles
    Structure type 3: 306 particles
    Structure type 4: 0 particles
    Structure type 5: 0 particles
    
Note that the :py:class:`~ovito.modifiers.CommonNeighborAnalysisModifier` class defines a set of integer constants 
that make it easier to refer to individual structure types, e.g.::

    >>> print("Number of FCC atoms:", cna.counts[CommonNeighborAnalysisModifier.Type.FCC])
    Number of FCC atoms: 1262

.. important::

   The most important thing to remember here is that such output attributes contain cached information from the 
   last evaluation of the analysis modifier. That means you have to call :py:meth:`ObjectNode.compute() <ovito.ObjectNode.compute>` first to make
   this information available and ensure that it is up to date!
 
